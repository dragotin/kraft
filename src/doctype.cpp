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
  QSqlQuery q;
  q.prepare( "SELECT docTypeID FROM DocTypes WHERE name=:name" );
  q.bindValue( ":name", docType );
  q.exec();
  if ( q.next() ) {
    int id = q.value( 0 ).toInt();
    return dbID( id );
  }
  return dbID();

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

int DocType::nextIdentId()
{
  QString numberCycle = numberCycleName();

  if ( numberCycle.isEmpty() ) {
    kdError() << "NumberCycle name is empty" << endl;
    return -1;
  }

  QSqlQuery qLock;
  qLock.exec( "LOCK TABLES numberCycles WRITE" );

  QSqlQuery q;
  q.prepare( "SELECT lastIdentNumber FROM numberCycles WHERE name=:name" );

  int num = -1;
  q.bindValue( ":name", numberCycle );
  q.exec();
  if ( q.next() ) {
    num = 1+( q.value( 0 ).toInt() );
    kdDebug() << "Got current number: " << num << endl;
  }

  QSqlQuery setQuery;
  setQuery.prepare( "UPDATE numberCycles SET lastIdentNumber=:newNumber WHERE name=:name" );
  setQuery.bindValue( ":name", numberCycle );
  setQuery.bindValue( ":newNumber", num );
  setQuery.exec();
  if ( setQuery.isActive() ) {
    kdDebug() << "Successfully created new id number for numbercycle " << numberCycle << ": " << num << endl;
  }
  qLock.exec( "UNLOCK TABLES" );

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
  kdDebug() << "ERROR" << q.lastError().text() << endl;
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
