/***************************************************************************
                                docdigest.cpp
                             -------------------
    begin                : Wed Mar 15 2006
    copyright            : (C) 2006 by Klaas Freitag
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

#include <qstring.h>
#include <kglobal.h>
#include <klocale.h>

#include <kabc/addressbook.h>
#include <kabc/stdaddressbook.h>
#include <kabc/addresseedialog.h>
#include <kabc/addressee.h>

#include "docdigest.h"

DocDigest::DocDigest( dbID id, const QString& type, const QString& clientID )
  :mID(id), mType( type ), mClientId( clientID )
{

}

DocDigest::DocDigest()
{
}

QString DocDigest::date()
{
  return KGlobal().locale()->formatDate( mDate, true );
}

QString DocDigest::lastModified()
{
  return KGlobal().locale()->formatDate( mLastModified, true );
}

QString DocDigest::clientName()
{
  KABC::AddressBook *adrBook =  KABC::StdAddressBook::self();
  KABC::Addressee contact;
  if( adrBook ) {
     contact = adrBook->findByUid( mClientId );
  }
  QString name = contact.realName();

  return name;
}

void DocDigest::addArchDocDigest( const ArchDocDigest& digest )
{
  mArchDocs.append( digest );
}

ArchDocDigestList DocDigest::archDocDigestList()
{
  return mArchDocs;
}

/* *************************************************************************** */

DocDigestsTimeline::DocDigestsTimeline()
  :mMonth( 0 ), mYear( 0 )
{

}

DocDigestsTimeline::DocDigestsTimeline( int m,  int y )
  :mMonth( m ), mYear( y )
{

}

void DocDigestsTimeline::setDigestList( const DocDigestList& list )
{
  mDigests = list;
}
