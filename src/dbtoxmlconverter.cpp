#include "dbtoxmlconverter.h"
#include "documentman.h"
#include "kraftdb.h"
#include "xmldocindex.h"
#include "defaultprovider.h"
#include "documentsaverdb.h"
#include "numbercycle.h"

#include <klocalizedstring.h>
#include <utime.h>

#include <QSaveFile>
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

    // dBase defaults to $HOME/.local/kraft/v2/current

    if (dBase.isEmpty()) {
        qDebug() << "convert called with empty database dir";

        return {};
    }

    QList<int> keys = years.keys();
    std::sort(keys.begin(), keys.end());

    QMap<QByteArray, int> overallResults;

    for (int year : keys) {
        int amount = years.value(year);
        Q_EMIT conversionOut(i18n("Transforming %1 documents for year %2...").arg(amount).arg(year));

        QMap<QByteArray, int> results;
        convertDocsOfYear(year, dBase, results);
        Q_EMIT conversionOut(i18n("     result: %1 ok, %2 fails, %3 PDF fails").arg(results[okStr]).arg(results[failsStr]).arg(results[pdfFailsStr]));

        overallResults[okStr] += results[okStr];
        overallResults[failsStr] += results[failsStr];
        overallResults[pdfFailsStr] += results[pdfFailsStr];

        // FIXME Check for errors and set ok flag
    }

    Q_EMIT conversionOut(i18n("<br/><b>Overall document conversion result:</b>"));
    Q_EMIT conversionOut(i18n("    Successfully transformed documents: %1").arg(overallResults[okStr]));
    Q_EMIT conversionOut(i18n("    Failed transformed documents: %1").arg(overallResults[failsStr]));
    Q_EMIT conversionOut(i18n("    PDF transformed fails: %1").arg(overallResults[pdfFailsStr]));

    // -- Convert the numbercycles
    int nc_cnt = convertNumbercycles(dBase);
    overallResults["numberCyclesOk"] = nc_cnt;
    Q_EMIT conversionOut(i18n("<br/>Transformed %1 numbercycle(s) successfully.").arg(nc_cnt));
    for( const auto& k : overallResults.keys()) {
        qDebug() << "Transformation result" << k << ":" << overallResults[k];
    }

    if (convertOwnIdentity(dBase)) {
        qDebug() << "manual own Identity converted";
        Q_EMIT conversionOut(i18n("<br/>Converted manual created identity successfully"));
    }

    if (nc_cnt == 0) {
        qDebug() << "Could not transform any numbercycles. Smell!";
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
    const QString sql {"SELECT id, name, lastIdentNumber, identTemplate FROM numberCycles order by id"};
    QSqlQuery q;
    q.prepare(sql);

    q.exec();

    int cnt{0};
    while( q.next()) {
        cnt++;
        NumberCycle nc;
        nc.setName(q.value(1).toString());
        nc.setCounter(q.value(2).toInt());
        nc.setTemplate(q.value(3).toString());
        nc.setDbId(q.value(0).toInt());
        if (NumberCycles::save(nc, baseDir) == NumberCycles::SaveResult::SaveOk) {
            qDebug() << "Saved numbercycle successfully:" << nc.name();
            cnt++;
        } else {
            qDebug() << "Failed to save Numbercycle" << nc.name();
        }
    }
    return cnt;
}

// copy a manually created own identity over to the new location
bool DbToXMLConverter::convertOwnIdentity(const QString& baseDir)
{
    // The Kraft 1.x file position is this:
    QString v1file = QStandardPaths::writableLocation( QStandardPaths::AppDataLocation );
    v1file += "/myidentity.vcd";

    QDir dir(baseDir);
    dir.cd(DefaultProvider::self()->kraftV2Subdir(DefaultProvider::KraftV2Dir::OwnIdentity));
    QString v2File = dir.absoluteFilePath("myidentity.vcd");

    QFile fi(v1file);
    bool re{false};
    if (fi.exists()) {
        re = fi.copy(v2File);
    }
    return re;
}
