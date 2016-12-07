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
#include <AkonadiCore/ItemFetchJob>
#include <AkonadiCore/ItemFetchScope>
#include <QDebug>

AddressProviderPrivate::AddressProviderPrivate( QObject *parent )
  :QObject( parent )
{
  using namespace Akonadi;
}

void AddressProviderPrivate::getAddressee( const QString& uid )
{
    if( uid.isEmpty() || mUidSearches.contains( uid ) ) {
        // search is already running
        // qDebug () << "Search already underways!";
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

