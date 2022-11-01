#include <QTest>
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "testconfig.h"
#include "kraftdb.h"
#include "attribute.h"
#include "dbids.h"

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
        const QString host {"testhost"};
        AttributeMap map(host);
        Attribute att1 = makeAtt("Test", "foo");
        Attribute att2 = makeAtt("Test2", "foobar");
        map[att1.name()] = att1;
        map[att2.name()] = att2;


        dbID id(342);
        map.save(id);

        AttributeMap m2(host);
        m2.load(id);

        QVERIFY(m2.contains(att1.name()));
        QVERIFY(m2.contains(att2.name()));
    }

    void listValues() {
        const QString host {"listtest"};
        AttributeMap map(host);
        {
            Attribute att1 = makeAtt("Test", "foo");
            att1.setListValue(true);
            const QStringList li1 {"foo", "bar", "baz"};
            att1.setValue(QVariant(li1));
            map[att1.name()] = att1;
        }
        Attribute att2 = makeAtt("Test2", "foobar");
        att2.setListValue(true);
        const QStringList li2 {"foo", "bar", "baz"};
        att2.setValue(QVariant(li2));
        map[att2.name()] = att2;

        // --- save
        dbID id(344);
        map.save(id);

        // --- and load again
        AttributeMap m2(host);
        m2.load(id);

        QVERIFY(m2.contains("Test"));
        QVERIFY(m2.contains(att2.name()));

        Attribute a3 = m2["Test2"];
        const QStringList li = a3.value().toStringList();
        QVERIFY(li.contains("foo"));
        QVERIFY(li.contains("bar"));
        QVERIFY(li.contains("baz"));

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

