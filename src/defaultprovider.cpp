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
#include "kraftsettings.h"
#include "doctype.h"
#include <kstandarddirs.h>

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

QString DefaultProvider::docType()
{
  QString type = KraftSettings::doctype();
  if ( type.isEmpty() )
    type = DocType::allLocalised()[0];
  return type;
}

DocTextList DefaultProvider::documentTexts( const QString& docType, KraftDoc::Part tt )
{
  DocTextList re;

  QString typeStr = DocText::textTypeToString( tt );

  QString sql = QString( "SELECT texts.docTextID, texts.name, texts.text, texts.description, "
                         "texts.textType, types.name as docTypeName FROM DocTexts texts, "
                         "DocTypes types WHERE texts.docTypeId=types.docTypeID AND "
                         "types.name=\'%1\' AND textType = \'%2\'").arg( docType ).arg( typeStr );

  // kdDebug() << "Reading texts from DB with: " << sql << endl;

  QSqlQuery query( sql );
  if ( query.isActive() ) {
    while ( query.next() ) {
      DocText dt;
      dt.setDbId( query.value( 0 ) /* docTextID */ .toInt() );
      dt.setName( query.value( 1 ) /* name */ .toString() );
      dt.setText( KraftDB::self()->mysqlEuroDecode( query.value( 2 ) /* text */ .toString() ) );
      dt.setDescription( query.value( 3 ) /* description */ .toString() );
      dt.setTextType( DocText::stringToTextType( query.value( 4 ) /* textType */ .toString() ) );
      dt.setDocType( query.value( 5 ) /* docType */ .toString() );

      re.append( dt );
    }
  }
  return re;
}

QString DefaultProvider::defaultText( const QString& docType, KraftDoc::Part p, DocGuardedPtr )
{
  QString re;

  DocTextList list = documentTexts( docType, p );
  DocTextList::iterator it;

  for ( it = list.begin(); it != list.end(); ++it ) {
    if ( ( *it ).name() == i18n( "Standard" ) ) {
      re = ( *it ).text();
      break;
    }
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
  buffer->setValue( "text", KraftDB::self()->mysqlEuroEncode( t.text() ) );
  buffer->setValue( "docType", t.docType() );

  dbID id = DocType::docTypeId( t.docType() );
  buffer->setValue( "docTypeId", id.toString() );
  buffer->setValue( "textType", t.textTypeString() );
  buffer->setValue( "modDate", "systimestamp" );
}

KLocale* DefaultProvider::locale()
{
  return KGlobal().locale();
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

QString DefaultProvider::currencySymbol() const
{
  return self()->locale()->currencySymbol();
}

QString DefaultProvider::iconvTool() const
{
  return KStandardDirs::findExe( "iconv" );
}

DefaultProvider::~DefaultProvider()
{

}

