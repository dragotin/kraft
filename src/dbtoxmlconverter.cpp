#include "dbtoxmlconverter.h"
#include "documentman.h"
#include "kraftdb.h"

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
    QMap<int, int> q = yearMap();

    QTemporaryDir tempDir;
    tempDir.setAutoRemove(false);
    const QString tempPath {tempDir.path()};

    qDebug() << "converting XML Documents to" << tempPath;

    QList<int> keys = q.keys();
    std::sort(keys.begin(), keys.end());
    int cnt {0};
    for (int year : keys) {
        cnt += convertDocsOfYear(year, tempPath);
    }
     qDebug() << "Done, saved"<< cnt << "docs to"<< tempPath;
}

QMap <int, int> DbToXMLConverter::yearMap()
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
