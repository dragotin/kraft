#include "dbtoxmlconverter.h"
#include "documentman.h"
#include "kraftdb.h"
#include "xmldocindex.h"
#include "defaultprovider.h"
#include "documentsaverdb.h"

#include <utime.h>

#include <QSqlQuery>
#include <QStandardPaths>
#include <QVariant>

namespace {

const char *okStr{"ok"};
const char *pdfFailsStr{"pdfFails"};
const char *failsStr{"fails"};

QDomElement xmlTextElement( QDomDocument& doc, const QString& name, const QString& value )
{
  QDomElement elem = doc.createElement( name );
  QDomText t = doc.createTextNode( value.toHtmlEscaped() );
  elem.appendChild( t );
  return elem;
}

bool writeJsonIndex(const QString& path, QJsonObject &json)
{
    QFileInfo fi(path, "kraftindx.json");

    QFile saveFile(fi.absoluteFilePath());

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }

    saveFile.write(QJsonDocument(json).toJson());
    return true;
}

}

DbToXMLConverter::DbToXMLConverter(QObject *parent) : QObject(parent)
{

}

void DbToXMLConverter::convert()
{
    QMap<int, int> years = yearMap();
    bool ok {true};

    // defaults to $HOME/.local/kraft/v2/current
    QString dBase = DefaultProvider::self()->createV2BaseDir();

    if (dBase.isEmpty()) {
        qDebug() << "A new v2 base path can not be created";

        return;
    }

    QList<int> keys = years.keys();
    std::sort(keys.begin(), keys.end());

    QJsonObject jsonDoc; // digest of all docs.
    QJsonObject yearsMap;

    QMap<QByteArray, int> results;

    QJsonArray yearsArr;
    for (int year : keys) {
        QJsonArray arr;
        convertDocsOfYear(year, dBase, arr, results);
        yearsMap[QString::number(year)] = arr;

        // FIXME Check for errors and set ok flag
    }
    qDebug() << "Conversion to" << dBase;
    for( const auto& k : results.keys()) {
        qDebug() << "Conversion result" << k << ":" << results[k];
    }

    jsonDoc["yearsData"] = yearsMap;
    writeJsonIndex(dBase, jsonDoc);

    // -- Convert the numbercycles
    int nc_cnt = convertNumbercycles(dBase);
    qDebug() << "Converted"<< nc_cnt << "numbercycles in"<< dBase;

    if (nc_cnt == 0) {
        qDebug() << "Could not convert any numbercycles. Smell!";
        ok = false;
    }

    // if all was good it is switched to the new base dir
    if (ok) {
        if (DefaultProvider::self()->switchToV2BaseDir(dBase)) {

            XmlDocIndex indx;
            Q_UNUSED(indx)
        }
    }
}

QMap<int, int> DbToXMLConverter::yearMap()
{
    QMap<int, int> reMap;

    QString sql {"SELECT count(docID), YEAR(date) as Year FROM document GROUP by Year"};
    // QSqlQuery q;
    if (KraftDB::self()->isSqlite()) {
        sql = QStringLiteral("SELECT count(docID), STRFTIME(\"%Y\", date) as Year FROM document GROUP by Year;");
    }

    QSqlQuery q;
    q.prepare(sql);
    if (q.exec()) {
        while( q.next()) {
            reMap.insert(q.value(1).toInt(), q.value(0).toInt());
        }
    }

    return reMap;
}

int DbToXMLConverter::amountOfDocsOfYear(int year)
{
    const QString sql {"SELECT count(docID) FROM document where DATE(date) BETWEEN :year AND :nextyear"};
    QSqlQuery q;
    q.prepare(sql);
    q.bindValue(":year", QString("%1-01-01").arg(QString::number(year)));
    q.bindValue(":nextyear", QString("%1-01-01").arg(QString::number(year+1)));

    q.exec();
    int amount {-1};

    while (q.next()) {
        amount = q.value(0).toInt();
    }
    return amount;
}

void DbToXMLConverter::convertDocsOfYear(int year, const QString& basePath, QJsonArray &yearArr, QMap<QByteArray, int>& results)
{
    const QString sql {"SELECT ident FROM document where DATE(date) BETWEEN :year AND :nextyear order by docID"};
    QSqlQuery q;
    q.prepare(sql);
    q.bindValue(":year", QString("%1-01-01").arg(QString::number(year)));
    q.bindValue(":nextyear", QString("%1-01-01").arg(QString::number(year+1)));

    q.exec();
    int cnt{0};
    int fails{0};
    int pdffails{0};

    QDir newDir(basePath);
    newDir.cd(DefaultProvider::self()->kraftV2Subdir(DefaultProvider::KraftV2Dir::XmlDocs));

    while( q.next()) {
        const QString ident = q.value(0).toString();
        const QString uuid = convertDbToXml(ident, newDir.absolutePath(), yearArr);
        if (ident.isEmpty() || uuid.isEmpty()) {
            qDebug() << "Failed to convert document" << ident;
            fails++;
        } else {
            if (!convertLatestPdf(basePath, ident, uuid)) {
                pdffails++;
            }
            cnt++;
        }
    }

    // report the results
    results[okStr] += cnt;
    results[failsStr] += fails;
    results[pdfFailsStr] += pdffails;

}

QString DbToXMLConverter::convertDbToXml(const QString& docID, const QString& basePath, QJsonArray& jsonArr)
{
    DocumentSaverDB docLoad;
    KraftDoc doc;

    // load from database by ident
    if (docLoad.loadByIdent(docID, &doc)) {

        DocumentSaverXML docSave;
        docSave.setBasePath(basePath);
        docSave.setArchiveMode(true); // do not set new lastModified etc.

        if (!docSave.saveDocument(&doc)) {
            qDebug() << "failed to save document as XML" << docID;
        } else {
            // File was written successfully. Tweak the modification time to the
            // last modified date of the document.
            const QString& fileName = docSave.lastSavedFileName();

            // push the JSON representation of the doc digest to an array
            QJsonObject obj;
            doc.toJsonObj(obj);
            jsonArr.append(obj);

            const QDateTime& lastModified = doc.lastModified();

            if (lastModified.isValid()) {
                /*
             *        The utimbuf structure is:
             *
             *        struct utimbuf {
             *          time_t actime;       // access time
             *          time_t modtime;      // modification time
             *        };
             */
                struct tm time;
                time.tm_sec = lastModified.time().second();
                time.tm_min = lastModified.time().minute();
                time.tm_hour = lastModified.time().hour();
                time.tm_mday = lastModified.date().day();
                time.tm_mon = lastModified.date().month()-1;
                time.tm_year = lastModified.date().year()-1900;
                struct utimbuf utime_par;
                utime_par.modtime = mktime(&time);
                // utime_par.actime  = mktime()
                utime(fileName.toUtf8().constData(), &utime_par);
            } else {
                qDebug() << "Invalid time stamp for last modified for" << fileName;
            }
        }
    } else {
        qDebug() << "Failed to load from db" << docID;
    }

    const QString uuid = doc.uuid();
    return uuid;

}

bool DbToXMLConverter::convertLatestPdf(const QString& basePath, const QString& ident, const QString& uuid)
{
    bool ok{false};

    // query the most recent PDF from the db.
    const QString sql{"select ident, archDocID, printDate from archdoc where ident=:ident order by printDate desc limit 1;"};
    QSqlQuery q;
    q.prepare(sql);
    q.bindValue(":ident", ident);

    q.exec();

    if (q.next()) {
        const QString dbIdent = q.value(0).toString();
        const QString archId = q.value(1).toString();
        const QDateTime printDate = q.value(2).toDateTime();

        const QString copyPdfName = QString("%1_%2.pdf").arg(dbIdent).arg(archId);
        const QString newPdfName = QString("%1.pdf").arg(uuid);
        QDir oldPdfDir{DefaultProvider::self()->pdfOutputDir()};
        QDir newPdfDir(basePath);
        newPdfDir.cd(DefaultProvider::self()->kraftV2Subdir(DefaultProvider::KraftV2Dir::PdfDocs));

        const QString oldPdfFile{oldPdfDir.absoluteFilePath(copyPdfName)};
        const QString newPdfFile{newPdfDir.absoluteFilePath(newPdfName)};
        if (!QFile::exists(oldPdfFile)) {
            qDebug() << "Old pdf file" << oldPdfFile << "can not be found";
            return false;
        }

        if (QFile::copy(oldPdfFile, newPdfFile)) {
            QFile file{newPdfFile};
            // set the mtime of the file
            if (file.open(QIODevice::WriteOnly)) {
                ok = file.setFileTime(printDate, QFileDevice::FileModificationTime);
                file.close();
            }
        }
    }
    return ok;
}

int DbToXMLConverter::convertNumbercycles(const QString& baseDir)
{
    const QString kncStr{"kraftNumberCycles"};
    const QString sql {"SELECT id, name, lastIdentNumber, identTemplate FROM numberCycles order by id"};
    QSqlQuery q;
    q.prepare(sql);

    q.exec();
    int cnt{0};

    QDomDocument xmldoc(kncStr);
    QDomProcessingInstruction instr = xmldoc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
    xmldoc.appendChild(instr);

    QDomElement root = xmldoc.createElement(kncStr);
    root.setAttribute("schemaVersion", "1");
    xmldoc.appendChild( root );

    while( q.next()) {
        QDomElement cycle = xmldoc.createElement( "cycle" );
        root.appendChild(cycle);
        cycle.appendChild(xmlTextElement(xmldoc, "name", q.value(1).toString()));
        cycle.appendChild(xmlTextElement(xmldoc, "lastNumber", q.value(2).toString()));
        cycle.appendChild(xmlTextElement(xmldoc, "template", q.value(3).toString()));
        cycle.appendChild(xmlTextElement(xmldoc, "dbId", q.value(0).toString()));
        cnt++;
    }

    const QString xml = xmldoc.toString();
    QDir dir(baseDir);
    dir.cd(DefaultProvider::self()->kraftV2Subdir(DefaultProvider::KraftV2Dir::NumberCycles));

    bool re;
    QSaveFile file(dir.absoluteFilePath("numbercycles.xml"));
    if ( file.open( QIODevice::WriteOnly | QIODevice::Text) ) {
        re = file.write(xml.toUtf8());

        if (re) {
            re = file.commit();
        }
    }
    return cnt;
}
