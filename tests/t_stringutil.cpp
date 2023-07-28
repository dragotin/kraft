#include <QTest>

#include "testconfig.h"
#include "stringutil.h"

class T_Stringutil : public QObject {
    Q_OBJECT
private slots:
    void simple1() {
        QString tmpl{"Foo %nn baz"};
        QMap<QString, QString> rep;
        rep.insert("%nn", "bar");
        QString re = StringUtil::replaceTagsInString(tmpl, rep);

        QCOMPARE(re, QStringLiteral("Foo bar baz"));
    }

    void keyLenVariant() {
        QString tmpl{"Foo %nn %nnn baz"};
        QMap<QString, QString> rep;
        rep.insert("%nn", "bar");
        rep.insert("%nnn", "bar2");
        QString re = StringUtil::replaceTagsInString(tmpl, rep);

        QCOMPARE(re, QStringLiteral("Foo bar bar2 baz"));
    }

    void multipleKey() {
        QString tmpl{"Foo %nn %nnn %nn baz"};
        QMap<QString, QString> rep;
        rep.insert("%nn", "bar");
        rep.insert("%nnn", "bar2");
        QString re = StringUtil::replaceTagsInString(tmpl, rep);

        QCOMPARE(re, QStringLiteral("Foo bar bar2 bar baz"));
    }

    void multipleKey2() {
        QString tmpl{"Foo %nn %nnn %nn %dd baz"};
        QMap<QString, QString> rep;
        rep.insert("%nn", "bar");
        rep.insert("%nnn", "bar2");
        rep.insert("%dd", "numbers");
        QString re = StringUtil::replaceTagsInString(tmpl, rep);

        QCOMPARE(re, QStringLiteral("Foo bar bar2 bar numbers baz"));
    }

    void percentVal() {
        QString tmpl{"Foo %nn %nnn"};
        QMap<QString, QString> rep;
        rep.insert("%nn", "bar %");
        rep.insert("%nnn", "bar2");
        QString re = StringUtil::replaceTagsInString(tmpl, rep);

        QCOMPARE(re, QStringLiteral("Foo bar % bar2"));
    }

};

QTEST_MAIN(T_Stringutil)
#include "t_stringutil.moc"

