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

#include "addressprovider_akonadi.h"
#include "akonadi/contact/contactsearchjob.h"

#include <AkonadiCore/ItemFetchJob>
#include <AkonadiCore/ItemFetchScope>

// #include "akonadi/session.h"

// #include <Akonadi/Item>
#include <AkonadiCore/CollectionFetchJob>

#include <AkonadiCore/ItemFetchJob>
#include <AkonadiCore/ItemFetchScope>
#include <kcontacts/contactgroup.h>
#include <AkonadiCore/entitydisplayattribute.h>
#include <AkonadiCore/control.h>

#include <QDebug>

using namespace Akonadi;

AddressProviderPrivate::AddressProviderPrivate( QObject *parent )
  :QObject( parent ),
    mMonitor(0),
    _model(0)
{
    if ( !Akonadi::Control::start( ) ) {
        qDebug() << "Failed to start Akonadi!";
    }
    mSession = new Akonadi::Session( "KraftSession" );
}

bool AddressProviderPrivate::isSearchOngoing(const QString& uid)
{
    return mUidSearches.contains(uid);
}

bool AddressProviderPrivate::lookupAddressee( const QString& uid )
{
    if( uid.isEmpty() ) {
        qDebug() << "Invalid: UID to lookup is empty.";
        return false;
    }

    if( mUidSearches.contains( uid ) ) {
        // search is already running
        // qDebug () << "Search already underways!";^
        return false;
    }

    Akonadi::ContactSearchJob *csjob = new Akonadi::ContactSearchJob( this );
    csjob->setLimit( 1 );
    csjob->setQuery(ContactSearchJob::ContactUid , uid, ContactSearchJob::ExactMatch);

    csjob->setProperty("UID", uid);
    connect( csjob, SIGNAL( result( KJob* ) ), this, SLOT( searchResult( KJob* ) ) );
    csjob->start();

    mUidSearches.insert( uid );

    model();
    return true;
}

void AddressProviderPrivate::searchResult( KJob* job )
{
    if( !job ) return;

    QString uid;
    KContacts::Addressee contact;

    uid = job->property("UID").toString();

    if( job->error() ) {
        // both uid and err message can be empty
        const QString errMsg = job->errorString();
        emit lookupError(uid, errMsg );
        // qDebug () << "Address Search job failed: " << job->errorString();
    } else {
	Akonadi::ContactSearchJob *searchJob = qobject_cast<Akonadi::ContactSearchJob*>( job );
        const KContacts::Addressee::List contacts = searchJob->contacts();
        // qDebug () << "Found list of " << contacts.size() << " addresses as search result";

        if( contacts.size() > 0 ) {
            contact = contacts[0];
            qDebug () << "Found uid search job for UID " << uid << " = " << contact.realName();
        }
    }

    // cleanup
    if(!uid.isEmpty()) {
        // if no address was found, emit the empty contact.
        emit addresseeFound(uid, contact);
        mUidSearches.remove( uid );
    }

    job->deleteLater();
}

QAbstractItemModel *AddressProviderPrivate::model()
{
    Akonadi::ItemFetchScope scope;
    // fetch all content of the contacts, including images
    scope.fetchFullPayload( true );
    // fetch the EntityDisplayAttribute, which contains custom names and icons
    scope.fetchAttribute<Akonadi::EntityDisplayAttribute>();
    if( ! mMonitor ) {
        mMonitor = new Akonadi::ChangeRecorder;
        mMonitor->setSession( mSession );
        // include fetching the collection tree
        mMonitor->fetchCollection( true );
        // set the fetch scope that shall be used
        mMonitor->setItemFetchScope( scope );
        // monitor all collections below the root collection for changes
        mMonitor->setCollectionMonitored( Akonadi::Collection::root() );
        // list only contacts and contact groups
        mMonitor->setMimeTypeMonitored( KContacts::Addressee::mimeType(), true );
        mMonitor->setMimeTypeMonitored( KContacts::ContactGroup::mimeType(), true );
    }
    if( !_model ) {
        _model = new Akonadi::ContactsTreeModel( mMonitor );
        Akonadi::ContactsTreeModel::Columns columns;
        columns << Akonadi::ContactsTreeModel::FullName;
        columns << Akonadi::ContactsTreeModel::HomeAddress;
        // columns << Akonadi::ContactsTreeModel::FamilyName;
        // columns << Akonadi::ContactsTreeModel::GivenName;

        _model->setColumns( columns );
    }
    return _model;
}

KContacts::Addressee AddressProviderPrivate::getAddressee(const QModelIndex& indx)
{
    KContacts::Addressee contact;

    if( indx.isValid() ) {
        const Akonadi::Item item = indx.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();

        if (item.hasPayload<KContacts::Addressee>()) {
            contact = item.payload<KContacts::Addressee>();
        }
    }
    return contact;

}

KContacts::Addressee AddressProviderPrivate::getAddressee(int row, const QModelIndex &parent)
{
    const QModelIndex index = _model->index(row, 0, parent);

    return getAddressee(index);
}

QString AddressProviderPrivate::formattedAddress( const KContacts::Addressee& contact ) const
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

