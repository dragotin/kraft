#include <QTest>

#include <QDate>
#include <QDomDocument>
#include <QSet>

#include "testconfig.h"
#include "kraftattrib.h"

class T_KraftAttrib: public QObject {
    Q_OBJECT
private slots:

    // copies the example xml document to a temporary path that simulates the
    // storage tree of kraft.
    void initTestCase()
    {
    }

    void simple1()
    {
        KraftAttrib attr("TestAttrib", QVariant("tarock"), KraftAttrib::Type::String);
        KraftAttrib attr2("AttribDate", QVariant(QDate(2023, 02, 24)), KraftAttrib::Type::Date);

        QCOMPARE(attr.name(), "TestAttrib");
        QCOMPARE(attr.value(), "tarock");
        QCOMPARE(attr.type(), KraftAttrib::Type::String);

        QCOMPARE(attr2.name(), "AttribDate");
        QCOMPARE(attr2.value().toDate(), QDate(2023, 2, 24));
        QCOMPARE(attr2.type(), KraftAttrib::Type::Date);
    }

    void stringToTypeTest()
    {
        QCOMPARE(KraftAttrib::stringToType("date"), KraftAttrib::Type::Date);
        QCOMPARE(KraftAttrib::stringToType("string"), KraftAttrib::Type::String);
        QCOMPARE(KraftAttrib::stringToType("integer"), KraftAttrib::Type::Integer);
        QCOMPARE(KraftAttrib::stringToType("color"), KraftAttrib::Type::Color);
    }

    void qsetTest()
    {
        QSet<KraftAttrib> aSet;

        aSet.insert(KraftAttrib("TestAttrib1", QStringLiteral("foobar1"), KraftAttrib::Type::String));
        aSet.insert(KraftAttrib("TestAttrib2", QStringLiteral("foobar2"), KraftAttrib::Type::String));
        aSet.insert(KraftAttrib("TestAttrib3", QStringLiteral("foobar3"), KraftAttrib::Type::String));

        QCOMPARE(aSet.size(), 3);
    }

    void domTest()
    {
        const QString input("<docAttrib>"
                      "<name>counter</name>"
                      "<value>21</value>"
                      "<type>integer</type>"
                      "</docAttrib>");
        QDomDocument xmlDoc;
        xmlDoc.setContent(input);
        QDomElement elem = xmlDoc.firstChild().toElement();

        KraftAttrib attr(elem);

        QCOMPARE(attr.name(), QStringLiteral("counter"));
        QCOMPARE(attr.value(), QVariant(21));
    }

private:

};

QTEST_MAIN(T_KraftAttrib)

#include "t_kraftattrib.moc"
