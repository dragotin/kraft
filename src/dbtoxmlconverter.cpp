#include "dbtoxmlconverter.h"
#include "documentsaverdb.h"
#include "documentsaverxml.h"
#include "kraftdoc.h"

#include <QSqlQuery>
#include <QStandardPaths>
#include <QVariant>

namespace {

}


DbToXMLConverter::DbToXMLConverter(QObject *parent) : QObject(parent)
{

}

QMap <int, int> DbToXMLConverter::yearMap()
{
    QMap<int, int> reMap;

    // const QString sql {"SELECT "};
    // QSqlQuery q;

    return reMap;
}

int DbToXMLConverter::amountOfDocs(int year)
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

int DbToXMLConverter::convertDocs(int year)
{
    const QString sql {"SELECT docID FROM document where DATE(date) BETWEEN :year AND :nextyear order by docID"};
    QSqlQuery q;
    q.prepare(sql);
    q.bindValue(":year", QString("%1-01-01").arg(QString::number(year)));
    q.bindValue(":nextyear", QString("%1-01-01").arg(QString::number(year+1)));

    q.exec();
    bool ok;

    DocumentSaverDB dbSaver;
    KraftDoc doc;
    DocumentSaverXML xmlSaver;
    xmlSaver.setBasePath("/tmp/xmlconverter/");
    int cnt { 0 };

    while( q.next()) {
        int docID = q.value(0).toInt(&ok);
        Q_ASSERT(ok);

        doc.openDocument(QString::number(docID));

        xmlSaver.saveDocument(&doc);
        cnt++;
    }
    return cnt;
}
