#include <QTest>
#include <QObject>

#include "format.h"

class T_Format : public QObject {
    Q_OBJECT
private slots:
    void initTestCase()
    {

    }

    void t1()
    {
        double s = 1.0;
        const QString sstr = Format::localeDoubleToString(s, QLocale::c());
        QCOMPARE(sstr, QStringLiteral("1"));

        double s1 = 1.2;
        const QString sstr1 = Format::localeDoubleToString(s1, QLocale::c());
        QCOMPARE(sstr1, QStringLiteral("1.2"));

        double s2 = 1.43;
        const QString sstr2 = Format::localeDoubleToString(s2, QLocale::c());
        QCOMPARE(sstr2, QStringLiteral("1.43"));

        double s3 = 1.334543;
        const QString sstr3 = Format::localeDoubleToString(s3, QLocale::c());
        QCOMPARE(sstr3, QStringLiteral("1.335"));

        double s4 = 1.50;
        const QString sstr4 = Format::localeDoubleToString(s4, QLocale::c());
        QCOMPARE(sstr4, QStringLiteral("1.5"));

    }

private:

};

QTEST_MAIN(T_Format)
#include "t_format.moc"

