/***************************************************************************
                 doctype.cpp - doc type class
                             -------------------
    begin                : Oct. 2007
    copyright            : (C) 2007 by Klaas Freitag
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

// include files for Qt
#include <qstring.h>
#include <qsqlcursor.h>

// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>

// application specific includes
#include "doctype.h"
#include "defaultprovider.h"
#include "kraftsettings.h"
#include "kraftdb.h"

/**
@author Klaas Freitag
*/

idMap DocType::mNameMap = idMap();


DocType::DocType()
  : mAttributes( QString::fromLatin1( "DocType" ) )
{
  init();
}

DocType::DocType( const QString& name )
  : mAttributes( QString::fromLatin1( "DocType" ) ),
    mName( name )
{
  init();
  if ( mNameMap.contains( name ) ) {
    dbID id = mNameMap[ name ];

    mAttributes.load( id );
  }
}

void DocType::init()
{
  if ( ! mNameMap.empty() ) return;

  QSqlCursor cur("DocTypes");
  cur.setMode( QSqlCursor::ReadOnly );
  cur.select();

  while ( cur.next() ) {
    dbID id( cur.value( "docTypeID" ).toInt() );
    mNameMap[ cur.value( "name" ).toString() ] = id;
    QString h = DefaultProvider::self()->locale()->translate( cur.value( "name" ).toString() );
    mNameMap[ h ] = id;
  }
}

void DocType::clearMap()
{
  mNameMap.clear();
}

QStringList DocType::all()
{
  init();

  QStringList re;

  QSqlCursor cur("DocTypes");
  cur.setMode( QSqlCursor::ReadOnly );
  cur.select();

  while ( cur.next() ) {
    re << cur.value( "name" ).toString();
  }

  return re;
}

QStringList DocType::allLocalised()
{
  QStringList re;

  QSqlCursor cur("DocTypes");
  cur.setMode( QSqlCursor::ReadOnly );
  cur.select();

  while ( cur.next() ) {
    re << DefaultProvider::self()->locale()->translate( cur.value( "name" ).toString() );
  }

  return re;
}

dbID DocType::docTypeId( const QString& docType )
{
  dbID id;
  init();
  if ( mNameMap.contains( docType ) ) {
    id = mNameMap[ docType ];

    return id;
  } else {
    kdError()<< "Can not find id for doctype named " << docType << endl;
  }
  return id;
}

bool DocType::allowDemand()
{
  bool re = false;

  if ( mAttributes.contains( "AllowDemand" ) ) {
    re = true;
  }
  return re;
}

bool DocType::allowAlternative()
{
  bool re = false;

  if ( mAttributes.contains( "AllowAlternative" ) ) {
    re = true;
  }
  return re;
}

QStringList DocType::follower()
{
  QStringList re;

  init();

  QSqlCursor cur( "DocTypeRelations" );
  cur.setMode( QSqlCursor::ReadOnly );

  QSqlIndex sortIndex = cur.index( "sequence" );
  QString select = QString( "typeId=%1" ).arg( mNameMap[mName].toInt() );
  kdDebug() << "SQL: " << select << endl;
  cur.select( select, sortIndex );

  while ( cur.next() ) {
    dbID followerId( cur.value( "followerId" ).toInt() );

    idMap::Iterator it;
    for ( it = mNameMap.begin(); it != mNameMap.end(); ++it ) {
      kdDebug() << it.key()  << endl;
      if ( it.data() == followerId ) {
        re << it.key();
      }
    }
  }
  return re;
}

QString DocType::numberCycleName()
{
  QString numberCycle( "default" );
  if ( mAttributes.contains( "identNumberCycle" ) ) {
    numberCycle = mAttributes["identNumberCycle"].value().toString();
    kdDebug() << "DocType using special numbercycle " << numberCycle << endl;
  }
  return numberCycle;
}

QString DocType::generateDocumentIdent( KraftDoc *doc, int id )
{
  /*
   * The pattern may contain the following tags:
   * %y - the year of the documents date.
   * %w - the week number of the documents date
   * %d - the day number of the documents date
   * %m - the month number of the documents date
   * %c - the customer id from kaddressbook
   * %i - the uniq identifier from db.
   * %type - the localised doc type (offer, invoice etc.)
   */

  QString pattern = identTemplate();
  if ( pattern.find( "%i" ) == -1 ) {
    kdWarning() << "No %i found in identTemplate, appending it to meet law needs!" << endl;
    pattern += "-%i";
  }
  QDate d = QDate::currentDate();
  if ( doc ) d = doc->date();

  KraftDB::StringMap m;
  int dummy;

  m[ "%y" ] = QString::number( d.year() );
  m[ "%w" ] = QString::number( d.weekNumber( &dummy ) );
  m[ "%d" ] = QString::number( d.day()  );
  m[ "%m" ] = QString::number( d.month() );
  if ( id == -1 ) { // hot mode: The database id is incremented by nextIdentId()
    m[ "%i" ] = QString::number( nextIdentId() );
  } else {
    m[ "%i" ] = QString::number( id );
  }
  if ( doc ) {
    m[ "%c" ] = doc->addressUid();
    m[ "%type" ] = doc->docType();
  } else {
    m[ "%c"] = QString(" <addressUid>" );
    m[ "%type" ] = mName;
  }

  QString re = KraftDB::self()->replaceTagsInWord( pattern, m );
  kdDebug() << "Generated document ident: " << re << endl;

  return re;
}

// if hot, the id is updated in the database, otherwise not.
int DocType::nextIdentId( bool hot )
{
  QString numberCycle = numberCycleName();

  if ( numberCycle.isEmpty() ) {
    kdError() << "NumberCycle name is empty" << endl;
    return -1;
  }

  QSqlQuery qLock;
  if ( hot ) {
    qLock.exec( "LOCK TABLES numberCycles WRITE" );
  }

  QSqlQuery q;
  q.prepare( "SELECT lastIdentNumber FROM numberCycles WHERE name=:name" );

  int num = -1;
  q.bindValue( ":name", numberCycle );
  q.exec();
  if ( q.next() ) {
    num = 1+( q.value( 0 ).toInt() );
    kdDebug() << "Got current number: " << num << endl;

    if ( hot ) {
      QSqlQuery setQuery;
      setQuery.prepare( "UPDATE numberCycles SET lastIdentNumber=:newNumber WHERE name=:name" );
      setQuery.bindValue( ":name", numberCycle );
      setQuery.bindValue( ":newNumber", num );
      setQuery.exec();
      if ( setQuery.isActive() ) {
        kdDebug() << "Successfully created new id number for numbercycle " << numberCycle << ": "
                  << num << endl;
      }
    }
  }
  if ( hot ) {
    qLock.exec( "UNLOCK TABLES" );
  }

  return num;
}

QString DocType::identTemplate()
{
  QSqlQuery q;
  QString tmpl;
  const QString defaultTempl = QString::fromLatin1( "%y%w-%i" );

  QString numberCycle = numberCycleName();
  if ( numberCycle.isEmpty() ) {
    kdError() << "Numbercycle for doctype is empty, returning default" << endl;
    return defaultTempl;
  }
  kdDebug() << "Picking ident Template for numberCycle " << numberCycle << endl;

  q.prepare( "SELECT identTemplate FROM numberCycles WHERE name=:name" );

  q.bindValue( ":name", numberCycle );
  q.exec();
  if ( q.next() ) {
    tmpl = q.value( 0 ).toString();
    kdDebug() << "Read ident template from database: " << tmpl << endl;
  }

  if ( tmpl.isEmpty() ) {
    // migration: If there is nothing yet in the database, check the local config and
    // transfer the setting to the db
    QString pattern = KraftSettings::self()->docIdent();
    if ( pattern.isEmpty() ) {
      // There is nothing in KConfig File, so we use our default from here.
      pattern = defaultTempl;
    }
    kdDebug() << "Writing ident template to database: " << pattern << endl;
    QSqlQuery insQuery;
    insQuery.prepare( "UPDATE numberCycles SET identTemplate=:pattern WHERE name=:name" );
    insQuery.bindValue( ":name", numberCycle );
    insQuery.bindValue( ":pattern", pattern );
    insQuery.exec();
    tmpl = pattern;
  }

  return tmpl;
}

QString DocType::name() const
{
  return mName;
}

void DocType::setName( const QString& name )
{
  mName = name;
}


