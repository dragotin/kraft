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

    // fetching all collections recursive, starting at the root collection
    CollectionFetchJob *job = new CollectionFetchJob( Collection::root(), CollectionFetchJob::Recursive );
    connect( job, SIGNAL(result(KJob*)), SLOT(fetchFinished(KJob*)) );

}

void AddressProviderPrivate::fetchFinished( KJob *job )
{
  if ( job->error() ) {
    qDebug() << "Error occurred";
    return;
  }
  CollectionFetchJob *fetchJob = qobject_cast<CollectionFetchJob*>( job );
  const Collection::List collections = fetchJob->collections();
  QStringList mimes = QStringList() << KContacts::Addressee::mimeType() << KContacts::ContactGroup::mimeType();

  foreach ( const Collection &collection, collections ) {
      // if( collection.contentMimeTypes().contains(mimes) ) {
      //    qDebug() << "Name:" << collection.name() << collection.contentMimeTypes();
      // }
  }
}

void AddressProviderPrivate::lookupAddressee( const QString& uid )
{
    if( uid.isEmpty() || mUidSearches.contains( uid ) ) {
        // search is already running
        // qDebug () << "Search already underways!";^
        return;
    }

    KJob *job = NULL;

    Akonadi::ContactSearchJob *csjob = new Akonadi::ContactSearchJob( this );
    csjob->setLimit( 1 );
    csjob->setQuery( Akonadi::ContactSearchJob::ContactUid , uid );
    mUidSearchJobs[csjob] = uid;
    job = csjob;
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( searchResult( KJob* ) ) );
    job->start();

    mUidSearches.insert( uid );
    mUidSearchJobs[job] = uid;

    model();

}

void AddressProviderPrivate::searchResult( KJob* job )
{
    if( !job ) return;

    QString uid;
    KContacts::Addressee contact;

    int cnt = 0;

    uid = mUidSearchJobs.value( job );

    if( job->error() ) {
        // qDebug () << "Address Search job failed: " << job->errorString();
    } else {
	Akonadi::ContactSearchJob *searchJob = qobject_cast<Akonadi::ContactSearchJob*>( job );
        const KContacts::Addressee::List contacts = searchJob->contacts();
        // qDebug () << "Found list of " << contacts.size() << " addresses as search result";

        if( mUidSearchJobs.contains( job )) {            
            if( contacts.size() > 0 ) {
                contact = contacts[0];
                qDebug () << "Found uid search job for UID " << uid << " = " << contact.realName();
            }
            emit addresseeFound( uid, contact );
            cnt++;
        }
    }
    // if no address was found, emit the empty contact.
    if( cnt == 0 ) {
        emit addresseeFound(uid, contact);
    }
    emit finished(cnt);

    // cleanup
    if(!uid.isEmpty()) {
        mUidSearches.remove( uid );
    }

    mUidSearchJobs.remove( job );

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

