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
#include <kconfig.h>


#include <kabc/addressee.h>

#include "docdigest.h"
#include "defaultprovider.h"

DocDigest::DocDigest( dbID id, const QString& type, const QString& clientID )
  :mID(id), mType( type ), mClientId( clientID ), mLocale( "kraft" )
{

}

DocDigest::DocDigest()
  :mLocale( "kraft" )
{
}

QString DocDigest::date()
{
  return mLocale.formatDate( mDate, KLocale::ShortDate );
}

QString DocDigest::lastModified()
{
  return mLocale.formatDateTime( mLastModified, KLocale::ShortDate );
}

void DocDigest::appendArchDocDigest( const ArchDocDigest& digest )
{
  mArchDocs.append( digest );
}

ArchDocDigestList DocDigest::archDocDigestList()
{
  return mArchDocs;
}

void DocDigest::setCountryLanguage( const QString& country, const QString& lang )
{
  KConfig *cfg = KGlobal::config().data();
  mLocale.setCountry( country, cfg );
  mLocale.setLanguage( lang, cfg ); // FIXME !!
}

KABC::Addressee DocDigest::addressee() const
{
  return mContact;
}
void DocDigest::setAddressee( const KABC::Addressee& contact )
{
  mContact = contact;
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
