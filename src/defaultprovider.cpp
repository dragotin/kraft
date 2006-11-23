/***************************************************************************
                  defaultprovider.cpp  - Default Providing Class
                             -------------------
    begin                : November 2006
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
#include <qsqlcursor.h>
#include <qsqlrecord.h>
#include <qsqlindex.h>
#include <qstringlist.h>

#include <kstaticdeleter.h>
#include <klocale.h>

#include <kdebug.h>

#include "defaultprovider.h"
#include "kraftdb.h"

static KStaticDeleter<DefaultProvider> selfDeleter;

DefaultProvider* DefaultProvider::mSelf = 0;

DefaultProvider *DefaultProvider::self()
{
  if ( !mSelf ) {
    selfDeleter.setObject( mSelf, new DefaultProvider() );
  }
  return mSelf;
}

DefaultProvider::DefaultProvider()
{

}

QStringList DefaultProvider::docTypes()
{
  QStringList re;
  re << i18n("Offer");
  re << i18n("Invoice");
  re << i18n("Acceptance of Order");

  return re;
}

QString DefaultProvider::docType()
{
  return i18n( "Offer" );
}

QString DefaultProvider::documentText( const QString& docType, const QString&textType, DocGuardedPtr )
{
  // FIXME: Later on use the document information to do replaces in the wordlist template
  QString re;

  QStringList validType = docTypes().grep( docType );
  if ( ! validType.count() ) {
    kdDebug() << "ERR: Do not know the docType " << docType << endl;
    return re;
  }
  // read the wordlist
  QStringList l = KraftDB::self()->wordList( QString( "doc%1_%2" ).arg( textType ).arg( docType ) );
  if ( l.count() ) {
    re = l[0]; // return the first of the list
  }
  return re;
}

void DefaultProvider::saveDocumentText( const QString& docType, const QString& textType, const QString& text )
{
  QStringList wl;
  wl << text;

  KraftDB::self()->writeWordList( QString( "doc%1_%2" ).arg( textType ).arg( docType ), wl );
}

DefaultProvider::~DefaultProvider()
{

}

