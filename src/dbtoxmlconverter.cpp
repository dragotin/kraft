#include "dbtoxmlconverter.h"
#include "documentman.h"
#include "kraftdb.h"
#include "xmldocindex.h"
#include "defaultprovider.h"

#include <QSqlQuery>
#include <QStandardPaths>
#include <QVariant>

namespace {

QDomElement xmlTextElement( QDomDocument& doc, const QString& name, const QString& value )
{
  QDomElement elem = doc.createElement( name );
  QDomText t = doc.createTextNode( value.toHtmlEscaped() );
  elem.appendChild( t );
  return elem;
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
    int cnt {0};

    const QString xmlBase{dBase + "/xmldoc"};
    for (int year : keys) {
        cnt += convertDocsOfYear(year, xmlBase);
        // FIXME Check for errors and set ok flag
    }
    qDebug() << "Done, saved"<< cnt << "docs to"<< xmlBase;

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
