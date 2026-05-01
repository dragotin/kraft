#include <QTest>
#include <QStringBuilder>

#include "testconfig.h"
#include "defaultprovider.h"
#include "doctype.h"

using namespace Qt::StringLiterals;

class T_DocType : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QDir sourceDir(TESTS_PATH);
        sourceDir.cdUp();
        const QByteArray ba{sourceDir.absolutePath().toLatin1()};
        qputenv("KRAFT_HOME", ba);

        _baseDir = DefaultProvider::self()->createV2BaseDir(_dir.path());
        qDebug() << "Test Basedir:" << _baseDir;
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

        QCOMPARE(dts.save(dt, _baseDir), DocTypes::SaveResult::SaveOk);
    }

    void load1() {
        DocTypes dts;
        const auto map = dts.map();

        const DocType dt = map["TestDocType"];
        QVERIFY(!dt.modified());
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

        XmlDirLister<DocType>::Map m;
        for (int i = 1; i < 10; i++) {
            const DocType dt = create(i);
            QVERIFY(dt.modified());
            m.insert(dt.name(), dt);
        }

        QCOMPARE(dts.saveAll(m, _baseDir), DocTypes::SaveResult::SaveOk);
    }

    void checkAllNames() {
        DocTypes dts;
        const QStringList allDts = dts.allNames();

        QCOMPARE(allDts.count(), 9);

        for (int i = 0; i < 9; i++) {
            QCOMPARE(allDts.at(i), "TestDocType " % QString::number(i + 1));
        }

        _docTypeName = allDts.at(0);
        QCOMPARE(_docTypeName, u"TestDocType 1"_s);
    }

    void loadADt() {
        DocTypes dts;
        const DocType dt = dts.get(_docTypeName);
        QCOMPARE(dt.name(), u"TestDocType 1"_s);
        QVERIFY(dt.allowAlternative());
        QVERIFY(dt.allowDemand());
        QVERIFY(dt.pricesHidden());
        QVERIFY(!dt.partialInvoice());
    }

    void checkFollowers() {
        DocTypes dts;
        const DocType dt = dts.get(_docTypeName);
        QCOMPARE(dt.name(), u"TestDocType 1"_s);

        const QStringList f = dt.follower();
        QCOMPARE(f.at(0), u"Auftragsbestätigung"_s);
        QCOMPARE(f.at(1), u"Rechnung"_s);
        QCOMPARE(f.at(2), u"Teilrechnung"_s);
        QVERIFY(!f.contains("Quadratrechnung"));
    }

    void createNewDoctype() {
        const QStringList f{"Angebot", "Rechnung"};

        DocType dt;
        dt.setName("NewTestDoctype");
        dt.setMergeIdent(2);
        dt.setFollowers(f);
        QCOMPARE(dt.follower().count(), 2);

        DocTypes dts;
        QCOMPARE(dts.save(dt), DocTypes::SaveResult::SaveOk);
    }

    // get() on an unknown name returns a default-constructed DocType
    void docTypeDefaults() {
        DocTypes dts;
        const DocType dt = dts.get("NonExistent");
        QVERIFY(dt.mergeIdent() == 0);
        QVERIFY(!dt.isXRechnungEnabled());
        QVERIFY(dt.name().isEmpty());
        QVERIFY(!dt.allowAlternative());
        QVERIFY(!dt.allowDemand());
    }

private:
    QString _docTypeName;
    QString _baseDir;
    QTemporaryDir _dir;
};

QTEST_MAIN(T_DocType)
#include "t_doctype.moc"
