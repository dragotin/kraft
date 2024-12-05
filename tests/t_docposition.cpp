#include <QTest>
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDateTime>
#include <QVariant>

#include "testconfig.h"
#include "kraftdoc.h"
#include "attribute.h"
#include "docposition.h"
#include "format.h"

void init_test_db()
{

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

class T_DocPosition: public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() {

    }

    void cleanupTestCase() {

    }

    void basicTest() {
        _dp.setAmount(4.0);
        _dp.setUnitPrice(Geld(1.50));
        _dp.setText("Position3");
        _dp.setTags(QStringList{"Material"});

        KraftAttrib attrib("attrib1", QVariant("value"), KraftAttrib::Type::String);
        _dp.setAttribute(attrib);

        QVERIFY(_dp.amount() == 4.0);
        QVERIFY(_dp.overallPrice().toDouble() == 6.00);
    }

    void kraftObj1() {
        KraftObj obj;

        const QString uuid = obj.createUuid();
        QCOMPARE(uuid, obj.uuid());

        auto dt = QDateTime::currentDateTime();
        obj.setLastModified(dt);
        QCOMPARE(dt, obj.lastModified());
    }

    void kraftObjTags() {
        KraftObj obj;
        const QStringList l {"Material", "Work", "Extra"};
        obj.setTags(l);
        const QStringList ol = obj.allTags();
        QCOMPARE(ol.size(), 3);
        QVERIFY(ol.contains("Material"));
        QVERIFY(ol.contains("Work"));
        QVERIFY(ol.contains("Extra"));
    }

    void copyObj() {
        KraftObj obj;
        const QStringList l {"Material", "Work", "Extra"};
        obj.setTags(l);
        const QString uuid = obj.createUuid();

        KraftObj cobj = obj;
        QCOMPARE(uuid, cobj.uuid());

        const QStringList ol = cobj.allTags();
        QCOMPARE(ol.size(), 3);
        QVERIFY(ol.contains("Material"));
        QVERIFY(ol.contains("Work"));
        QVERIFY(ol.contains("Extra"));
    }

    void copyDocPosition() {
        DocPosition newDp;

        newDp = _dp;
        QVERIFY(newDp.amount() == 4.0);
        QVERIFY(newDp.overallPrice().toDouble() == 6.00);
        QVERIFY(newDp.text() == QStringLiteral("Position3"));
        QStringList tags = newDp.allTags();
        QCOMPARE(tags.size(), 1);
        QVERIFY(tags.at(0)== QStringLiteral("Material"));

        QMap<QString, KraftAttrib> attribs = newDp.attributes();

        QCOMPARE(attribs.size(), 1);
        QCOMPARE(attribs["attrib1"].value(), QVariant("value"));
    }
private:
    DocPosition _dp;

};

QTEST_MAIN(T_DocPosition)
#include "t_docposition.moc"

