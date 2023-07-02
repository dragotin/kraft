#include "dbtoxmlconverter.h"
#include "documentman.h"
#include "kraftdb.h"
#include "xmldocindex.h"
#include "kraftsettings.h"

#include <QSqlQuery>
#include <QStandardPaths>
#include <QVariant>

namespace {

}

DbToXMLConverter::DbToXMLConverter(QObject *parent) : QObject(parent)
{

}

void DbToXMLConverter::convert()
{
    QMap<int, int> years = yearMap();

    QString dBase = KraftSettings::self()->xmlDocumentsBasePath(); // something like $HOME/.local/kraft/xmldoc

    if (dBase.isEmpty()) {
        qDebug() << "No xml Documents base path found";

        return;
    }

    // Append a uniq part
    bool ok {false};
    QString link; // used further down to create the link
    QDir baseDir;
    do { // Loop until we find a not yet taken
        QUuid uuid = QUuid::createUuid();
        const QString fragment{uuid.toString(QUuid::StringFormat::WithoutBraces).left(5)};

        QFileInfo fi( QString("%1/%2").arg(dBase).arg(fragment));
        if (!fi.exists()) {
            baseDir.setPath(fi.absoluteFilePath());
            baseDir.mkpath(baseDir.absolutePath());
            link = fragment;
            qDebug() << "converting XML Documents to" << baseDir.absolutePath() ;
            ok = true;
        }
    } while(!ok);

    QList<int> keys = years.keys();
    std::sort(keys.begin(), keys.end());
    int cnt {0};
    const QString bp {baseDir.path()};
    for (int year : keys) {
        cnt += convertDocsOfYear(year, baseDir.path());
        // FIXME Check for errors
    }
    qDebug() << "Done, saved"<< cnt << "docs to"<< baseDir.absolutePath();

    if (ok) {
        QDir d(baseDir);
        d.cdUp();
        const QString linkFile{d.absoluteFilePath("current")};

        QFile f(linkFile);
        if (f.exists()) {
            f.remove();
        }
        ok = QFile::link(link, linkFile);
    }

    if (ok) {
        XmlDocIndex indx;
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

int DbToXMLConverter::convertDocsOfYear(int year, const QString& basePath)
{
    const QString sql {"SELECT ident FROM document where DATE(date) BETWEEN :year AND :nextyear order by docID"};
    QSqlQuery q;
    q.prepare(sql);
    q.bindValue(":year", QString("%1-01-01").arg(QString::number(year)));
    q.bindValue(":nextyear", QString("%1-01-01").arg(QString::number(year+1)));

    q.exec();
    int cnt{0};

    while( q.next()) {
        const QString ident = q.value(0).toString();
        if (DocumentMan::self()->convertDbToXml(ident, basePath)) {

        }
        cnt++;
    }
    return cnt;
}
