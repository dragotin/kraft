#include <QAbstractItemModelTester>
#include <QTest>

#include <KLocalizedString>

#include "testconfig.h"
#include "vatreport.h"
#include "vatreportwindow.h"
#include "models/vatreportmodel.h"
#include "kraftdoc.h"
#include "docposition.h"

namespace {

DocPosition *addPos(KraftDoc *doc, double amount, double price, DocPosition::Tax tax)
{
    DocPosition *dp = doc->createPosition(DocPosition::Type::Position);
    dp->setAmount(amount);
    dp->setUnitPrice(Geld(price));
    dp->setTaxType(tax);
    return dp;
}

KraftDoc *createInvoice(const QDate &date, const QDate &tosStart = QDate(), const QDate &tosEnd = QDate())
{
    KraftDoc *doc = new KraftDoc;
    doc->setDate(date);
    doc->setTaxValues(19.0, 7.0);
    doc->state().setState(KraftDocState::State::Final);
    if (tosStart.isValid()) {
        doc->setTimeOfSupply(QDateTime(tosStart, QTime(0, 0)), tosEnd.isValid() ? QDateTime(tosEnd, QTime(0, 0)) : QDateTime());
    }
    return doc;
}

} // namespace

class T_VatReport : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        // find the views/vatreport.gtmpl in the source dir
        QDir sourceDir(TESTS_PATH);
        sourceDir.cdUp();
        qputenv("KRAFT_HOME", sourceDir.absolutePath().toLatin1());
        QLocale::setDefault(QLocale::German);
        KLocalizedString::setApplicationDomain("kraft");
    }

    void entrySums()
    {
        std::unique_ptr<KraftDoc> doc{createInvoice(QDate(2026, 3, 10))};
        addPos(doc.get(), 2.0, 50.0, DocPosition::Tax::Full);    // 100,00
        addPos(doc.get(), 1.0, 10.0, DocPosition::Tax::Reduced); //  10,00
        addPos(doc.get(), 1.0, 5.0, DocPosition::Tax::None);     //   5,00
        doc->createPosition(DocPosition::Type::Text);            // no price

        const VatReport::DocEntry e = VatReport::entryFromDoc(doc.get());
        QCOMPARE(e.netFull.toLong(), 10000L);
        QCOMPARE(e.taxFull.toLong(), 1900L);
        QCOMPARE(e.netReduced.toLong(), 1000L);
        QCOMPARE(e.taxReduced.toLong(), 70L);
        QCOMPARE(e.netNoTax.toLong(), 500L);
        QCOMPARE(e.netUnknownTax.toLong(), 0L);
        QVERIFY(e.dateFromInvoice);
    }

    void taxDateFromTimeOfSupply()
    {
        bool fromInvoice{true};

        // supply period end determines the tax period
        std::unique_ptr<KraftDoc> doc1{createInvoice(QDate(2026, 2, 3), QDate(2026, 1, 1), QDate(2026, 1, 31))};
        QCOMPARE(VatReport::taxDate(doc1.get(), &fromInvoice), QDate(2026, 1, 31));
        QVERIFY(!fromInvoice);

        // single day of supply
        std::unique_ptr<KraftDoc> doc2{createInvoice(QDate(2026, 2, 3), QDate(2025, 12, 20))};
        QCOMPARE(VatReport::taxDate(doc2.get(), &fromInvoice), QDate(2025, 12, 20));
        QVERIFY(!fromInvoice);

        // fallback to invoice date
        std::unique_ptr<KraftDoc> doc3{createInvoice(QDate(2026, 2, 3))};
        QCOMPARE(VatReport::taxDate(doc3.get(), &fromInvoice), QDate(2026, 2, 3));
        QVERIFY(fromInvoice);
    }

    void monthlyPeriods()
    {
        VatReport report(VatReport::PeriodType::Month);

        std::unique_ptr<KraftDoc> doc1{createInvoice(QDate(2026, 2, 3), QDate(2026, 1, 5), QDate(2026, 1, 20))};
        addPos(doc1.get(), 1.0, 100.0, DocPosition::Tax::Full);
        std::unique_ptr<KraftDoc> doc2{createInvoice(QDate(2026, 1, 28))};
        addPos(doc2.get(), 1.0, 200.0, DocPosition::Tax::Full);
        std::unique_ptr<KraftDoc> doc3{createInvoice(QDate(2026, 3, 2))};
        addPos(doc3.get(), 1.0, 50.0, DocPosition::Tax::Reduced);

        report.addDocument(doc3.get());
        report.addDocument(doc1.get());
        report.addDocument(doc2.get());

        const auto periods = report.periods();
        QCOMPARE(periods.size(), 2);

        QCOMPARE(periods.at(0).period, 1);
        QCOMPARE(periods.at(0).docs.size(), 2);
        QCOMPARE(periods.at(0).netFull.toLong(), 30000L);
        QCOMPARE(periods.at(0).taxFull.toLong(), 5700L);
        QCOMPARE(periods.at(0).firstDay(), QDate(2026, 1, 1));
        QCOMPARE(periods.at(0).lastDay(), QDate(2026, 1, 31));

        QCOMPARE(periods.at(1).period, 3);
        QCOMPARE(periods.at(1).taxReduced.toLong(), 350L);

        const VatReport::PeriodSum total = report.total();
        QCOMPARE(total.taxSum().toLong(), 6050L);
        QCOMPARE(total.docs.size(), 3);
    }

    void quarterlyPeriods()
    {
        VatReport report(VatReport::PeriodType::Quarter);

        std::unique_ptr<KraftDoc> doc1{createInvoice(QDate(2026, 3, 31))};
        addPos(doc1.get(), 1.0, 100.0, DocPosition::Tax::Full);
        std::unique_ptr<KraftDoc> doc2{createInvoice(QDate(2026, 4, 1))};
        addPos(doc2.get(), 1.0, 100.0, DocPosition::Tax::Full);

        report.addDocument(doc1.get());
        report.addDocument(doc2.get());

        const auto periods = report.periods();
        QCOMPARE(periods.size(), 2);
        QCOMPARE(periods.at(0).period, 1);
        QCOMPARE(periods.at(1).period, 2);
        QCOMPARE(periods.at(1).firstDay(), QDate(2026, 4, 1));
        QCOMPARE(periods.at(1).lastDay(), QDate(2026, 6, 30));

        const auto all = report.periodsOfYear(2026);
        QCOMPARE(all.size(), 4);
        QCOMPARE(all.at(0).docs.size(), 1);
        QCOMPARE(all.at(2).docs.size(), 0);
        QCOMPARE(all.at(3).period, 4);
        QCOMPARE(all.at(3).taxSum().toLong(), 0L);
    }

    void negativeInvoice()
    {
        // a cancellation reduces the VAT of the period
        std::unique_ptr<KraftDoc> doc{createInvoice(QDate(2026, 5, 4))};
        addPos(doc.get(), 1.0, -100.0, DocPosition::Tax::Full);

        const VatReport::DocEntry e = VatReport::entryFromDoc(doc.get());
        QCOMPARE(e.netFull.toLong(), -10000L);
        QCOMPARE(e.taxFull.toLong(), -1900L);
    }

    void renderHtml()
    {
        VatReport report(VatReport::PeriodType::Quarter);

        std::unique_ptr<KraftDoc> doc1{createInvoice(QDate(2026, 2, 3), QDate(2026, 1, 5), QDate(2026, 1, 20))};
        doc1->setIdent(QStringLiteral("RE-2026-0001"));
        addPos(doc1.get(), 1.0, 100.0, DocPosition::Tax::Full);
        std::unique_ptr<KraftDoc> doc2{createInvoice(QDate(2026, 8, 12))};
        doc2->setIdent(QStringLiteral("RE-2026-0002"));
        addPos(doc2.get(), 1.0, 50.0, DocPosition::Tax::Reduced);

        report.addDocument(doc1.get());
        report.addDocument(doc2.get());

        const QString html = VatReportWindow::reportHtml(report, 2026);
        QVERIFY(html.contains(QStringLiteral("RE-2026-0001")));
        QVERIFY(html.contains(QStringLiteral("RE-2026-0002")));
        QVERIFY(html.contains(QStringLiteral("Q3/2026")));
        QVERIFY(html.contains(QStringLiteral("kraft:document?uuid=")));
        // Q1 and Q3 have documents, so there are anchors for them, but not for Q2
        QVERIFY(html.contains(QStringLiteral("href=\"#p1\"")));
        QVERIFY(!html.contains(QStringLiteral("href=\"#p2\"")));
        // the tax of the year: 19,00 + 3,50
        QVERIFY(html.contains(QStringLiteral("22,50")));
    }

    void treeModel()
    {
        VatReport report(VatReport::PeriodType::Month);

        std::unique_ptr<KraftDoc> doc1{createInvoice(QDate(2026, 2, 3), QDate(2026, 1, 5), QDate(2026, 1, 20))};
        doc1->setIdent(QStringLiteral("RE-1"));
        addPos(doc1.get(), 1.0, 100.0, DocPosition::Tax::Full);
        std::unique_ptr<KraftDoc> doc2{createInvoice(QDate(2026, 1, 10))};
        doc2->setIdent(QStringLiteral("RE-2"));
        addPos(doc2.get(), 1.0, 200.0, DocPosition::Tax::Full);
        std::unique_ptr<KraftDoc> doc3{createInvoice(QDate(2026, 3, 2))};
        doc3->setIdent(QStringLiteral("RE-3"));
        addPos(doc3.get(), 1.0, 50.0, DocPosition::Tax::Reduced);
        std::unique_ptr<KraftDoc> doc4{createInvoice(QDate(2026, 11, 30))};
        doc4->setIdent(QStringLiteral("RE-4"));
        addPos(doc4.get(), 1.0, 10.0, DocPosition::Tax::Full);
        // other year, must not show up
        std::unique_ptr<KraftDoc> doc5{createInvoice(QDate(2025, 12, 30))};
        addPos(doc5.get(), 1.0, 10.0, DocPosition::Tax::Full);

        for (KraftDoc *d : {doc1.get(), doc2.get(), doc3.get(), doc4.get(), doc5.get()}) {
            report.addDocument(d);
        }

        VatReportModel model;
        QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
        model.setReport(report, 2026);

        QCOMPARE(model.rowCount(), 4);
        QCOMPARE(model.columnCount(), int(VatReportModel::ColumnCount));

        const QModelIndex q1 = model.index(0, 0);
        QCOMPARE(model.nodeType(q1), VatReportModel::NodeType::Quarter);
        QCOMPARE(model.rowCount(q1), 3);
        QCOMPARE(model.index(0, VatReportModel::DocCount).data().toString(), QStringLiteral("3"));
        // 19,00 + 38,00 + 3,50
        QCOMPARE(model.index(0, VatReportModel::TaxSum).data(VatReportModel::SortRole).toLongLong(), 6050LL);

        // January: two invoices, sorted by tax date
        const QModelIndex jan = model.index(0, 0, q1);
        QCOMPARE(model.nodeType(jan), VatReportModel::NodeType::Month);
        QCOMPARE(model.rowCount(jan), 2);
        QCOMPARE(model.index(0, VatReportModel::Label, jan).data().toString(), QStringLiteral("RE-2"));
        QCOMPARE(model.index(1, VatReportModel::Label, jan).data().toString(), QStringLiteral("RE-1"));
        QCOMPARE(model.index(0, VatReportModel::TaxSum, q1).data(VatReportModel::SortRole).toLongLong(), 5700LL);

        // February is empty
        const QModelIndex feb = model.index(1, 0, q1);
        QCOMPARE(model.rowCount(feb), 0);
        QCOMPARE(model.index(1, VatReportModel::DocCount, q1).data().toString(), QStringLiteral("0"));

        // documents
        const QModelIndex docIdx = model.index(1, 0, jan);
        QCOMPARE(model.nodeType(docIdx), VatReportModel::NodeType::Document);
        QCOMPARE(model.uuid(docIdx), doc1->uuid());
        QCOMPARE(model.rowCount(docIdx), 0);
        QCOMPARE(model.parent(docIdx), jan);
        QCOMPARE(model.parent(jan), q1);

        // Q4 -> November
        const QModelIndex q4 = model.index(3, 0);
        const QModelIndex nov = model.index(1, 0, q4);
        QCOMPARE(model.rowCount(nov), 1);
        QCOMPARE(model.index(0, VatReportModel::Label, nov).data().toString(), QStringLiteral("RE-4"));

        // Q2 and Q3 are empty
        QCOMPARE(model.index(1, VatReportModel::DocCount).data().toString(), QStringLiteral("0"));
        QCOMPARE(model.index(2, VatReportModel::DocCount).data().toString(), QStringLiteral("0"));

        model.clear();
        QCOMPARE(model.rowCount(), 0);
    }

    void relevantStates()
    {
        QVERIFY(VatReport::countsForVat(KraftDocState::State::Final));
        QVERIFY(VatReport::countsForVat(KraftDocState::State::Retracted));
        QVERIFY(!VatReport::countsForVat(KraftDocState::State::Draft));
        QVERIFY(!VatReport::countsForVat(KraftDocState::State::Invalid));
        QVERIFY(!VatReport::countsForVat(KraftDocState::State::New));
    }
};

QTEST_MAIN(T_VatReport)

#include "t_vatreport.moc"
