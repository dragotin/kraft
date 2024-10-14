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

#include <kcontacts_version.h>
#include <kcontacts/resourcelocatorurl.h>
#include <kcontacts/vcardconverter.h>

KContacts::Addressee MyIdentity::_myContact = KContacts::Addressee();

MyIdentity::MyIdentity(QObject *parent)
    : QObject{parent},
      _addressProvider{nullptr}
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

#if KContacts_VERSION >= QT_VERSION_CHECK(5, 88, 0)
    KContacts::Email email;
    email.setEmail(ui.leEmail->text());
    email.setPreferred(true);
    email.setType(KContacts::Email::TypeFlag::Work);
    add.addEmail(email);
#else
    add.insertEmail(ui.leEmail->text(), true /* prefered */ );
#endif
    return add;
}

QString MyIdentity::identityFile()
{
    QString file = DefaultProvider::self()->kraftV2Dir(DefaultProvider::KraftV2Dir::OwnIdentity);
    file += "/myidentity.vcd";

    return file;
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
        _source = Source::Manual;
        const QString file = identityFile();
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
            }
        }
        slotAddresseeFound(myUid, contact);
    }
}

void MyIdentity::slotAddresseeFound(const QString& uid, const KContacts::Addressee& contact)
{
    _myContact = contact;
    emit myIdentityLoaded(uid, contact);
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

    // emit the signal for consumers of the address
    slotAddresseeFound(uuid, contact);

    // update the settings - clear the user name as it is deprecated anyway
    KraftSettings::self()->setUserName(QString());
    KraftSettings::self()->setUserUid(uuid);
    KraftSettings::self()->save();
}
