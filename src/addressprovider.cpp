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

AddressProvider::AddressProvider( QObject *parent )
  :QObject( parent )
{
  using namespace Akonadi;
}

void AddressProvider::allAddresses( )
{
  Akonadi::ContactSearchJob *job = new Akonadi::ContactSearchJob( this );
  connect( job, SIGNAL(result(KJob*)), this, SLOT( searchResult( KJob*)));
  mAllAddressesJobs[job] = 1;
  job->start();
}

void AddressProvider::getAddressee( const QString& uid )
{
  Akonadi::ContactSearchJob *job = new Akonadi::ContactSearchJob( this );
  job->setLimit( 100 );
  job->setQuery( Akonadi::ContactSearchJob::ContactUid , uid );

  connect( job, SIGNAL( result( KJob* ) ), this, SLOT( searchResult( KJob* ) ) );

  mUidSearchJobs[job] = uid;
  job->start();
}

void AddressProvider::searchResult( KJob* job )
{
  Akonadi::ContactSearchJob *searchJob = qobject_cast<Akonadi::ContactSearchJob*>( job );

  if( searchJob->error() ) {
    kDebug() << "Address search job failed: " << job->errorString();
    return;
  }

  const KABC::Addressee::List contacts = searchJob->contacts();
  if( contacts.size() > 0 )  {
    if( mAllAddressesJobs.contains( job )) {
      kDebug() << "Found list of " << contacts.size() << " addresses for all addresses";
      mAllAddressesJobs.remove( job );
      emit addressListFound( contacts );
    }
    if( mUidSearchJobs.contains( job )) {
      // mUidSearchJobs.remove(job);
      KABC::Addressee contact = contacts[0];
      const QString uid = mUidSearchJobs.value( job );
      kDebug() << "Found uid search job for UID " << uid << " = " << contact.realName();

      emit addresseeFound( uid, contact );
    }
  } else {
    kDebug() << "Akonadi search result list has size of 0";
    if( mUidSearchJobs.contains(job)) {
      emit addresseeFound( mUidSearchJobs.value(job), KABC::Addressee() );
    }
  }
  mUidSearchJobs.remove( job );
  // FIXME: Remove job entries from mUidSearchJobs and mAllAddressesJobs
  emit( finished( contacts.size() ) );
}
