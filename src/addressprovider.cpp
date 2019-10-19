/***************************************************************************
                          addressprovider.cpp  -
                             -------------------
    begin                : Fri Mar 4 2011
    copyright            : (C) 2011 by Klaas Freitag
    email                : freitag@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "addressprovider.h"
#include <QDebug>

// FIXME this needs to change once there are more address book providers, ie.
// on Mac.

#include "addressprovider_akonadi.h"

/* ==================================================================================== */

AddressProvider::AddressProvider( QObject *parent )
  :QObject( parent ),
    _d( new AddressProviderPrivate(parent) )
{
    connect(_d, SIGNAL(addresseeFound(QString, KContacts::Addressee)),
            this, SLOT(slotAddresseeFound(QString, KContacts::Addressee)));
    connect(_d, SIGNAL(lookupError( QString, QString)), this,
            SLOT(slotErrorMsg(QString, QString)));
    connect(_d, SIGNAL(addresseeNotFound(QString)), SLOT(slotAddresseeNotFound(QString)));
}

bool AddressProvider::backendUp()
{
    return _d->backendUp();
}

QString AddressProvider::backendName() const
{
    return _d->backendName();
}

void AddressProvider::slotAddresseeFound( const QString& uid, const KContacts::Addressee contact)
{
    // remove a potential error message in case an error happened before
    if( !( uid.isEmpty() || contact.isEmpty()) ) {
        _errMessages.remove(uid);
    }
    _addressCache[uid] = contact;
    _addressCache[uid].insertCustom(CUSTOM_ADDRESS_MARKER, "addressbook");
    emit lookupResult(uid, _addressCache[uid]);
}

void AddressProvider::slotAddresseeNotFound( const QString& uid )
{
    KContacts::Addressee contact; // Empty for not found.
    _notFoundUids.insert(uid);
    emit lookupResult(uid, contact);
}

void AddressProvider::slotResetNotFoundCache()
{
    _notFoundUids.clear();
}

void AddressProvider::slotErrorMsg(const QString& uid, const QString& msg)
{
    if( !uid.isEmpty() ) {
        _errMessages[uid] = msg;
    }
}

QString AddressProvider::errorMsg( const QString& uid )
{
    if( !_d->backendUp() ) {
        return "Backend down";
    }
    if( _errMessages.contains(uid) ) {
        return _errMessages[uid];
    }
    return QString();
}

KContacts::Addressee AddressProvider::getAddresseeFromCache(const QString& uid)
{
    KContacts::Addressee adr;
    if( _addressCache.contains(uid)) {
        adr = _addressCache[uid];
    }
    return adr;
}

AddressProvider::LookupState AddressProvider::lookupAddressee( const QString& uid )
{
    // FIXME: Check for the size of the err messages. If it is big,
    // maybe do not bother the backend more
    if( !_d->backendUp() ) {
        return BackendError;
    }
    if( _notFoundUids.contains(uid)) {
        // qDebug() << uid << "was not found before";
        return LookupNotFound;
    }
    if( _addressCache.contains(uid)) {
        return LookupFromCache;
    }
    if( _d->isSearchOngoing(uid) ) {
        return  LookupOngoing;
    }
    if( _d->lookupAddressee(uid) ) {
        return LookupStarted;
    }

    return ItemError;

}

KContacts::Addressee AddressProvider::getAddressee(const QModelIndex& indx)
{
    return _d->getAddressee(indx);
}

KContacts::Addressee AddressProvider::getAddressee( int row, const QModelIndex &parent)
{
    return _d->getAddressee(row, parent);
}

QAbstractItemModel *AddressProvider::model()
{
    return _d->model();
}

QString AddressProvider::formattedAddress( const KContacts::Addressee& contact ) const
{
  QString re;
  KContacts::Address address;

  address = contact.address( KContacts::Address::Pref );
  if( address.isEmpty() )
    address = contact.address(KContacts::Address::Work );
  if( address.isEmpty() )
    address = contact.address(KContacts::Address::Home );
  if( address.isEmpty() )
    address = contact.address(KContacts::Address::Postal );

  if( address.isEmpty() ) {
    re = contact.realName();
  } else {
    re = address.formattedAddress( contact.realName(), contact.organization() );
  }
  return re;
}

