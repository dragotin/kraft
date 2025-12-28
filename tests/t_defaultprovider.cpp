#include <QTest>
#include <QObject>
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QSettings>

#include "defaultprovider.h"


namespace {
void createTestFile( const QString& testFile, const QString& content = QString())
{
    QString c {content};

    if (c.isEmpty() ) {
        c = QLatin1String("This is a super fancy test file");
    }

    QFile file(testFile);
    QVERIFY (file.open(QIODevice::WriteOnly));
    {
        QTextStream stream(&file);
        stream << c << Qt::endl;
    }
}

}

class T_Defaultprovider: public QObject {
    Q_OBJECT

private:
    QString _systemDir;

private Q_SLOTS:
    void initTestCase()
    {
        const QStringList dirs = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
        const QString home = QDir::homePath();
        const QString appName = qAppName();
        const QString testDir = QString("%1/.local/share/%2").arg(home, appName);

        int indx = dirs.indexOf(testDir);
        QVERIFY(indx > -1);
        _systemDir = dirs.at(indx);

        QDir td(_systemDir + "/styles");
        if (!td.exists())
            td.mkpath(td.path());
        QVERIFY(td.exists());
    }

    void cleanupTestCase()
    {
        QDir td(_systemDir);
        QVERIFY(td.removeRecursively());
    }

    void testLocateFileSytemPath()
    {
        QVERIFY(!_systemDir.isEmpty());

        // KRAFT_HOME has to be empty for this check
        const QString kraftHome = QString::fromUtf8(qgetenv( "KRAFT_HOME" ));
        QVERIFY(kraftHome.isEmpty());

        // first, check if it is installed in the system
        const QString fullTestFile = QString("%1/styles/docoverview.css").arg(_systemDir);

        createTestFile(fullTestFile);

        QVERIFY(QFile::exists(fullTestFile));

        const QString foundFile = DefaultProvider::self()->locateFile("styles/docoverview.css");

        QCOMPARE(foundFile, fullTestFile);

        const QString noExistFile = DefaultProvider::self()->locateFile("styles/nothere");
        QVERIFY(noExistFile.isEmpty());

        const QString noExistFile2 = DefaultProvider::self()->locateFile("nostyles/docoverview.css");
        QVERIFY(noExistFile2.isEmpty());

    }

    // This test sets the environment var KRAFT_HOME to a dir under the systemDir,
    // and checks if the filename is returned correctly.
    void testLocateWithKraftHome()
    {
        QVERIFY(!_systemDir.isEmpty());

        const QString fullTestFile = QString("%1/krafthome/mydir/kraftfile").arg(_systemDir);

        QDir td(QString("%1/krafthome/mydir").arg(_systemDir));
        if (!td.exists()) {
            td.mkpath(td.path());
        }
        QVERIFY(td.exists());
        createTestFile(fullTestFile);

        // KRAFT_HOME should be empty for this check
        const QString kraftHome = QString::fromUtf8(qgetenv( "KRAFT_HOME" ));
        QVERIFY(kraftHome.isEmpty());

        QByteArray kh {_systemDir.toUtf8()};
        kh.append("/krafthome");
        qputenv("KRAFT_HOME", kh);

        const QString foundFile = DefaultProvider::self()->locateFile("mydir/kraftfile");
        QCOMPARE(foundFile, fullTestFile);

        const QString noExistFile = DefaultProvider::self()->locateFile("mydir/nothere");
        QVERIFY(noExistFile.isEmpty());

        const QString noExistFile2 = DefaultProvider::self()->locateFile("nomydir/kraftfile");
        QVERIFY(noExistFile2.isEmpty());

        qunsetenv("KRAFT_HOME");
    }

    // test relative to the app dir
    void testLocateRelative()
    {
        QDir::setCurrent(QCoreApplication::applicationDirPath());

        // Is it possible to create a kraft-dir ?
        QFileInfo fi("../share");
        if (!fi.exists()) {
            const QString mydir{"../share/kraft/mydir"};
            QDir td(mydir);
            const QString abso = td.absolutePath();
            QVERIFY(td.mkpath(abso));

            const QString fullTestFile = QString("%1/kraftfile").arg(abso);
            createTestFile(fullTestFile);

            QFileInfo fi(fullTestFile);

            const QString foundFile = DefaultProvider::self()->locateFile("mydir/kraftfile");
            QCOMPARE(foundFile, fi.canonicalFilePath());

            const QString noExistFile = DefaultProvider::self()->locateFile("mydir/nothere");
            QVERIFY(noExistFile.isEmpty());

            const QString noExistFile2 = DefaultProvider::self()->locateFile("nomydir/kraftfile");
            QVERIFY(noExistFile2.isEmpty());

            QVERIFY(td.removeRecursively());
        } else {
            qDebug() << "Skipped relative path test, directory share exists.";
        }
    }

    void testKraftV2Dirs()
    {
        QTemporaryDir dir;
        dir.setAutoRemove(true);

        const QString p = dir.path();
        const QString base = DefaultProvider::self()->createV2BaseDir(p);
        QVERIFY(base.startsWith(p));
        QVERIFY(base.length() == p.length()+6); // it has the a uuid partikel appended

        QString pRoot = DefaultProvider::self()->kraftV2Dir(DefaultProvider::KraftV2Dir::Root);
        const QString pNumC = DefaultProvider::self()->kraftV2Dir(DefaultProvider::KraftV2Dir::NumberCycles);
        const QString pXmlD = DefaultProvider::self()->kraftV2Dir(DefaultProvider::KraftV2Dir::XmlDocs);
        const QString pOwnI = DefaultProvider::self()->kraftV2Dir(DefaultProvider::KraftV2Dir::OwnIdentity);
        QCOMPARE(base, pOwnI);
        QCOMPARE(base, pRoot);
        QCOMPARE(base + "/numbercycles", pNumC);
        QCOMPARE(base + "/xmldoc", pXmlD);
    }

    void createNewBaseDir()
    {
        QTemporaryDir _dir;
        _dir.setAutoRemove(true);

        const QString p{ _dir.path() + "/v2"};
        QDir d{p};
        QVERIFY(d.mkpath(p));

        const QString newDir = DefaultProvider::self()->createV2BaseDir(p);
        QVERIFY(newDir.startsWith(p));
        qDebug() << "New directory:" << newDir;
        bool ok = DefaultProvider::self()->switchToV2BaseDir(newDir);
        QVERIFY(ok);

        QSettings settings(p + "/Kraft2.ini", QSettings::NativeFormat);
        const QString frag = settings.value("Global/currentDir", "NOT_FOUND").toString();
        QVERIFY(frag != "NOT_FOUND");
        QFileInfo fi{p, frag};
        QVERIFY(fi.exists());
        QVERIFY(fi.isDir());
    }
};

QTEST_MAIN(T_Defaultprovider)
#include "t_defaultprovider.moc"

