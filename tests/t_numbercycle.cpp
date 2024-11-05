#include <QTest>
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "testconfig.h"
#include "numbercycle.h"
#include "defaultprovider.h"

namespace {

bool directFileChange(const QString& thefile, int from, int to) {

    QFile file(thefile);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Unable to open xml document file";
        return false;
    }

    QByteArray arr = file.readAll();
    file.close();

    QByteArray old{"lastNumber>"};
    QByteArray n;
    n.setNum(from);
    old.append(n);
    old.append("</last");

    QByteArray nnew{"lastNumber>"};
    n.setNum(to);
    nnew.append(n);
    nnew.append("</last");
    arr.replace(old, nnew);

    QFile fileW(thefile);
    if (!fileW.open(QIODevice::WriteOnly)) {
        qDebug() << "Unable to open xml document file";
        return false;
    }
    bool re = fileW.write(arr) > 0;
    fileW.close();
    return(re);
}

}

class T_NumberCycle: public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        _baseDir = DefaultProvider::self()->createV2BaseDir(_dir.path());

        // FIXME: Get rid of the basedir stuff here, and hold the base dir in the
        // DefaultProvider
        // Use this method to create a temp base dir: DefaultProvider::self()->switchToV2BaseDir(d);
        qDebug() << "Test Basedir:" << _baseDir;
        // in the base path there needs to be a directory called 'current'
    }

    void saveTest()
    {
        NumberCycle nc;
        nc.setCounter(121);
        nc.setName("TestCycle1");
        nc.setTemplate("TEST1-%y-%w-%d-%i");

        auto res = _ncs.addUpdate(nc);
        QCOMPARE(res, NumberCycles::SaveResult::SaveOk);
    }

    void addAnotherNC()
    {
        NumberCycle nc;
        nc.setCounter(12);
        nc.setName("TestCycle2");
        nc.setTemplate("TEST2-%y-%w-%d-%i");
        auto res = _ncs.addUpdate(nc);
        QCOMPARE(res, NumberCycles::SaveResult::SaveOk);
    }

    void updateNC()
    {
        NumberCycle nc = _ncs.get("TestCycle2");

        nc.setTemplate("NEWTEST2-%y-%w-%i");
        auto res = _ncs.addUpdate(nc);
        QCOMPARE(res, NumberCycles::SaveResult::SaveOk);

        NumberCycle nc2 = _ncs.get("TestCycle2");
        QCOMPARE(nc2.getTemplate(), "NEWTEST2-%y-%w-%i");
    }

    void loadTest()
    {
        QMap<QString, NumberCycle> map = _ncs.load();
        QVERIFY(!map.isEmpty());
    }

    void getTest()
    {
        NumberCycle nc = _ncs.get("TestCycle1");
        QVERIFY(nc.counter() == 121);
        QVERIFY(nc.getTemplate() == QStringLiteral("TEST1-%y-%w-%d-%i"));

        const NumberCycle nc2 = _ncs.get("TestCycle2");
        QVERIFY(nc2.counter() == 12);
        QVERIFY(nc2.getTemplate() == QStringLiteral("NEWTEST2-%y-%w-%i"));
    }

    void clash1()
    {
        NumberCycle nc1 = _ncs.get("TestCycle1");
        QCOMPARE(nc1.counter(), 121);
        // increase the number by one.
        QString i = _ncs.generateIdent("TestCycle1", "Rechnung", QDate(2023, 1, 23), "addressUid");
        QCOMPARE(i, "TEST1-2023-4-23-122");

        // clash the nc file
        QVERIFY(directFileChange(_baseDir + "/numbercycles/numbercycles.xml", 122, 158));

        i = _ncs.generateIdent("TestCycle1", "Rechnung", QDate(2023, 1, 24), "addressUid");
        QCOMPARE(i, "TEST1-2023-4-24-159");

    }

    void checkVariableReplacement() {
        NumberCycle nc;
        nc.setName("Test");
        nc.setCounter(121);
        nc.setTemplate("FOO-%y-%w-%d-%i");
        QString re = nc.exampleIdent("Test", QDate(2020, 01,23), "addressUID");
        QCOMPARE(re, QStringLiteral("FOO-2020-4-23-122"));

        nc.setTemplate("FOO-%y-%ww-%d-%i");
        re = nc.exampleIdent("Test", QDate(2020, 01,23), "addressUID");
        QCOMPARE(re, QStringLiteral("FOO-2020-04-23-122")); // the id can change

        nc.setTemplate("FOO-%y-%ww-%d-%iiiii");
        re = nc.exampleIdent("Test", QDate(2020, 01,23), "addressUID");
        QCOMPARE(re, QStringLiteral("FOO-2020-04-23-00122")); // the id can change

        nc.setTemplate("FOO-%y-%ww-%d-%iiii");
        re = nc.exampleIdent("Test", QDate(2020, 01,23), "addressUID");
        QCOMPARE(re, QStringLiteral("FOO-2020-04-23-0122")); // the id can change

        nc.setTemplate("FOO-%y-%ww-%d-%iii");
        re = nc.exampleIdent("Test", QDate(2020, 01,23), "addressUID");
        QCOMPARE(re, QStringLiteral("FOO-2020-04-23-122")); // the id can change

        nc.setTemplate("FOO-%y-%ww-%d-%ii");
        re = nc.exampleIdent("Test", QDate(2020, 01,23), "addressUID");
        QCOMPARE(re, QStringLiteral("FOO-2020-04-23-122")); // the id can change
    }

private:
    NumberCycles _ncs;
    QString _baseDir;
    QTemporaryDir _dir;

};

QTEST_MAIN(T_NumberCycle)
#include "t_numbercycle.moc"

