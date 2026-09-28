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

#include <KLocalizedString>
#include <QFile>
#include <QRegularExpression>

#include <kcontacts_version.h>
#include <kcontacts/resourcelocatorurl.h>
#include <kcontacts/vcardconverter.h>

KContacts::Addressee MyIdentity::_myContact = KContacts::Addressee();

using namespace Qt::StringLiterals;

MyIdentity::MyIdentity(QObject *parent)
    : QObject{parent},
      _addressProvider{nullptr},
      _source{Source::Unknown}
{

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

/* Business data of the own company that is not part of a contact and therefore
 * has no place in the own identity vCard: tax registration, commercial register
 * and the bank account, so it is kept in the settings instead.
 */
QMap<QString, QString> MyIdentity::ownBusinessData()
{
    QMap<QString, QString> re;
    auto *settings = KraftSettings::self();

    // BR-CO-9 wants BT-31 prefixed with the ISO 3166-1 alpha-2 country code and no
    // spaces, so "DE 123 456 789" as a user may type it has to be squeezed first.
    QString vatId = settings->sellerVatId();
    vatId.remove(QRegularExpression(u"\\s"_s));
    re.insert(u"VATID"_s, vatId.toUpper());
    re.insert(u"TAXNUMBER"_s, settings->sellerTaxNumber().trimmed());
    re.insert(u"REGISTRATIONID"_s, settings->sellerRegistrationId().trimmed());
    re.insert(u"LEGALFORM"_s, settings->sellerLegalForm().trimmed());

    re.insert(u"ACCOUNTNAME"_s, settings->bankAccountName().trimmed());
    re.insert(u"IBAN"_s, settings->bankAccountIBAN().trimmed());
    re.insert(u"BIC"_s, settings->bankAccountBIC().trimmed());

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
    return (_addressProvider && _addressProvider->backendUp());
}

void MyIdentity::load()
{
    // Fetch my address
    const QString myUid = KraftSettings::self()->userUid();
    _addressProvider = new AddressProvider(this);
    connect(_addressProvider, &AddressProvider::lookupResult,
             this, &MyIdentity::slotAddresseeFound);

    _myContact = KContacts::Addressee();

    KContacts::Addressee contact;
    if( ! myUid.isEmpty() ) {
        qDebug() << "looking up my identity" << myUid << "in address provider";
        _source = Source::Backend;
        // qDebug () << "Got My UID: " << myUid;
        AddressProvider::LookupState state = _addressProvider->lookupAddressee( myUid );
        switch( state ) {
        case AddressProvider::LookupFromCache:
            contact = _addressProvider->getAddresseeFromCache(myUid);
            break;
        case AddressProvider::LookupNotFound:
        case AddressProvider::ItemError:
        case AddressProvider::BackendError:
            // Try to read from stored vcard.
            break;
        case AddressProvider::LookupOngoing:
        case AddressProvider::LookupStarted:
            // Not much to do, just wait for the signal to come in
            break;
        }
    } else {
        // check if the vcard can be read

        const QString file = identityFile();
        qDebug() << "looking up my identity in vcard file"<< file;
        QFile f(file);
        if( f.exists() ) {
            if( f.open( QIODevice::ReadOnly )) {
                const QByteArray data = f.readAll();
                KContacts::VCardConverter converter;
                KContacts::Addressee::List list = converter.parseVCards( data );

                if( list.count() > 0 ) {
                    contact = list.at(0);
                    contact.insertCustom(CUSTOM_ADDRESS_MARKER, "manual");
                }
                _source = Source::Manual;
            }
        } else {
            qDebug() << "VCard file does not exist!";
            _source = Source::Unknown;
        }
        slotAddresseeFound(myUid, contact);
    }
}

QString MyIdentity::errorMsg(const QString& uid)
{
    return _addressProvider->errorMsg(uid);
}

void MyIdentity::slotAddresseeFound(const QString& uid, const KContacts::Addressee& contact)
{
    _myContact = contact;
    Q_EMIT myIdentityLoaded(uid, contact);
}

KContacts::Addressee MyIdentity::contact() const
{
    return _myContact;
}

MyIdentity::Source MyIdentity::source() const
{
    return _source;
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
    } else {
        QFile::remove(file); // remove a maybe existing file
    }

    // Q_EMIT the signal for consumers of the address
    slotAddresseeFound(uuid, contact);

    // update the settings - clear the user name as it is deprecated anyway
    KraftSettings::self()->setUserName(QString());
    KraftSettings::self()->setUserUid(uuid);
    KraftSettings::self()->save();
}
