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

#include <kglobal.h>

#include "addressprovider.h"
#include "akonadi/contact/contactsearchjob.h"
#include "akonadi/session.h"

#include <Akonadi/Item>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>

AddressProvider::AddressProvider( QObject *parent )
  :QObject( parent )
{
  using namespace Akonadi;
}

KJob* AddressProvider::searchAddressGID( const QString& gid )
{
    if( gid.isEmpty() ) return 0;

    Akonadi::Item item;
#if KDE_IS_VERSION(4,12,0)
    item.setGid( gid );
#endif

    Akonadi::ItemFetchJob *fetchJob = new Akonadi::ItemFetchJob(item, this);
    connect( fetchJob, SIGNAL( result( KJob* ) ), this, SLOT( searchResult( KJob* ) ) );
    fetchJob->fetchScope().fetchFullPayload();
    fetchJob->start();
    return fetchJob;
}

void AddressProvider::getAddressee( const QString& uid )
{
    if( uid.isEmpty() || mUidSearches.contains( uid ) ) {
        // search is already running
        kDebug() << "Search already underways!";
        return;
    }

    KJob *job = NULL;

#if KDE_IS_VERSION(4,12,0)
    job = searchAddressGID( uid );
#else
    Akonadi::ContactSearchJob *csjob = new Akonadi::ContactSearchJob( this );
    csjob->setLimit( 1 );
    csjob->setQuery( Akonadi::ContactSearchJob::ContactUid , uid );
    mUidSearchJobs[csjob] = uid;
    job = csjob;
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( searchResult( KJob* ) ) );
    job->start();
#endif
    mUidSearches.insert( uid );
    mUidSearchJobs[job] = uid;

}

void AddressProvider::searchResult( KJob* job )
{
    if( !job ) return;

    QString uid;
    KABC::Addressee contact;

    int cnt = 0;

    uid = mUidSearchJobs.value( job );

    if( job->error() ) {
        kDebug() << "Address Search job failed: " << job->errorString();
    } else {
#if KDE_IS_VERSION(4,12,0)
        Akonadi::ItemFetchJob *fetchJob = qobject_cast<Akonadi::ItemFetchJob*>(job);

        const Akonadi::Item::List items = fetchJob->items();

        foreach( Akonadi::Item item, items ) {
           if( item.hasPayload<KABC::Addressee>() ) {
                contact = item.payload<KABC::Addressee>();
                uid = contact.uid();
                cnt++;
                kDebug() << "Found uid search job for UID " << uid << " = " << contact.realName();
                emit addresseeFound( uid, contact );
           }
        }
#else
	Akonadi::ContactSearchJob *searchJob = qobject_cast<Akonadi::ContactSearchJob*>( job );
        const KABC::Addressee::List contacts = searchJob->contacts();
        kDebug() << "Found list of " << contacts.size() << " addresses as search result";

        if( mUidSearchJobs.contains( job )) {            
            if( contacts.size() > 0 ) {
                contact = contacts[0];
                kDebug() << "Found uid search job for UID " << uid << " = " << contact.realName();
            }
            emit addresseeFound( uid, contact );
            cnt++;
        }
#endif
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

QString AddressProvider::formattedAddress( const KABC::Addressee& contact ) const
{
  QString re;
  KABC::Address address;

  address = contact.address( KABC::Address::Pref );
  if( address.isEmpty() )
    address = contact.address(KABC::Address::Work );
  if( address.isEmpty() )
    address = contact.address(KABC::Address::Home );
  if( address.isEmpty() )
    address = contact.address(KABC::Address::Postal );

  if( address.isEmpty() ) {
    re = contact.realName();
  } else {
    re = address.formattedAddress( contact.realName(), contact.organization() );
  }
  return re;
}

