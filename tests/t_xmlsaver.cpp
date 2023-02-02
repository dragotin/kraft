#include <QTest>

#include <QTemporaryDir>
#include <QFile>

#include "testconfig.h"
#include "kraftdoc.h"
#include "documentsaverxml.h"

class T_XmlSaver: public QObject {
    Q_OBJECT
private slots:

    // copies the example xml document to a temporary path that simulates the
    // storage tree of kraft.
    void initTestCase()
    {
        Q_ASSERT(_dir.isValid());

        // dir.path() returns the unique directory path
        QDir d {_dir.path()};
        const QString docDir {d.path()+_subdir};
        Q_ASSERT(d.mkpath(docDir));
        const QString docPath = QString("%1/%2.xml").arg(docDir).arg(_docIdent);
        qDebug() << "Example document path: " << docPath;

        const QString kraftHome{TESTS_PATH};
        QVERIFY(!kraftHome.isEmpty());
        const QString src{kraftHome+"/../xml/kraftdoc.xml"};
        QFile::copy(src, docPath);

        QFileInfo fi(docPath);
        Q_ASSERT(fi.exists());
    }

    void xmlVerify()
    {
        DocumentSaverXML xmlSaver;
        xmlSaver.setBasePath(_dir.path());

        const QString kraftHome{TESTS_PATH};
        QVERIFY(!kraftHome.isEmpty());
        const QString src{kraftHome+"/../xml/kraftdoc.xml"};

        const QString schema{kraftHome+"/../xml/kraftdoc.xsd"};
        const QUrl schemaFile = QUrl::fromLocalFile(schema);
        QVERIFY(xmlSaver.verifyXmlFile(schemaFile, src));

    }

    void load1()
    {
        DocumentSaverXML xmlSaver;
        xmlSaver.setBasePath(_dir.path());

        KraftDoc doc;
        QCOMPARE(doc.state(), KraftDoc::State::New);

        QVERIFY(xmlSaver.loadByIdent(_docIdent, &doc));

        QCOMPARE(doc.ident(), "20110127");
        QCOMPARE(doc.docType(), "Rechnung");
        QCOMPARE(doc.whiteboard(), "whiteboard content");
        QCOMPARE(doc.uuid(), "f4deb784-6131-4e49-bd6d-92bd1355ea4d");
        QCOMPARE(doc.state(), KraftDoc::State::Sent);
        QCOMPARE(doc.date(), QDate(2011, 1, 27));
        QCOMPARE(doc.timeOfSupplyStart(), QDateTime(QDate(2019, 11, 21), QTime(0,0)));
        QCOMPARE(doc.timeOfSupplyEnd(), QDateTime(QDate(2019, 11, 23), QTime(23,59, 59)));
        QCOMPARE(doc.owner(), "kf");
        QCOMPARE(doc.lastModified(), QDateTime::fromString("2018-12-15T18:22:20", Qt::ISODate));
        QCOMPARE(doc.predecessor(), "id");

        QVERIFY(!doc.isNew());

    }

private:
    QString _docTypeName;
    // needs to be defined according the example document in $KRAFT_HOME/xml
    const QString _subdir {"/2011/01"};
    const QString _docIdent {"20110127"};
    QTemporaryDir _dir;

};

QTEST_MAIN(T_XmlSaver)

#include "t_xmlsaver.moc"
