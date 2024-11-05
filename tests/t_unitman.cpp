#include <QTest>
#include <QObject>
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "testconfig.h"
#include "kraftdb.h"
#include "sql_states.h"
#include "unitmanager.h"
#include "einheit.h"

void init_test_db()
{
    const QString dbName("__test.db");

    QFile::remove(dbName);

    KraftDB::self()->dbConnect("QSQLITE", dbName, QString(), QString(), QString());

    QDir sourceDir(TESTS_PATH);
    sourceDir.cdUp();
    const QByteArray ba {sourceDir.absolutePath().toLatin1()};
    qputenv("KRAFT_HOME", ba);

    SqlCommandList sqls = KraftDB::self()->parseCommandFile("create_schema.sql");
    KraftDB::self()->processSqlCommands(sqls);

    sqls = KraftDB::self()->parseCommandFile("fill_schema_de.sql");
    KraftDB::self()->processSqlCommands(sqls);

    // This adds a migration from file 24_dbmigrate.sql to the units table. Unfortunately the
    // migration file cannot be used directly here, because it is only found in a setup where
    // KRAFT_HOME is defined or the 24_dbmigrate.sql is installed in the system.
    // For simplification we do that manually here.
    sqls.clear();
    sqls.append(SqlCommand("ALTER TABLE  units ADD COLUMN ec20 VARCHAR(10);", "", false));
    sqls.append(SqlCommand("UPDATE units set ec20 = \"MTR\" WHERE unitShort = \"m\";", "", false));
    KraftDB::self()->processSqlCommands(sqls);
}

class T_UnitMan : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        init_test_db();
    }

    void checkUnitsLoaded()
    {
        QStringList units = UnitManager::self()->allUnits();
        QVERIFY(units.count() > 0);
        qDebug() << "ALL Units:" << units;
    }

    void checkPauschUnit()
    {
        Einheit e = UnitManager::self()->getPauschUnit();
        QCOMPARE(e.einheitSingular(), QStringLiteral("pausch."));
    }

    void checkECE20()
    {
        auto u = UnitManager::self()->getECE20("m");
        QCOMPARE(u, "MTR");
    }

};

QTEST_MAIN(T_UnitMan)
#include "t_unitman.moc"

