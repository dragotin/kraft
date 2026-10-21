#include <QTest>

#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <optional>
#include <QtEnvironmentVariables>

#include <xmlindexdb.h>

#include "docdigest.h"
#include "qtestcase.h"
#include "testconfig.h"
#include "documentsaverxml.h"
#include "kraftdoc.h"
#include "defaultprovider.h"

class T_XmlIndexDb: public QObject {
    Q_OBJECT
private Q_SLOTS:

    // copies the example xml document to a temporary path that simulates the
    // storage tree of kraft.
    void initTestCase()
    {
        Q_ASSERT(_dir.isValid());

        // dir.path() returns the unique directory path

        //        const QString kraftHome{TESTS_PATH};
        //        QVERIFY(!kraftHome.isEmpty());

        QProcess p;
        p.start("../bin/kraftdemodata", {"--lang", "de", _dir.path()}, QIODeviceBase::ReadOnly);
        p.waitForStarted();
        p.waitForFinished();

        int ex = p.exitCode();
        QCOMPARE(ex, 0);

        // read the output of the demo data script
        QByteArray all = p.readAllStandardOutput();
        qDebug() << "** all:" << all;
        Q_ASSERT(!all.isEmpty());

        // read until ": " from the back
        const QByteArray token{"Kraft base dir: "};
        int pos = all.lastIndexOf(token);

        QVERIFY(pos > 0);
        pos += token.length();
        QVERIFY(pos < all.length());

        // ...and cut out the test dir
        _baseDir = all.right(all.length() - pos);
        _baseDir.chop(1);
        qDebug() << "Base dir:" << _baseDir;
        // kraftdemodata ran in another process, register its dir as current one (writes Kraft2.ini into _dir)
        qputenv("KRAFT_DATA_DIR", _dir.path().toUtf8());
        QVERIFY(DefaultProvider::self()->switchToV2BaseDir(_baseDir));
        // construct only now, otherwise the index opens in the users real data dir
        _indx.emplace();
    }

    void checkPathesByIdent()
    {
        const QString sExp{_baseDir + "/xmldoc/2026/06/181d923a-365b-5ae0-a8f5-5462603603c4"};
        {
            const QFileInfo s{ _indx->xmlPathByIdent("202625-9001")};
            QCOMPARE(s.filePath(), sExp+".xml");
        }
        {
            const QFileInfo s1{ _indx->pdfPathByIdent("202625-9001")};
            QCOMPARE(s1.filePath(), sExp+".pdf");
        }
    }

    void checkPathesByUuid()
    {
        const QString sExp{_baseDir + "/xmldoc/2026/06/18dd2db9-4b14-5cbc-aee5-8ad02ad0063d"};
        {
            const QFileInfo s{ _indx->xmlPathByUuid("18dd2db9-4b14-5cbc-aee5-8ad02ad0063d")};
            QCOMPARE(s.filePath(), sExp+".xml");
        }
        {
            const QFileInfo s1{ _indx->pdfPathByUuid("18dd2db9-4b14-5cbc-aee5-8ad02ad0063d")};
            QCOMPARE(s1.filePath(), sExp+".pdf");
        }
    }

    void findDigest()
    {
        DocDigest d = _indx->findDigest("2026", "18dd2db9-4b14-5cbc-aee5-8ad02ad0063d");
        QCOMPARE(d.clientAddress(), QString("Familie\nJulia und Markus Becker\nAm Oberberg 12\n91126 Schwabach"));
        QDate expD(2026,06,30);

        QCOMPARE(d.rawDate(), expD);
    }

    void addAndUpdateEntry()
    {
        KraftDoc doc;
        doc.setDocType("Rechnung");
        doc.setDate(QDate(2026, 3, 7));
        doc.setIdent("77-2026");
        doc.setUuid("aaaaaaaa-1111-2222-3333-444444444444");
        const int mapSize = XmlIndexDb::dateMap().size();
        _indx->addEntry(&doc);
        QCOMPARE(XmlIndexDb::dateMap().size(), mapSize + 1);

        const QString sExp{_baseDir + "/xmldoc/2026/03/aaaaaaaa-1111-2222-3333-444444444444.xml"};
        QCOMPARE(_indx->xmlPathByIdent("77-2026").filePath(), sExp);

        // date change moves the path to the new month and updates the digest and the date map
        doc.setDate(QDate(2026, 4, 1));
        doc.setWhiteboard("changed");
        _indx->updateEntry(&doc);
        QCOMPARE(_indx->xmlPathByUuid(doc.uuid()).filePath(), _baseDir + "/xmldoc/2026/04/aaaaaaaa-1111-2222-3333-444444444444.xml");
        QCOMPARE(XmlIndexDb::dateMap().size(), mapSize + 1);
        QVERIFY(XmlIndexDb::dateMap().contains(QDate(2026, 4, 1)));
        QVERIFY(!XmlIndexDb::dateMap().contains(QDate(2026, 3, 7)));

        DocDigest d = _indx->findDigest("2026", "2026/04/" + doc.uuid());
        QCOMPARE(d.rawDate(), QDate(2026, 4, 1));
        QCOMPARE(d.ident(), QString("77-2026"));
    }

private:
    // needs to be defined according the example document in $KRAFT_HOME/xml
    QTemporaryDir _dir;
    std::optional<XmlIndexDb> _indx;
    QString _baseDir;
};

QTEST_MAIN(T_XmlIndexDb)

#include "t_xmlindexdb.moc"
