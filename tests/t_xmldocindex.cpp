#include <QTest>

#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <xmldocindex.h>

#include "docdigest.h"
#include "qtestcase.h"
#include "testconfig.h"
#include "documentsaverxml.h"
#include "defaultprovider.h"

class T_XmlDocIndex: public QObject {
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

private:
    // needs to be defined according the example document in $KRAFT_HOME/xml
    QTemporaryDir _dir;
    XmlDocIndex _indx;
    QString _baseDir;
};

QTEST_MAIN(T_XmlDocIndex)

#include "t_xmldocindex.moc"
