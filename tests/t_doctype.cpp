#include <QTest>
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringBuilder>

#include "testconfig.h"
#include "doctype.h"
#include "kraftdb.h"

using namespace Qt::StringLiterals;

void init_test_db()
{
    const QString dbName("__test.db");

    QDir sourceDir(TESTS_PATH);
    sourceDir.cdUp();
    const QByteArray ba {sourceDir.absolutePath().toLatin1()};
    qputenv("KRAFT_HOME", ba);

    qDebug() << "KRAFT_HOME is set to" << ba;
    QFile::remove(dbName);

    KraftDB::self()->dbConnect("QSQLITE", dbName, QString(), QString(), QString());

    SqlCommandList sqls = KraftDB::self()->parseCommandFile("5_dbmigrate.sql");
    QVERIFY(sqls.size() > 0);
    KraftDB::self()->processSqlCommands(sqls);

    // modify the initial attributes tables
    sqls = KraftDB::self()->parseCommandFile("10_dbmigrate.sql");
    KraftDB::self()->processSqlCommands(sqls);

    // get the followers into the database
    sqls = KraftDB::self()->parseCommandFile("8_dbmigrate.sql");
    KraftDB::self()->processSqlCommands(sqls);

}

class T_DocType : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        init_test_db();

        // =============== Xml Based
        _baseDir = DefaultProvider::self()->createV2BaseDir(_dir.path());

        // FIXME: Get rid of the basedir stuff here, and hold the base dir in the
        // DefaultProvider
        // Use this method to create a temp base dir: DefaultProvider::self()->switchToV2BaseDir(d);
        qDebug() << "Test Basedir:" << _baseDir;
        // in the base path there needs to be a directory called 'current'

    }

    void checkTablesExist() {
        const QStringList attribCols{"id", "hostObject", "hostId", "name", "valueIsList",
                                "relationTable", "relationIDColumn", "relationStringColumn"};
        QVERIFY(KraftDB::self()->checkTableExistsSqlite("attributes", attribCols));

        const QStringList attribColsDt{"docTypeID", "name"};
        QVERIFY(KraftDB::self()->checkTableExistsSqlite("DocTypes", attribColsDt));

        const QStringList colsAttribValues{"id", "attributeId", "value"};
        QVERIFY(KraftDB::self()->checkTableExistsSqlite("attributeValues", colsAttribValues));
    }

    void save1() {
        DocTypes dts;
        DocType dt;
        dt.setName("TestDocType");
        dt.setAppendPDFFile("appender.pdf");
        dt.setTemplateFile("mytemplate.gtmpl");
        dt.setWatermarkFile("mywatermark.pdf");
        dt.setXRechnungTemplate("myxrechnungstemplate.xml");
        dt.setXRechnungEnabled(true);
        dt.setMergeIdent(3);
        dt.setUuid("foo-bar-baz");
        const QStringList li{"Tag1", "Tag2", "Tag3"};
        dt.setTags(li);

        const QStringList flist{"Auftragsbestätigung", "Rechnung", "Teilrechnung"};
        dt.setFollowers(flist);

        auto res = dts.save(dt, _baseDir);
        QCOMPARE(res, DocTypes::SaveResult::SaveOk);

    }

    void load1() {
        DocTypes dts;
        dts.loadAll(_baseDir);
        const auto map = dts.map();

        const DocType dt = map["TestDocType"];

        QCOMPARE(map.size(), 1);
    }

    void saveAll() {
        DocTypes dts;

        auto create = [](int i) -> DocType {
            DocType dt;
            dt.setName("TestDocType " % QString::number(i));
            dt.setAppendPDFFile("appender.pdf");
            dt.setTemplateFile("mytemplate.gtmpl");
            dt.setWatermarkFile("mywatermark.pdf");
            dt.setXRechnungTemplate("myxrechnungstemplate.xml");
            dt.setXRechnungEnabled(true);
            dt.setMergeIdent(3);
            dt.setUuid("foo-bar-baz");
            dt.setPricesHidden(true);
            dt.setAllowAlternative(true);
            dt.setAllowDemand(true);

            const QStringList flist{"Auftragsbestätigung", "Rechnung", "Teilrechnung"};
            dt.setFollowers(flist);

            return dt;
        };

        QMap<QString, DocType> m;
        for (int i = 1; i < 10; i++) {
            const DocType dt = create(i);
            m.insert(dt.name(), dt);
        }

        const auto res = dts.saveAll(m, _baseDir);
        QCOMPARE(res, DocTypes::SaveResult::SaveOk);

    }

    void checkAllNames() {
        DocTypes dts;
        const QStringList allDts = dts.allNames();

        for( const QString& s : allDts ) {
            qDebug() << "** " << s;
        }
        QVERIFY(allDts.count() == 10);

        for (int i = 1; i < 10; i++) {
            QCOMPARE(allDts.at(i), "TestDocType " % QString::number(i) );
        }

        _docTypeName = allDts.at(1);
        QCOMPARE(_docTypeName, "TestDocType 1");
    }

    void loadADt() {
        qDebug() << "Loading doctype" << _docTypeName;
        DocTypes dts;
        DocType dt = dts.get(_docTypeName);
        QCOMPARE(dt.name(), "TestDocType 1");

        QVERIFY( dt.name() == _docTypeName);
        QVERIFY( dt.allowAlternative());
        QVERIFY( dt.allowDemand());
        QVERIFY(dt.pricesHidden());
        QVERIFY(!dt.partialInvoice());
    }

    void checkFollowers() {
        DocTypes dts;
        DocType dt = dts.get(_docTypeName);
        QCOMPARE(dt.name(), "TestDocType 1");

        QStringList f = dt.follower();
        QVERIFY(f.at(1) == u"Rechnung"_s);
        QVERIFY(f.at(0) == u"Auftragsbestätigung"_s);
        QVERIFY(f.at(2) == u"Teilrechnung"_s);
        QVERIFY(!f.contains("Quadratrechnung"));
    }

    void createNewDoctype() {
        const QStringList f{"Angebot", "Rechnung"};

        DocTypes dts;
        DocType dt = dts.get( "Test" );
        dt.setMergeIdent(2);
        dt.setFollowers(f);
        int num = dt.follower().count();
        QVERIFY(2 == num );
        dts.save(dt);
    }

    void docTypeDefaults() {
        DocTypes dts;
        const DocType dt = dts.get("Test");
        QVERIFY(dt.mergeIdent() == 0);
        QVERIFY(!dt.isXRechnungEnabled());
        QVERIFY(dt.name().isEmpty());
        QVERIFY(! dt.allowAlternative());
        QVERIFY(! dt.allowDemand());
    }

private:
    QString _docTypeName;
    QString _baseDir;
    QTemporaryDir _dir;
};

QTEST_MAIN(T_DocType)
#include "t_doctype.moc"

