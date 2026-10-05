#include <QTest>

#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QTemporaryDir>

#include "testconfig.h"
#include "defaultprovider.h"
#include "kraftcontact.h"

using namespace Qt::StringLiterals;

class T_KraftContact : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QDir sourceDir(TESTS_PATH);
        sourceDir.cdUp();
        const QByteArray ba{sourceDir.absolutePath().toLatin1()};
        qputenv("KRAFT_HOME", ba);

        _baseDir = DefaultProvider::self()->createV2BaseDir(_dir.path());
        QVERIFY(!_baseDir.isEmpty());
    }

    // The contacts dir is part of a freshly created Kraft base dir.
    void contactsDirExists()
    {
        const QString d = DefaultProvider::self()->kraftV2Dir(DefaultProvider::KraftV2Dir::Contacts);

        QVERIFY(!d.isEmpty());
        QCOMPARE(QDir(d).dirName(), u"contacts"_s);
        QVERIFY(QFileInfo(d).isDir());
    }

    void valuesAreTrimmedAndRemovable()
    {
        KraftContact c;

        QVERIFY(c.stringAttribute(KraftContact::VatId).isEmpty());

        c.setStringAttribute(KraftContact::VatId, u"  DE123456789  "_s);
        QCOMPARE(c.stringAttribute(KraftContact::VatId), u"DE123456789"_s);
        QVERIFY(c.hasAttribute(KraftContact::VatId));

        // An emptied value is no value at all, not an empty attribute.
        c.setStringAttribute(KraftContact::VatId, u"   "_s);
        QVERIFY(!c.hasAttribute(KraftContact::VatId));
        QVERIFY(c.stringAttribute(KraftContact::VatId).isEmpty());
    }

    /* Writing the same values again is what a settings dialog does on every OK.
     * It must not make the object dirty, otherwise the flag says nothing.
     */
    void writingTheSameValueIsNoModification()
    {
        KraftContact c;

        c.setStringAttribute(KraftContact::Bic, u"BYLADEM1001"_s);
        QVERIFY(c.modified());
        const QDateTime firstWrite = c.lastModified();

        c.setModified(false);
        QVERIFY(!c.modified());
        // Marking it clean is no modification either, so the time has to stand.
        QCOMPARE(c.lastModified(), firstWrite);

        c.setStringAttribute(KraftContact::Bic, u"BYLADEM1001"_s);
        QVERIFY(!c.modified());
        c.setStringAttribute(KraftContact::Bic, u"  BYLADEM1001  "_s); // same, trimmed
        QVERIFY(!c.modified());

        // Removing one that is not there is no modification either.
        c.setStringAttribute(KraftContact::VatId, QString());
        QVERIFY(!c.modified());

        c.setStringAttribute(KraftContact::Bic, u"GENODEF1S02"_s);
        QVERIFY(c.modified());
    }

    /* The contact of a document is loaded straight from its address uid, which is
     * what names the file.
     */
    void saveAndLoad()
    {
        const QString addressUid{u"12345-abcde"_s};
        QString uuid;
        QDateTime lastModified;
        {
            KraftContact c(addressUid);
            c.setStringAttribute(KraftContact::VatId, u"DE123456789"_s);
            c.setStringAttribute(KraftContact::TaxNumber, u"123/456/78901"_s);
            c.setStringAttribute(KraftContact::Iban, u"DE02120300000000202051"_s);
            c.addTag(u"customer"_s);

            QVERIFY(c.modified());
            QVERIFY(c.save());
            QVERIFY(!c.modified());
            uuid = c.uuid();
            lastModified = c.lastModified();
            QVERIFY(!uuid.isEmpty());

            QVERIFY(QFile::exists(c.storageFile()));
            QCOMPARE(QFileInfo(c.storageFile()).fileName(), addressUid + u".xml"_s);
        }

        KraftContact c2(addressUid);
        QVERIFY(c2.load());
        QVERIFY(!c2.modified());
        // The time of the last change belongs to the contact, not to the load.
        // Compared as stored, because the XML keeps seconds and no milliseconds.
        QCOMPARE(c2.lastModified().toString(Qt::ISODate), lastModified.toString(Qt::ISODate));

        QCOMPARE(c2.uuid(), uuid);
        QCOMPARE(c2.backendUid(), addressUid);
        QCOMPARE(c2.stringAttribute(KraftContact::VatId), u"DE123456789"_s);
        QCOMPARE(c2.stringAttribute(KraftContact::TaxNumber), u"123/456/78901"_s);
        QCOMPARE(c2.stringAttribute(KraftContact::Iban), u"DE02120300000000202051"_s);
        QVERIFY(c2.stringAttribute(KraftContact::LegalForm).isEmpty());
        QVERIFY(c2.hasTag(u"customer"_s));
    }

    // A contact Kraft has no data of yet is not an error, it just has no values.
    void loadOfUnknownContact()
    {
        KraftContact c(u"never-stored"_s);
        QVERIFY(!c.load());
        QVERIFY(c.stringAttribute(KraftContact::VatId).isEmpty());
    }

    // An address typed by hand has no uid, so there is nothing to store it under.
    void noStorageKeyIsNotStored()
    {
        KraftContact c;
        QVERIFY(c.storageFile().isEmpty());
        QVERIFY(!c.save());
        QVERIFY(!c.load());
    }

    /* A storage key of its own is what the own identity uses, because it keeps its
     * business data when the address behind it is swapped.
     */
    void storageKeyOverridesTheBackendUid()
    {
        KraftContact c(u"an-address-uid"_s);
        c.setStorageKey(u"myidentity"_s);
        QCOMPARE(QFileInfo(c.storageFile()).fileName(), u"myidentity.xml"_s);

        c.setStringAttribute(KraftContact::VatId, u"DE999999999"_s);
        QVERIFY(c.save());

        KraftContact c2;
        c2.setStorageKey(u"myidentity"_s);
        QVERIFY(c2.load());
        QCOMPARE(c2.stringAttribute(KraftContact::VatId), u"DE999999999"_s);
        QCOMPARE(c2.backendUid(), u"an-address-uid"_s);
    }

    /* Akonadi uids are no valid file names. They have to end up in one file each
     * all the same, which the hash appended to the sanitized name takes care of.
     */
    void uidsThatAreNoFileNames()
    {
        const QString uid1{u"akonadi://?item=42&collection=7"_s};
        const QString uid2{u"akonadi://?item=43&collection=7"_s};

        KraftContact c1(uid1);
        KraftContact c2(uid2);

        const QString f1 = QFileInfo(c1.storageFile()).fileName();
        const QString f2 = QFileInfo(c2.storageFile()).fileName();

        QVERIFY(!f1.contains(u'/'));
        QVERIFY(!f1.contains(u'?'));
        QVERIFY(f1 != f2);

        c1.setStringAttribute(KraftContact::VatId, u"DE111111111"_s);
        c2.setStringAttribute(KraftContact::VatId, u"DE222222222"_s);
        QVERIFY(c1.save());
        QVERIFY(c2.save());

        KraftContact r1(uid1);
        QVERIFY(r1.load());
        QCOMPARE(r1.stringAttribute(KraftContact::VatId), u"DE111111111"_s);
    }

private:
    QString _baseDir;
    QTemporaryDir _dir;
};

QTEST_MAIN(T_KraftContact)
#include "t_kraftcontact.moc"
