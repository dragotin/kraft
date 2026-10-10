#include <QTest>

#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <xmlindexdb.h>

#include "docdigest.h"
#include "qtestcase.h"
#include "testconfig.h"
#include "documentsaverxml.h"
#include "defaultprovider.h"
#include "kraftdoc.h"

class T_XmlIndexDb: public QObject {
    Q_OBJECT
private Q_SLOTS:

    // copies the example xml document to a temporary path that simulates the
    // storage tree of kraft.
    void initTestCase()
    {
        Q_ASSERT(_dir.isValid());

        // dir.path() returns the unique directory path
        _baseDir = DefaultProvider::self()->createV2BaseDir(_dir.path());

        const QString kraftHome{TESTS_PATH};
        QVERIFY(!kraftHome.isEmpty());
        const QString src{kraftHome+"/kraftindx.json"};
        qDebug() << "Copy kraftindx.json from" << src << "to" << _baseDir;
        QVERIFY(QFile::copy(src, _baseDir + "/kraftindx.json"));

        // generate the index
        _indx.setBasePath(_baseDir);
    }

    void checkPathesByIdent()
    {
        const QString sExp{_baseDir + "/xmldoc/2024/12/40f4e553-daf5-4e46-b172-4389cde712df"};
        {
            const QFileInfo s{ _indx.xmlPathByIdent("2-2024")};
            QCOMPARE(s.filePath(), sExp+".xml");
        }
        {
            const QFileInfo s1{ _indx.pdfPathByIdent("2-2024")};
            QCOMPARE(s1.filePath(), sExp+".pdf");
        }
    }

    void checkPathesByUuid()
    {
        const QString sExp{_baseDir + "/xmldoc/2025/05/18d3680d-4ca8-4020-b815-0e10c6517ddd"};
        {
            const QFileInfo s{ _indx.xmlPathByUuid("18d3680d-4ca8-4020-b815-0e10c6517ddd")};
            QCOMPARE(s.filePath(), sExp+".xml");
        }
        {
            const QFileInfo s1{ _indx.pdfPathByUuid("18d3680d-4ca8-4020-b815-0e10c6517ddd")};
            QCOMPARE(s1.filePath(), sExp+".pdf");
        }
    }

    void findDigest()
    {
        DocDigest d = _indx.findDigest("2025", "18d3680d-4ca8-4020-b815-0e10c6517ddd");
        QCOMPARE(d.clientAddress(), QString("Goofy Stambulchicz\n\nRönneburger Kirchweg 4\n13211 Submistatis\t"));
        QDate expD(2025,05,18);

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
        _indx.addEntry(&doc);
        QCOMPARE(XmlIndexDb::dateMap().size(), mapSize + 1);

        const QString sExp{_baseDir + "/xmldoc/2026/03/aaaaaaaa-1111-2222-3333-444444444444.xml"};
        QCOMPARE(_indx.xmlPathByIdent("77-2026").filePath(), sExp);

        // date change keeps the file path but updates the digest and the date map
        doc.setDate(QDate(2026, 4, 1));
        doc.setWhiteboard("changed");
        _indx.updateEntry(&doc);
        QCOMPARE(_indx.xmlPathByUuid(doc.uuid()).filePath(), sExp);
        QCOMPARE(XmlIndexDb::dateMap().size(), mapSize + 1);
        QVERIFY(XmlIndexDb::dateMap().contains(QDate(2026, 4, 1)));

        DocDigest d = _indx.findDigest("2026", "2026/03/" + doc.uuid());
        QCOMPARE(d.rawDate(), QDate(2026, 4, 1));
        QCOMPARE(d.ident(), QString("77-2026"));
    }

private:
    // needs to be defined according the example document in $KRAFT_HOME/xml
    QTemporaryDir _dir;
    XmlIndexDb _indx;
    QString _baseDir;
};

QTEST_MAIN(T_XmlIndexDb)

#include "t_xmlindexdb.moc"
