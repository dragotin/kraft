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
#include "doctext.h"

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

DocTextList DefaultProvider::documentTexts( const QString& docType, KraftDoc::Part tt )
{
  DocTextList re;

  QSqlCursor cur( "DocTexts" );
  cur.setMode( QSqlCursor::ReadOnly );

  QString typeStr = DocText::textTypeToString( tt );
  QString crit = QString( "docType=\'%1\' AND textType=\'%2\'" ).arg( docType ).arg( typeStr );
  cur.select( crit );

  while ( cur.next() ) {
    DocText dt;
    dt.setDbId( cur.value( "docTextID" ).toInt() );
    dt.setName( cur.value( "name" ).toString() );
    dt.setText( cur.value( "text" ).toString() );
    dt.setDescription( cur.value( "description" ).toString() );
    dt.setTextType( DocText::stringToTextType( cur.value( "textType" ).toString() ) );
    dt.setDocType( cur.value( "docType" ).toString() );

    re.append( dt );
  }
  return re;
}


QString DefaultProvider::documentText( const QString& docType, KraftDoc::Part p, DocGuardedPtr )
{
  // FIXME: Later on use the document information to do replaces in the wordlist template
  QString re;

  QStringList validType = docTypes().grep( docType );
  if ( ! validType.count() ) {
    kdDebug() << "ERR: Do not know the docType " << docType << endl;
    return re;
  }
  // read the wordlist
  QString textType = KraftDoc::partToString( p );
  QStringList l = KraftDB::self()->wordList( QString( "doc%1_%2" ).arg( textType ).arg( docType ) );
  if ( l.count() ) {
    re = l[0]; // return the first of the list
  }
  return re;
}

dbID DefaultProvider::saveDocumentText( const DocText& t )
{
  QSqlCursor cur( "DocTexts" );
  cur.setMode( QSqlCursor::Writable );
  dbID retVal;

  if ( t.dbId().isOk() ) {
    // Update required.
    QString crit = QString( "docTextID=%1" ).arg( t.dbId().toInt() );
    cur.select( crit );
    if ( cur.next() ) {
      QSqlRecord *buffer = cur.primeUpdate();
      fillDocTextBuffer( t, buffer );
      retVal = t.dbId();
      cur.update();
    }
  } else {
    // Lets insert
    QSqlRecord *buffer = cur.primeInsert();
    fillDocTextBuffer( t, buffer );
    cur.insert();

    retVal = KraftDB::self()->getLastInsertID();
  }
  return retVal;
}

void DefaultProvider::fillDocTextBuffer( const DocText& t, QSqlRecord *buffer )
{
  if ( ! buffer ) return;

  buffer->setValue( "name", t.name() );
  buffer->setValue( "description", t.description() );
  buffer->setValue( "text", t.text() );
  buffer->setValue( "docType", t.docType() );
  buffer->setValue( "textType", t.textTypeString() );
  buffer->setValue( "modDate", "systimestamp" );
}


void DefaultProvider::deleteDocumentText( const DocText& dt )
{
  QSqlCursor cur( "DocTexts" );

  // QString sql = QString( "name=\'%1\' AND docType=\'%2\' AND textType=\'%3\'" )
  if ( dt.dbId().isOk() ) {
    QString sql = QString( "docTextID=%1" ).arg( dt.dbId().toInt() );
    cur.select( sql );
    if ( cur.next() ) {
      cur.primeDelete();
      cur.del();
    }
  }
}


DefaultProvider::~DefaultProvider()
{

}

