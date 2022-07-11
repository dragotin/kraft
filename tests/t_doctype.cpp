#include <QTest>
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "testconfig.h"
#include "doctype.h"
#include "kraftdb.h"
#include "sql_states.h"

void init_test_db()
{
    const QString dbName("__test.db");

    QDir sourceDir(TESTS_PATH);
    sourceDir.cdUp();
    const QByteArray ba {sourceDir.absolutePath().toLatin1()};
    qputenv("KRAFT_HOME", ba);

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
private slots:
    void initTestCase()
    {
        init_test_db();
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

    void checkAll() {
        QStringList allDts = DocType::allLocalised();

        for( const QString& s : allDts ) {
            qDebug() << "** " << s;
        }
        QVERIFY(allDts.count() > 2);

        _docTypeName = allDts.at(1);
    }

    void loadADt() {
        qDebug() << "Loading doctype" << _docTypeName;
        DocType dt(_docTypeName);

        QVERIFY( dt.name() == _docTypeName);
        QVERIFY( dt.name() == QLatin1Literal("Angebot"));
        QVERIFY( dt.allowAlternative());
        QVERIFY( dt.allowDemand());
    }

    void checkFollowers() {

        DocType dt(_docTypeName);

        QStringList f = dt.follower();
        QVERIFY(f.contains("Rechnung"));
    }

    void createNewDoctype() {
        QStringList f;
        f.append("Angebot");
        f.append("Rechnung");

        DocType dt( "Test" );
        dt.setAttribute("myattrib", "Kraft");
        dt.setMergeIdent("2");
        dt.save();
        int num = dt.setAllFollowers(f);
        QVERIFY(2 == num );
    }

    void readNewDoctype() {
        DocType dt("Test");
        QVERIFY(dt.mergeIdent() == "2");
        QVERIFY(dt.name() == "Test");
        QVERIFY(! dt.allowAlternative());

        QStringList li = dt.follower();
        QVERIFY( li.size() == 2 );
        QVERIFY( li.contains("Angebot"));
        QVERIFY( li.indexOf("Angebot") == 0 );
        QVERIFY( li.indexOf("Rechnung") == 1 );
    }

    void addAFollower() {
        DocType dt("Test");
        QStringList li = dt.follower();
        QVERIFY(li.size() == 2);
        li.append("Offer");
        int num = dt.setAllFollowers(li);
        qDebug() << "oo " << num;
        QVERIFY(1 == num);
        QVERIFY( li.contains("Angebot"));
        QVERIFY( li.contains("Rechnung"));
        QVERIFY( li.contains("Offer"));
    }

    void checkVariableReplacement() {
        DocType dt("Test");

        dt.setIdentTemplate("FOO-%y-%w-%d-%i");
        QString re = dt.generateDocumentIdent(QDate(2020, 01,23), "addressUID", 122, 0);
        QCOMPARE(re, QStringLiteral("FOO-2020-4-23-122"));

        dt.setIdentTemplate("FOO-%y-%ww-%d-%i");
        re = dt.generateDocumentIdent(QDate(2020, 01,23), "addressUID", 122, 0);
        QCOMPARE(re, QStringLiteral("FOO-2020-04-23-122")); // the id can change

        dt.setIdentTemplate("FOO-%y-%ww-%d-%iiiii");
        re = dt.generateDocumentIdent(QDate(2020, 01,23), "addressUID", 122, 0);
        QCOMPARE(re, QStringLiteral("FOO-2020-04-23-00122")); // the id can change

        dt.setIdentTemplate("FOO-%y-%ww-%d-%iiii");
        re = dt.generateDocumentIdent(QDate(2020, 01,23), "addressUID", 122, 0);
        QCOMPARE(re, QStringLiteral("FOO-2020-04-23-0122")); // the id can change

        dt.setIdentTemplate("FOO-%y-%ww-%d-%iii");
        re = dt.generateDocumentIdent(QDate(2020, 01,23), "addressUID", 122, 0);
        QCOMPARE(re, QStringLiteral("FOO-2020-04-23-122")); // the id can change

        dt.setIdentTemplate("FOO-%y-%ww-%d-%ii");
        re = dt.generateDocumentIdent(QDate(2020, 01,23), "addressUID", 122, 0);
        QCOMPARE(re, QStringLiteral("FOO-2020-04-23-122")); // the id can change

    }

    void checkDayCntIncrement()
    {
        DocType dt("Test");
        const QDate d1(2022, 01, 23);
        const QDate d2(2022, 01, 25);
        int cnt = dt.nextDayCounter(d1);
        QCOMPARE(cnt, 1);
        cnt = dt.nextDayCounter(d1);
        QCOMPARE(cnt, 2);
        cnt = dt.nextDayCounter(d1);
        QCOMPARE(cnt, 3);

        // - switch to next date
        cnt = dt.nextDayCounter(d2);
        QCOMPARE(cnt, 1);
        cnt = dt.nextDayCounter(d2);
        QCOMPARE(cnt, 2);

        // - back to d1
        cnt = dt.nextDayCounter(d1);
        QCOMPARE(cnt, 1);
    }

    void checkDateCounter()
    {
        DocType dt("Test");

        // Attention: This test assumes that all verifies run on the same day.
        dt.setIdentTemplate("FOO-%n");
        QString re = dt.generateDocumentIdent(QDate(2020, 01,23), "addressUID", 122, 2);
        QCOMPARE(re, "FOO-2");

        // Test the padding functionality
        dt.setIdentTemplate("FOO-%nn");
        re = dt.generateDocumentIdent(QDate(2020, 01,23), "addressUID", 122, 3);
        QCOMPARE(re, "FOO-03");

        dt.setIdentTemplate("FOO-%nnn");
        re = dt.generateDocumentIdent(QDate(2020, 01,23), "addressUID", 122, 3);
        QCOMPARE(re, "FOO-003");

        dt.setIdentTemplate("FOO-%nnnn");
        re = dt.generateDocumentIdent(QDate(2020, 01,23), "addressUID", 122, 6);
        QCOMPARE(re, "FOO-0006");

    }
private:
    QString _docTypeName;


};

QTEST_MAIN(T_DocType)
#include "t_doctype.moc"

