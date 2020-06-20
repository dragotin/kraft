#include <QTest>
#include <QObject>
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "kraftdb.h"
#include "sql_states.h"
#include "unitmanager.h"
#include "einheit.h"

void init_test_db()
{
    const QString dbName("__test.db");

    QFile::remove(dbName);

    KraftDB::self()->dbConnect("QSQLITE", dbName, QString(), QString(), QString());

    SqlCommandList sqls = KraftDB::self()->parseCommandFile("create_schema.sql");
    KraftDB::self()->processSqlCommands(sqls);

    sqls = KraftDB::self()->parseCommandFile("fill_schema_de.sql");
    KraftDB::self()->processSqlCommands(sqls);

}

class T_UnitMan : public QObject {
    Q_OBJECT

private slots:
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
};

QTEST_MAIN(T_UnitMan)
#include "t_unitman.moc"

