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
#include <qsqlrecord.h>
#include <qsqlindex.h>
#include <qstringlist.h>
#include <QSqlQuery>

#include <k3staticdeleter.h>
#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>

#include "defaultprovider.h"
#include "kraftdb.h"
#include "doctext.h"
#include "kraftsettings.h"
#include "doctype.h"
#include <kstandarddirs.h>

static K3StaticDeleter<DefaultProvider> selfDeleter;

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
  QString type = KraftSettings::self()->doctype();
  if ( type.isEmpty() ) {
    QStringList allTypes = DocType::allLocalised();
    if( ! allTypes.isEmpty() ) {
      type = DocType::allLocalised()[0];
    } else {
      type = i18n( "Unknown" );
    }
  }
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

  // kDebug() << "Reading texts from DB with: " << sql << endl;

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
  dbID retVal;

  QSqlQuery q;
  if ( t.dbId().isOk() ) {
    q.prepare( "UPDATE DocTexts SET (name=:name, description=:desc, text=:text,"
               "docType=:doctype, docTypeId=:doctypeid, textType=:texttype, "
               "modDate=systemtimestamp) "
               "WHERE docTextID=:id" );
    q.bindValue( ":id",  t.dbId().toInt() );
  } else {
    // Lets insert
    q.prepare( "INSERT INTO DocTexts (name, description, text, docType, docTypeId, "
               "textType, modDate) "
               "VALUES (:name, :description, :text, :doctype, :doctypeid, :texttype, \"systemtimestamp\" )" );
  }
  q.bindValue( ":name", t.name() );
  q.bindValue( ":description", t.description() );
  q.bindValue( ":text", KraftDB::self()->mysqlEuroEncode( t.text() ) );
  q.bindValue( ":doctype", t.docType() );
  dbID id = DocType::docTypeId( t.docType() );
  q.bindValue( ":doctypeid", id.toInt() );
  q.bindValue( ":texttype", t.textTypeString() );

  q.exec();

  retVal = KraftDB::self()->getLastInsertID();

  return retVal;
}


KLocale* DefaultProvider::locale()
{
  return KGlobal::locale();
}

void DefaultProvider::deleteDocumentText( const DocText& dt )
{
  if ( dt.dbId().isOk() ) {
    QSqlQuery q;
    q.prepare("DELETE FROM DocTexts WHERE docTextID=:id") ;
    q.bindValue( ":id", dt.dbId().toInt());
    q.exec();
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

