#include <QTest>
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "testconfig.h"
#include "doctype.h"
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
        QVERIFY( dt.name() == QLatin1String("Angebot"));
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

private:
    QString _docTypeName;


};

QTEST_MAIN(T_DocType)
#include "t_doctype.moc"

