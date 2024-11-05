#include <QTest>
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "jsonindexfile.h"
#include "testconfig.h"
#include "kraftdb.h"
#include "kraftdoc.h"
#include "attribute.h"
#include "docposition.h"
#include "defaultprovider.h"

void init_test_db()
{
    const QString dbName("__test.db");

    QDir sourceDir(TESTS_PATH);
    sourceDir.cdUp();
    const QByteArray ba {sourceDir.absolutePath().toLatin1()};
    qputenv("KRAFT_HOME", ba);

    QFile::remove(dbName);

    KraftDB::self()->dbConnect("QSQLITE", dbName, QString(), QString(), QString());

    // create the tagTemplate table which is required by DocPositionBase::hasTag
    SqlCommandList sqls = KraftDB::self()->parseCommandFile("10_dbmigrate.sql");
    QVERIFY(sqls.size() > 0);
    KraftDB::self()->processSqlCommands(sqls);

#if 0
    sqls = KraftDB::self()->parseCommandFile("10_dbmigrate.sql");
    KraftDB::self()->processSqlCommands(sqls);

    sqls = KraftDB::self()->parseCommandFile("11_dbmigrate.sql");
    KraftDB::self()->processSqlCommands(sqls);
#endif
}

namespace {

}

class T_JsonIndexFile: public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QVERIFY(_dir.isValid());
        const QString d = _dir.path();
        const QString base = DefaultProvider::self()->createV2BaseDir(d);

        // it should not end with "current"
        QVERIFY(! (base.endsWith("current") || base.endsWith("current/")));

        init_test_db();
    }

    void cleanupTestCase() {
        const QString dbName("__test.db");

        QDir sourceDir(TESTS_PATH);
        sourceDir.cdUp();
        QVERIFY(QFile::remove(dbName));
    }

    void addDocToIndexFile() {

        KraftDoc *kraftdoc = &_kDoc;
        QDateTime modDt{QDate(2024, 1, 25), QTime(1, 46, 12, 1)};

        kraftdoc->setDocType("Rechnung");
        kraftdoc->setDate(QDate(2024, 1, 24));
        kraftdoc->setLastModified(modDt);
        kraftdoc->setIdent("20240124-1");
        kraftdoc->setProjectLabel("ProjectLabel");
        kraftdoc->state().setState(KraftDocState::State::Draft);

        const QString d = _dir.path();
        JsonIndexFile indx;
        qDebug() << "Temp dir" << d;
        indx.addDoc(kraftdoc);

        JsonIndexFile rereadIndx;
        QJsonArray j24 = rereadIndx.docsPerYear("2024");
        QCOMPARE(j24.size(), 1);
        QJsonArray j23 = rereadIndx.docsPerYear("2023");
        QCOMPARE(j23.size(), 0);
    }

    // append another doc to the index above
    void addSecondDocToIndexFile() {

        KraftDoc *kraftdoc = &_kDoc;
        QDateTime modDt{QDate(2024, 1, 15), QTime(5, 12, 10, 8)};

        kraftdoc->setDocType("Rechnung");
        kraftdoc->setDate(QDate(2024, 1, 14));
        kraftdoc->setLastModified(modDt);
        kraftdoc->setIdent("20240114-01");
        kraftdoc->setProjectLabel("Second ProjectLabel");
        kraftdoc->state().setState(KraftDocState::State::Draft);

        JsonIndexFile indx;
        indx.addDoc(kraftdoc);

        JsonIndexFile rereadIndx;
        QJsonArray j24 = rereadIndx.docsPerYear("2024");
        QCOMPARE(j24.size(), 2);
    }

    void addThirdDocToIndexFile() {

        KraftDoc *kraftdoc = &_kDoc;
        QDateTime modDt{QDate(2023, 10, 13), QTime(5, 12, 10, 8)};

        kraftdoc->setDocType("Rechnung");
        kraftdoc->setUuid("1c55c452-5527-429d-ab91-e6839888509e");
        kraftdoc->setDate(QDate(2023, 10, 12));
        kraftdoc->setLastModified(modDt);
        kraftdoc->setIdent("20231013-01");
        kraftdoc->setProjectLabel("Third ProjectLabel");
        kraftdoc->state().setState(KraftDocState::State::Draft);

        JsonIndexFile indx;
        indx.addDoc(kraftdoc);

        JsonIndexFile rereadIndx;
        QJsonArray j24 = rereadIndx.docsPerYear("2024");
        QCOMPARE(j24.size(), 2);

        QJsonArray j23 = rereadIndx.docsPerYear("2023");
        QCOMPARE(j23.size(), 1);

        QJsonObject d23 = j23.first().toObject();
        QCOMPARE(d23["ident"].toString(), QStringLiteral("20231013-01"));
    }

    void updateDoc() {
        const QString& uuid = _kDoc.uuid();  // from the last test
        QCOMPARE(uuid, "1c55c452-5527-429d-ab91-e6839888509e");

        QCOMPARE(_kDoc.state().state(), KraftDocState::State::Draft);

        _kDoc.state().setState(KraftDocState::State::Final);

        JsonIndexFile indx;
        indx.updateDoc(&_kDoc);

        JsonIndexFile rereadIndx;

        QJsonArray j23 = rereadIndx.docsPerYear("2023");
        QCOMPARE(j23.size(), 1);
        QJsonObject d23 = j23.first().toObject();
        QCOMPARE(d23["uuid"].toString(), QStringLiteral("1c55c452-5527-429d-ab91-e6839888509e"));
        QCOMPARE(d23["state"].toString(), QStringLiteral("Final"));
    }


private:
    KraftDoc _kDoc;
    QTemporaryDir _dir;

};

QTEST_MAIN(T_JsonIndexFile)
#include "t_jsonindexfile.moc"

