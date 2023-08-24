#include <QTest>

#include <QTemporaryDir>
#include <QFile>
#include <xmldocindex.h>

#include "qtestcase.h"
#include "testconfig.h"
#include "kraftdoc.h"
#include "documentsaverxml.h"
#include "kraftdb.h"

void init_test_db()
{
    const QString dbName("__test.db");

    QDir sourceDir(TESTS_PATH);
    sourceDir.cdUp();
    const QByteArray ba {sourceDir.absolutePath().toLatin1()};
    qputenv("KRAFT_HOME", ba);

    QFile::remove(dbName);

    KraftDB::self()->dbConnect("QSQLITE", dbName, QString(), QString(), QString());

    SqlCommandList sqls = KraftDB::self()->parseCommandFile("create_schema.sql");
    QVERIFY(sqls.size() > 0);
    KraftDB::self()->processSqlCommands(sqls);

    sqls = KraftDB::self()->parseCommandFile("fill_schema_en.sql");
    QVERIFY(sqls.size() > 0);
    KraftDB::self()->processSqlCommands(sqls);

    sqls = KraftDB::self()->parseCommandFile("24_dbmigrate.sql");
    QVERIFY(sqls.size() > 0);
    KraftDB::self()->processSqlCommands(sqls);
}

class T_XmlSaver: public QObject {
    Q_OBJECT
private slots:

    // copies the example xml document to a temporary path that simulates the
    // storage tree of kraft.
    void initTestCase()
    {
        Q_ASSERT(_dir.isValid());

        // dir.path() returns the unique directory path
        QDir d {_dir.path()};
        const QString docDir {d.path()+_subdir};
        Q_ASSERT(d.mkpath(docDir));
        const QString docPath = QString("%1/%2.xml").arg(docDir).arg(_docIdent);
        qDebug() << "Example document path: " << docPath;

        const QString kraftHome{TESTS_PATH};
        QVERIFY(!kraftHome.isEmpty());
        const QString src{kraftHome+"/../xml/kraftdoc.xml"};
        QFile::copy(src, docPath);

        // generate the index
        XmlDocIndex indx;
        indx.setBasePath(_dir.path());

        QFileInfo fi(docPath);
        Q_ASSERT(fi.exists());

        init_test_db();
    }

    void xmlVerify()
    {
        DocumentSaverXML xmlSaver;
        xmlSaver.setBasePath(_dir.path());

        const QString kraftHome{TESTS_PATH};
        QVERIFY(!kraftHome.isEmpty());
        const QString src{kraftHome+"/../xml/kraftdoc.xml"};

        const QString schema{kraftHome+"/../xml/kraftdoc.xsd"};
        const QUrl schemaFile = QUrl::fromLocalFile(schema);
        QVERIFY(xmlSaver.verifyXmlFile(schemaFile, src));

    }

    void loadMetaAndHeader()
    {
        DocumentSaverXML xmlSaver;
        xmlSaver.setBasePath(_dir.path());

        KraftDoc doc;
        QCOMPARE(doc.state(), KraftDoc::State::New);

        QVERIFY(xmlSaver.loadByIdent(_docIdent, &doc));

        QCOMPARE(doc.ident(), "20110127");
        QCOMPARE(doc.docType(), "Rechnung");
        QCOMPARE(doc.whiteboard(), "whiteboard content");
        QCOMPARE(doc.uuid(), "f4deb784-6131-4e49-bd6d-92bd1355ea4d");
        QCOMPARE(doc.state(), KraftDoc::State::Final);
        QCOMPARE(doc.date(), QDate(2011, 1, 27));
        QCOMPARE(doc.timeOfSupplyStart(), QDateTime(QDate(2019, 11, 21), QTime(0,0)));
        QCOMPARE(doc.timeOfSupplyEnd(), QDateTime(QDate(2019, 11, 23), QTime(23,59, 59)));
        QCOMPARE(doc.owner(), "kf");
        QCOMPARE(doc.lastModified(), QDateTime::fromString("2018-12-15T18:22:20", Qt::ISODate));
        QCOMPARE(doc.predecessor(), "id");

        KraftAttrib attrib = doc.attribute("valid until");
        QCOMPARE(attrib.name(), "valid until");
        QCOMPARE(attrib.type(), KraftAttrib::Type::Date);
        QCOMPARE(attrib.value().toDate(), QDate(2012, 1, 21));

        attrib = doc.attribute("counter");
        QCOMPARE(attrib.name(), "counter");
        QCOMPARE(attrib.type(), KraftAttrib::Type::Integer);
        QCOMPARE(attrib.value().toInt(), 21);

        const QStringList tags = doc.allTags();
        QVERIFY(tags.contains("foo"));
        QVERIFY(tags.contains("bar"));
        QVERIFY(!doc.isNew());

        QCOMPARE(doc.addressUid(), "BMhh9EhLwr");;
        QCOMPARE(doc.address(), "Goofy Stambulchicz");

        QCOMPARE(doc.projectLabel(), "hausgarten");
        QCOMPARE(doc.salut(), "lieber goofy,");
        QCOMPARE(doc.preText(), "Wir freuen uns, mit Dir Geschäfte machen zu können.");

        QCOMPARE(doc.postText(), "Danke für dein Interesse,");
        QCOMPARE(doc.goodbye(), "mit den besten Grüssen,");
    }

    void loadItems()
    {
        DocumentSaverXML xmlSaver;
        xmlSaver.setBasePath(_dir.path());

        KraftDoc doc;
        QCOMPARE(doc.state(), KraftDoc::State::New);

        QVERIFY(xmlSaver.loadByIdent(_docIdent, &doc));

        DocPositionList list = doc.positions();
        QCOMPARE(list.count(), 4);

        DocPosition *dp = static_cast<DocPosition*>(list[0]);
        QCOMPARE(dp->type(), DocPositionBase::PositionType::Position);
        QCOMPARE(dp->text(), QStringLiteral("first item"));
        QCOMPARE(dp->amount(), 8.4);
        QCOMPARE(dp->unit().einheitSingular(), QStringLiteral("sm"));
        QCOMPARE(dp->taxType(), DocPositionBase::TaxType::TaxFull);
        QCOMPARE(dp->unitPrice().toDouble(), 22.21);
        QCOMPARE(dp->overallPrice().toDouble(), 186.56);

        QVERIFY(dp->hasTag("Work"));

        dp = static_cast<DocPosition*>(list[1]);
        QCOMPARE(dp->type(), DocPositionBase::PositionType::Position);
        QCOMPARE(dp->text(), QStringLiteral("second item"));
        QCOMPARE(dp->amount(), 4.4);
        QCOMPARE(dp->unit().einheitSingular(), QStringLiteral("cbm"));
        QCOMPARE(dp->taxType(), DocPositionBase::TaxType::TaxReduced);
        QCOMPARE(dp->unitPrice().toDouble(), 33.33);
        QCOMPARE(dp->overallPrice().toDouble(), 146.65);

        QVERIFY(dp->hasTag("Work"));
        QVERIFY(dp->hasTag("Plants"));
    }

    void checkTotals()
    {
        DocumentSaverXML xmlSaver;
        xmlSaver.setBasePath(_dir.path());

        KraftDoc doc;
        doc.setTaxValues(19.0, 7.0);
        QVERIFY(xmlSaver.loadByIdent(_docIdent, &doc));

        XML::Totals t = xmlSaver.getLastTotals();
        QCOMPARE(t._brutto.toLong(), doc.bruttoSum().toLong());
        QCOMPARE(t._netto.toLong(), doc.nettoSum().toLong());
    }


private:
    QString _docTypeName;
    // needs to be defined according the example document in $KRAFT_HOME/xml
    const QString _subdir {"/2011/01"};
    const QString _docIdent {"20110127"};
    QTemporaryDir _dir;

};

QTEST_MAIN(T_XmlSaver)

#include "t_xmlsaver.moc"
