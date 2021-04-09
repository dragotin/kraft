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

int DbToXMLConverter::amountOfDocs(int year)
{
    const QString sql {"SELECT count(docID) FROM document where DATE(date) BETWEEN ':year-01-01' AND ':nextyear-01-01'"};
    QSqlQuery q;
    q.prepare(sql);
    q.bindValue(":year", QVariant(year));
    q.bindValue(":nextyear", QVariant(year +1));

    q.exec();
    int amount {0};

    while (q.next()) {
        amount = q.value(0).toInt();
    }
    return amount;
}

int DbToXMLConverter::convertDocs(int year)
{
    const QString sql {"SELECT docID FROM document where DATE(date) BETWEEN ':year-01-01' AND ':nextyear-01-01' order by docID"};
    QSqlQuery q;
    q.prepare(sql);
    q.bindValue(":year", QVariant(year));
    q.bindValue(":nextyear", QVariant(year +1));

    q.exec();
    bool ok;

    DocumentSaverDB dbSaver;
    KraftDoc doc;
    DocumentSaverXML xmlSaver;
    int cnt { 0 };

    while( q.next()) {
        int docID = q.value(0).toInt(&ok);
        Q_ASSERT(ok);

        dbSaver.load(QString::number(docID), &doc);

        xmlSaver.saveDocument(&doc);
        cnt++;
    }
    return cnt;
}
