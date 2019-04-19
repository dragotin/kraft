#include <QTest>
#include <QBuffer>

#include "metaxmlparser.h"

class T_MetaParser : public QObject {
    Q_OBJECT
private slots:
    void initTestCase()
    {

    }

    void goodParser() {
        QByteArray xml = "<kraftmeta>\
                <migrate>\
                <doctype>\
                <name>Progress Payment Invoice</name>\
                <numbercycle>default</numbercycle>\
                <lang>en</lang>\
                <attrib>\
                  <key>PartialInvoice</key>\
                  <value>true</value>\
                </attrib>\
                <attrib>\
                  <key>RedRider</key>\
                  <value>true</value>\
                </attrib>\
                <follower>Final Invoice</follower>\
                <follower>Invoice</follower>\
                </doctype>\
                </migrate>\
                </kraftmeta>";

        QBuffer buf( &xml);

        MetaXMLParser parser;
        QVERIFY(parser.parse(&buf));

        QList<MetaDocTypeAdd> list = parser.metaDocTypeAddList();
        QVERIFY(list.size() == 1 );

        MetaDocTypeAdd tadd = list.first();
        QCOMPARE(tadd.name(), QLatin1String("Progress Payment Invoice") );
        QCOMPARE(tadd.numbercycle(), QLatin1String("default"));
        QCOMPARE(tadd.lang(), QLatin1String("en"));
        QVERIFY(tadd._attribs.size() == 2);
        QVariant rr(tadd._attribs.value("RedRider"));
        QVERIFY(rr.toBool());
        QVERIFY(tadd._follower.size() == 2);
        QCOMPARE(tadd._follower.at(0), QLatin1String("Final Invoice"));
        QCOMPARE(tadd._follower.at(1), QLatin1String("Invoice"));
    }
private:

};

QTEST_MAIN(T_MetaParser)
#include "t_metaparser.moc"

