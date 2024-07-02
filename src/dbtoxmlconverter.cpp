#include "dbtoxmlconverter.h"
#include "documentman.h"
#include "kraftdb.h"
#include "xmldocindex.h"
#include "defaultprovider.h"
#include "documentsaverdb.h"

#include <klocalizedstring.h>
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

bool setFileMTime(const QString& fileName, const QDateTime& dt)
{
    bool ok{false};

    QFile file{fileName};
    // set the mtime of the file, important: Use Append open to not wipe the file
    if (file.open(QIODevice::WriteOnly|QFile::Append)) {
        ok = file.setFileTime(dt, QFileDevice::FileModificationTime);
        file.close();
    }
    return ok;
}

}

DbToXMLConverter::DbToXMLConverter(QObject *parent) : QObject(parent)
{

}

QMap<QByteArray, int> DbToXMLConverter::convert(const QString& dBase)
{
    QMap<int, int> years = yearMap();

    // defaults to $HOME/.local/kraft/v2/current
    // QString dBase = DefaultProvider::self()->createV2BaseDir();

    if (dBase.isEmpty()) {
        qDebug() << "A new v2 base path can not be created";

        return {};
    }

    QList<int> keys = years.keys();
    std::sort(keys.begin(), keys.end());

    QMap<QByteArray, int> overallResults;

    for (int year : keys) {
        int amount = years.value(year);
        emit conversionOut(i18n("Converting %1 documents for year %2...").arg(amount).arg(year));

        QMap<QByteArray, int> results;
        convertDocsOfYear(year, dBase, results);
        emit conversionOut(i18n("     result: %1 ok, %2 fails, %3 PDF fails").arg(results[okStr]).arg(results[failsStr]).arg(results[pdfFailsStr]));

        overallResults[okStr] += results[okStr];
        overallResults[failsStr] += results[failsStr];
        overallResults[pdfFailsStr] += results[pdfFailsStr];

        // FIXME Check for errors and set ok flag
    }

    emit conversionOut(i18n("<br/><b>Overall document conversion result:</b>"));
    emit conversionOut(i18n("    Successfully converted documents: %1").arg(overallResults[okStr]));
    emit conversionOut(i18n("    Failed converted documents: %1").arg(overallResults[failsStr]));
    emit conversionOut(i18n("    PDF conversion fails: %1").arg(overallResults[pdfFailsStr]));

    // -- Convert the numbercycles
    int nc_cnt = convertNumbercycles(dBase);
    overallResults["numberCyclesOk"] = nc_cnt;
    emit conversionOut(i18n("<br/>Converted %1 numbercycle(s) successfully.").arg(nc_cnt));
    for( const auto& k : overallResults.keys()) {
        qDebug() << "Conversion result" << k << ":" << overallResults[k];
    }

    if (nc_cnt == 0) {
        qDebug() << "Could not convert any numbercycles. Smell!";
    }
    return overallResults;
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

void DbToXMLConverter::convertDocsOfYear(int year, const QString& basePath, QMap<QByteArray, int>& results)
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

    while( q.next()) {
        const QString ident = q.value(0).toString();
        const QString uuid = convertDbToXml(ident);
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

QString DbToXMLConverter::convertDbToXml(const QString& docID)
{
    DocumentSaverDB docLoad;
    KraftDoc doc;

    // load from database by ident
    if (docLoad.loadByIdent(docID, &doc)) {

        DocumentSaverXML docSave;
        docSave.setArchiveMode(true); // do not set new lastModified etc.

        if (!docSave.saveDocument(&doc)) {
            qDebug() << "failed to save document as XML" << docID;
        } else {
            // File was written successfully. Tweak the modification time to the
            // last modified date of the document.
            const QString& fileName = docSave.lastSavedFileName();

            const QDateTime& lastModified = doc.lastModified();

            if (lastModified.isValid()) {
                setFileMTime(fileName, lastModified);
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
    const QString sql{"select ident, archDocID, date, printDate from archdoc where ident=:ident order by printDate desc limit 1;"};
    QSqlQuery q;
    q.prepare(sql);
    q.bindValue(":ident", ident);

    q.exec();

    if (q.next()) {
        const QString dbIdent = q.value(0).toString();
        const QString archId = q.value(1).toString();
        const QDateTime docDate = q.value(2).toDateTime();
        const QDateTime printDate = q.value(3).toDateTime();

        const QString copyPdfName = QString("%1_%2.pdf").arg(dbIdent).arg(archId);
        const QString newPdfName = QString("%1.pdf").arg(uuid);
        QDir oldPdfDir{DefaultProvider::self()->pdfOutputDir()};
        QDir newPdfDir(basePath);
        newPdfDir.cd(DefaultProvider::self()->kraftV2Subdir(DefaultProvider::KraftV2Dir::PdfDocs));
        newPdfDir.cd(QString("%1/%2/").arg(docDate.date().year()).arg(docDate.date().month()));
        if (!newPdfDir.exists()) {
            newPdfDir.mkpath(newPdfDir.absolutePath());
        }

        const QString oldPdfFile{oldPdfDir.absoluteFilePath(copyPdfName)};
        const QString newPdfFile{newPdfDir.absoluteFilePath(newPdfName)};
        if (!QFile::exists(oldPdfFile)) {
            qDebug() << "Old pdf file" << oldPdfFile << "can not be found";
            return false;
        }

        if (QFile::copy(oldPdfFile, newPdfFile)) {
            setFileMTime(newPdfFile, printDate);
            ok = true;
        } else {
            qDebug() << "Unable to copy" << oldPdfFile << "to" << newPdfFile;
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
