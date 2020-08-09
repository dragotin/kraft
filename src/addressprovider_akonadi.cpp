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
#include <kcontacts/contactgroup.h>

#include <QDebug>

#ifdef HAVE_AKONADI
#include <AkonadiCore/ItemFetchJob>
#include <AkonadiCore/ItemFetchScope>

#include "akonadi/contact/contactsearchjob.h"
#include <AkonadiCore/CollectionFetchJob>

#include <AkonadiCore/ItemFetchJob>
#include <AkonadiCore/ItemFetchScope>
#include <AkonadiCore/entitydisplayattribute.h>
#include <AkonadiCore/control.h>

using namespace Akonadi;
#endif

AddressProviderPrivate::AddressProviderPrivate( QObject *parent )
  :QObject( parent ),
    _akonadiUp(false),
    mSession(0),
    mMonitor(0),
    _model(0)
{
    init();
}

bool AddressProviderPrivate::init()
{
    _akonadiUp = false;
#ifdef HAVE_AKONADI
    if ( !Akonadi::Control::start( ) ) {
        qDebug() << "Failed to start Akonadi!";
    } else {
        mSession = new Akonadi::Session( "KraftSession" );
        _akonadiUp = true;
    }
#endif
    return _akonadiUp;
}

QString AddressProviderPrivate::backendName() const
{
#ifdef HAVE_AKONADI
    return QStringLiteral("Akonadi");
#else
    return QStringLiteral("Akonadi disabled by compile option!");
#endif
}


bool AddressProviderPrivate::backendUp()
{
    return _akonadiUp;
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
#ifdef HAVE_AKONADI
    Akonadi::ContactSearchJob *csjob = new Akonadi::ContactSearchJob( this );
    csjob->setLimit( 1 );
    csjob->setQuery(ContactSearchJob::ContactUid , uid, ContactSearchJob::ExactMatch);

    csjob->setProperty("UID", uid);
    connect( csjob, SIGNAL( result( KJob* ) ), this, SLOT( searchResult( KJob* ) ) );
    csjob->start();

    mUidSearches.insert( uid );

    model();
    return true;
#endif
    return false;
}

#ifdef HAVE_AKONADI
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
        const KContacts::Addressee::List contacts =
                searchJob->contacts();
                KContacts::Addressee::List();
        // qDebug () << "Found list of " << contacts.size() << " addresses as search result";

        if( contacts.size() > 0 ) {
            contact = contacts[0];
            // qDebug() << "Found uid search job for UID " << uid << " = " << contact.realName();
            emit addresseeFound(uid, contact);
        } else {
            // qDebug() << "No search result for UID" << uid;
            emit addresseeNotFound(uid);
        }
    }

    // cleanup
    if(!uid.isEmpty()) {
        mUidSearches.remove( uid );
    }

    job->deleteLater();
}
#endif

QAbstractItemModel* AddressProviderPrivate::model()
{
    if( !_akonadiUp ) {
        return 0;
    }
#ifdef HAVE_AKONADI
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
#endif
    return 0;
}

KContacts::Addressee AddressProviderPrivate::getAddressee(const QModelIndex& indx)
{
    KContacts::Addressee contact;
#ifdef HAVE_AKONADI
    if( indx.isValid() ) {
        const Akonadi::Item item = indx.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();

        if (item.hasPayload<KContacts::Addressee>()) {
            contact = item.payload<KContacts::Addressee>();
        }
    }
#endif
    return contact;

}

KContacts::Addressee AddressProviderPrivate::getAddressee(int row, const QModelIndex &parent)
{
#ifdef HAVE_AKONADI
    const QModelIndex index = _model->index(row, 0, parent);

    return getAddressee(index);
#endif
    return KContacts::Addressee();
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

