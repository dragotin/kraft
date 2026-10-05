/***************************************************************************
                             myidentity.h
                             -------------------
    begin                : Oct. 2024
    copyright            : (C) 2024 by Klaas Freitag
    email                : kraft@freisturz.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "myidentity.h"
#include "kraftsettings.h"
#include "addressprovider.h"
#include "defaultprovider.h"
#include "kraftcontact.h"

#include <KLocalizedString>
#include <QFile>

#include <kcontacts_version.h>
#include <kcontacts/resourcelocatorurl.h>
#include <kcontacts/vcardconverter.h>

KraftContact *MyIdentity::_ownContact = nullptr;

using namespace Qt::StringLiterals;

// The storage key of the own identity. Unlike a customer contact it can not be
// stored under its backend uid, because it may have none at all, and it has to
// stay findable when the user picks another address book entry for it.
static const QString OwnIdentityKey = u"myidentity"_s;

MyIdentity::MyIdentity(QObject *parent)
    : QObject{parent}
{

}

KraftContact *MyIdentity::ownContact()
{
    if (!_ownContact) {
        _ownContact = new KraftContact();
        _ownContact->setStorageKey(OwnIdentityKey);

        if (!_ownContact->load()) {
            // There is no contact file yet, which is the case for a Kraft that was
            // updated rather than set up freshly.
            migrateSettingsToContact(_ownContact);
        }
    }
    return _ownContact;
}

/* Until the own company became a KraftContact, its bank account and its business
 * data were kept in the settings file. Take the values over once, so that an
 * update does not silently empty the bank account on the documents.
 */
void MyIdentity::migrateSettingsToContact(KraftContact *contact)
{
    auto *settings = KraftSettings::self();

    contact->setStringAttribute(KraftContact::AccountName, settings->bankAccountName());
    contact->setStringAttribute(KraftContact::Iban, settings->bankAccountIBAN());
    contact->setStringAttribute(KraftContact::Bic, settings->bankAccountBIC());

    contact->setStringAttribute(KraftContact::VatId, settings->sellerVatId());
    contact->setStringAttribute(KraftContact::TaxNumber, settings->sellerTaxNumber());
    contact->setStringAttribute(KraftContact::RegistrationId, settings->sellerRegistrationId());
    contact->setStringAttribute(KraftContact::LegalForm, settings->sellerLegalForm());

    if (contact->modified()) {
        qDebug() << "Migrated the business data from the settings to the own contact";
        contact->save();
    }
}

KContacts::Addressee MyIdentity::UIToAddressee(Ui::manualOwnIdentity ui)
{
    KContacts::Addressee add;
    add.setFormattedName(ui.leName->text());
    add.setOrganization(ui.leOrganization->text());
    KContacts::Address workAddress;

    workAddress.setStreet(ui.leStreet->text());
    workAddress.setPostalCode(ui.lePostcode->text());
    workAddress.setLocality(ui.leCity->text());
    workAddress.setType(KContacts::Address::Work);
    add.insertAddress(workAddress);

    add.insertPhoneNumber(KContacts::PhoneNumber(ui.lePhone->text(), KContacts::PhoneNumber::Work));
    add.insertPhoneNumber(KContacts::PhoneNumber(ui.leFax->text(), KContacts::PhoneNumber::Fax));
    add.insertPhoneNumber(KContacts::PhoneNumber(ui.leMobile->text(), KContacts::PhoneNumber::Cell));
    KContacts::ResourceLocatorUrl resUrl;
    resUrl.setUrl(QUrl(ui.leWebsite->text()));
    add.setUrl(resUrl);

    KContacts::Email email;
    email.setEmail(ui.leEmail->text());
    email.setPreferred(true);
    email.setType(KContacts::Email::TypeFlag::Work);
    add.addEmail(email);

    return add;
}

/* The business data of the own company, plus the one value that only makes sense
 * for it: the country code, which a customer brings along in its address.
 */
QMap<QString, QString> MyIdentity::ownBusinessData()
{
    QMap<QString, QString> re = ownContact()->businessData();

    // ISO 3166-1 alpha-2 of the locale Kraft runs under. The vCard only knows the
    // country as a localized name, which is of no use for the XRechnung BT-40.
    const QLocale *loc = DefaultProvider::self()->locale();
    re.insert(u"COUNTRYCODE"_s, QLocale::territoryToCode(loc->territory()));

    return re;
}

/* The template variables of the `me` namespace that do not come from the contact.
 * Escaped here, once, so no template has to remember to do it.
 */
QVariantHash MyIdentity::ownBusinessVariantHash()
{
    QVariantHash re;

    const auto data = ownBusinessData();
    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        re.insert(it.key(), it.value().toHtmlEscaped());
    }
    return re;
}

QString MyIdentity::identityFile()
{
    QString file = DefaultProvider::self()->kraftV2Dir(DefaultProvider::KraftV2Dir::OwnIdentity);
    file += "/myidentity.vcd";

    return file;
}

bool MyIdentity::hasBackend()
{
    return ownContact()->hasBackend();
}

void MyIdentity::load()
{
    KraftContact *own = ownContact();

    // The identity is loaded more than once over the lifetime of Kraft, ie. again
    // after the user picked another address in the settings.
    own->setAddressee(KContacts::Addressee());
    connect(own, &KraftContact::addresseeLoaded,
            this, &MyIdentity::slotAddresseeFound, Qt::UniqueConnection);

    // Fetch my address
    const QString myUid = KraftSettings::self()->userUid();
    own->setBackendUid(myUid);

    if (!myUid.isEmpty()) {
        qDebug() << "looking up my identity" << myUid << "in address provider";
        // Emits addresseeLoaded, either right away or once the backend answers.
        own->lookupAddressee();
    } else {
        // Without a uid the identity is an address the user typed into the settings.
        // It is kept as a vCard of its own, which no address book knows about.
        const QString file = identityFile();
        qDebug() << "looking up my identity in vcard file" << file;

        KContacts::Addressee contact;
        own->setSource(KraftContact::Source::Unknown);

        QFile f(file);
        if (f.exists()) {
            if (f.open(QIODevice::ReadOnly)) {
                const QByteArray data = f.readAll();
                KContacts::VCardConverter converter;
                KContacts::Addressee::List list = converter.parseVCards(data);

                if (list.count() > 0) {
                    contact = list.at(0);
                    contact.insertCustom(CUSTOM_ADDRESS_MARKER, "manual");
                }
                own->setSource(KraftContact::Source::Manual);
            }
        } else {
            qDebug() << "VCard file does not exist!";
        }
        slotAddresseeFound(myUid, contact);
    }
}

QString MyIdentity::errorMsg(const QString& uid)
{
    return ownContact()->errorMsg(uid);
}

void MyIdentity::slotAddresseeFound(const QString& uid, const KContacts::Addressee& contact)
{
    ownContact()->setAddressee(contact);
    Q_EMIT myIdentityLoaded(uid, contact);
}

KContacts::Addressee MyIdentity::contact() const
{
    return ownContact()->addressee();
}

MyIdentity::Source MyIdentity::source() const
{
    switch (ownContact()->source()) {
    case KraftContact::Source::Manual:
        return Source::Manual;
    case KraftContact::Source::Backend:
        return Source::Backend;
    case KraftContact::Source::Unknown:
        break;
    }
    return Source::Unknown;
}

void MyIdentity::save(const QString& uuid, const KContacts::Addressee& contact)
{
    const QString file{identityFile()};

    const QString myUid = KraftSettings::self()->userUid();
    if (!uuid.isEmpty() && myUid == uuid) {
        // nothing has changed
        return;
    }

    if (uuid.isEmpty()) { // save the manual address
        KContacts::VCardConverter vcc;
        const QByteArray vcard = vcc.createVCard(contact);

        QFile f ( file );
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            f.write(vcard);
            f.close();
            qDebug() << "Saved own identity to " << file;
        }
        ownContact()->setSource(KraftContact::Source::Manual);
    } else {
        QFile::remove(file); // remove a maybe existing file
        ownContact()->setSource(KraftContact::Source::Backend);
    }

    // The business data stays with the identity no matter which address it uses,
    // only the reference to the address book entry changes.
    ownContact()->setBackendUid(uuid);
    ownContact()->save();

    // Q_EMIT the signal for consumers of the address
    slotAddresseeFound(uuid, contact);

    // update the settings - clear the user name as it is deprecated anyway
    KraftSettings::self()->setUserName(QString());
    KraftSettings::self()->setUserUid(uuid);
    KraftSettings::self()->save();
}
