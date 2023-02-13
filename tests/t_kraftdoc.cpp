#include <QTest>
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "testconfig.h"
#include "kraftdb.h"
#include "kraftdoc.h"
#include "attribute.h"
#include "docposition.h"
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

DocPositionList buildPosList() {
    DocPositionList positions;

    // Attention: only use tags here that exist in the database, as
    // defined in the 10_migrate.sql
    DocPosition *dp1 = new DocPosition;
    dp1->setAmount(2.0);
    dp1->setUnitPrice(Geld(6.50));
    dp1->setText("Position1");
    dp1->setTags(QStringList{"Work"});

    positions.append(dp1);

    DocPosition *dp2 = new DocPosition;
    dp2->setAmount(4.0);
    dp2->setUnitPrice(Geld(12.50));
    dp2->setText("Position2");
    dp2->setTags(QStringList{"Work"});

    positions.append(dp2);

    DocPosition *dp3 = new DocPosition;
    dp3->setAmount(4.0);
    dp3->setUnitPrice(Geld(1.50));
    dp3->setText("Position3");
    dp3->setTags(QStringList{"Material"});

    positions.append(dp3);

    return positions;
}
}

class T_KraftDoc: public QObject {
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
        const QStringList attribCols {"sortkey", "name", "description", "color"};
        QVERIFY(KraftDB::self()->checkTableExistsSqlite("tagTemplates", attribCols));
    }

    void sumPerTag() {
        KraftDoc *kraftdoc = &kDoc;

        DocPositionList positions = buildPosList();
        double fullTax = 19.0;
        double redTax = 7.0;

        const QString tmpl{"This is a template test with NETTO_SUM_PER_TAG(Work)"};

        const QString expanded = kraftdoc->resolveMacros(tmpl, positions, _date, fullTax, redTax);

        qDebug() << "Expanded text is" << expanded;
        QString shouldBe{tmpl};
        // 0xA0 is a non splitable space, no idea how to hardcode it.
        shouldBe.replace("NETTO_SUM_PER_TAG(Work)", "63,00" + QChar(0xA0) + "€");
        QCOMPARE(expanded, shouldBe);
    }

    void sumPerTagBrutto() {
        KraftDoc *kraftdoc = &kDoc;

        DocPositionList positions = buildPosList();
        double fullTax = 19.0;
        double redTax = 7.0;

        const QString tmpl{"This is a template test with NETTO_SUM_PER_TAG(Work) and BRUTTO_SUM_PER_TAG(Work) and VAT_SUM_PER_TAG(Work)"};

        const QString expanded = kraftdoc->resolveMacros(tmpl, positions, _date, fullTax, redTax);

        qDebug() << "Expanded text is" << expanded;
        QString shouldBe{tmpl};
        // 0xA0 is a non splitable space, no idea how to hardcode it.
        shouldBe.replace("NETTO_SUM_PER_TAG(Work)", "63,00" + QChar(0xA0) + "€");
        shouldBe.replace("BRUTTO_SUM_PER_TAG(Work)", "74,97" + QChar(0xA0) + "€");
        shouldBe.replace("VAT_SUM_PER_TAG(Work)", "11,97" + QChar(0xA0) + "€");
        QCOMPARE(expanded, shouldBe);
    }

    void sumPerTagBruttoNull() {
        KraftDoc *kraftdoc = &kDoc;

        DocPositionList positions = buildPosList();
        double fullTax = 19.0;
        double redTax = 7.0;

        const QString tmpl{"This is a template test with NETTO_SUM_PER_TAG(NoWork) and BRUTTO_SUM_PER_TAG(NoWork) and VAT_SUM_PER_TAG(NoWork)"};

        const QString expanded = kraftdoc->resolveMacros(tmpl, positions, _date, fullTax, redTax);

        qDebug() << "Expanded text is" << expanded;
        QString shouldBe{tmpl};
        // 0xA0 is a non splitable space, no idea how to hardcode it.
        shouldBe.replace("NETTO_SUM_PER_TAG(NoWork)", "0,00" + QChar(0xA0) + "€");
        shouldBe.replace("BRUTTO_SUM_PER_TAG(NoWork)", "0,00" + QChar(0xA0) + "€");
        shouldBe.replace("VAT_SUM_PER_TAG(NoWork)", "0,00" + QChar(0xA0) + "€");
        QCOMPARE(expanded, shouldBe);
    }

    // Verify that the sum is zero for a pos list where no pos has that tag
    void sumPerTagNoMatch() {
        KraftDoc *kraftdoc = &kDoc;

        DocPositionList positions = buildPosList();
        double fullTax = 19.0;
        double redTax = 7.0;

        const QString tmpl{"This is a template test with NETTO_SUM_PER_TAG(Plants)"};
        const QString expanded = kraftdoc->resolveMacros(tmpl, positions, _date, fullTax, redTax);

        QString shouldBe{tmpl};
        // 0xA0 is a non splitable space, no idea how to hardcode it.
        shouldBe.replace("NETTO_SUM_PER_TAG(Plants)", "0,00" + QChar(0xA0) + "€");
        QCOMPARE(expanded, shouldBe);
    }

    void ifHasTag() {
        KraftDoc *kraftdoc = &kDoc;

        DocPositionList positions = buildPosList();
        double fullTax = 19.0;
        double redTax = 7.0;

        const QString tmpl{"This template IF_ANY_HAS_TAG(Work) has the tag Work END_HAS_TAG"};
        const QString expanded = kraftdoc->resolveMacros(tmpl, positions, _date, fullTax, redTax);

        QString shouldBe{"This template has the tag Work"};
        // 0xA0 is a non splitable space, no idea how to hardcode it.
        QCOMPARE(expanded, shouldBe);
    }

    void ifHasTagNoEnd() {
        KraftDoc *kraftdoc = &kDoc;

        DocPositionList positions = buildPosList();
        double fullTax = 19.0;
        double redTax = 7.0;

        const QString tmpl{"This template IF_ANY_HAS_TAG(Work) has the tag Work without an end"};
        const QString expanded = kraftdoc->resolveMacros(tmpl, positions, _date, fullTax, redTax);

        QString shouldBe{"This template has the tag Work without an end"};
        // 0xA0 is a non splitable space, no idea how to hardcode it.
        QCOMPARE(expanded, shouldBe);
    }

    void ifHasTagDoesNotHaveTheTag() {
        KraftDoc *kraftdoc = &kDoc;

        DocPositionList positions = buildPosList();
        double fullTax = 19.0;
        double redTax = 7.0;

        const QString tmpl{"This template IF_ANY_HAS_TAG(Plants) has the tag Work END_HAS_TAG"};
        const QString expanded = kraftdoc->resolveMacros(tmpl, positions, _date, fullTax, redTax);

        QString shouldBe{"This template"};
        // 0xA0 is a non splitable space, no idea how to hardcode it.
        QCOMPARE(expanded, shouldBe);
    }

    void amountOfTag() {
        KraftDoc *kraftdoc = &kDoc;

        DocPositionList positions = buildPosList();
        double fullTax = 19.0;
        double redTax = 7.0;

        const QString tmpl{"This template has ITEM_COUNT_WITH_TAG(Work) work items"};
        const QString expanded = kraftdoc->resolveMacros(tmpl, positions, _date, fullTax, redTax);

        QString shouldBe{"This template has 2 work items"};
        // 0xA0 is a non splitable space, no idea how to hardcode it.
        QCOMPARE(expanded, shouldBe);
    }

    void dateAddDay() {
        KraftDoc *kraftdoc = &kDoc;

        DocPositionList positions = buildPosList();
        QDate d{2020, 1, 24};
        double fullTax = 19.0;
        double redTax = 7.0;

        QString tmpl{"This is 12 days later than 24.01.2020: DATE_ADD_DAYS(12)"};
        QString expanded = kraftdoc->resolveMacros(tmpl, positions, d, fullTax, redTax);

        QString shouldBe{"This is 12 days later than 24.01.2020: 05.02.2020"};
        QCOMPARE(expanded, shouldBe);

        tmpl = "This is 0 days later than 24.01.2020: DATE_ADD_DAYS(0)";
        expanded = kraftdoc->resolveMacros(tmpl, positions, d, fullTax, redTax);
        shouldBe = "This is 0 days later than 24.01.2020: 24.01.2020";
        QCOMPARE(expanded, shouldBe);

        tmpl = "This is -5 days later than 24.01.2020: DATE_ADD_DAYS(-5)";
        expanded = kraftdoc->resolveMacros(tmpl, positions, d, fullTax, redTax);
        shouldBe = "This is -5 days later than 24.01.2020: 19.01.2020";
        QCOMPARE(expanded, shouldBe);
    }

private:
    KraftDoc kDoc;
    QDate _date;

};

QTEST_MAIN(T_KraftDoc)
#include "t_kraftdoc.moc"

