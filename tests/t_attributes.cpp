#include <QTest>
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "testconfig.h"
#include "kraftdb.h"
#include "attribute.h"

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

    sqls = KraftDB::self()->parseCommandFile("10_dbmigrate.sql");
    KraftDB::self()->processSqlCommands(sqls);

    sqls = KraftDB::self()->parseCommandFile("11_dbmigrate.sql");
    KraftDB::self()->processSqlCommands(sqls);

}

class T_Attributes : public QObject {
    Q_OBJECT
private slots:
    void initTestCase()
    {
        init_test_db();
    }

    void cleanupTestCase() {
        const QString dbName("__test.db");

        QDir sourceDir(TESTS_PATH);
        sourceDir.cdUp();
        Q_ASSERT(QFile::remove(dbName));
    }

    void checkTablesExist() {
        const QStringList attribCols{"id", "hostObject", "hostId", "name", "valueIsList",
                                "relationTable", "relationIDColumn", "relationStringColumn"};
        QVERIFY(KraftDB::self()->checkTableExistsSqlite("attributes", attribCols));

        const QStringList colsAttribValues{"id", "attributeId", "value"};
        QVERIFY(KraftDB::self()->checkTableExistsSqlite("attributeValues", colsAttribValues));
    }

    void simple1() {
        Attribute att = makeAtt("Test", "foo");

        QVERIFY(att.name() == QStringLiteral("Test"));
        QVERIFY(att.value().toString() == QStringLiteral("foo"));
    }

    void copyAttribute() {
        Attribute att = makeAtt("Test", "foo");

        Attribute att2 = att;
        QVERIFY(att2.name() == QStringLiteral("Test"));
        QVERIFY(att2.value().toString() == QStringLiteral("foo"));
    }

    void saveAndLoadAttribs() {
        AttributeMap map;



    }
private:
    Attribute makeAtt(const QString& name, const QString& val) {
        Attribute att(name);

        att.setValue(val);
        att.setPersistant(true);
        att.setListValue(false);

        return att;
    }

};

QTEST_MAIN(T_Attributes)
#include "t_attributes.moc"

