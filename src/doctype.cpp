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
    QString h = KGlobal().locale()->translate( cur.value( "name" ).toString() );
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
    re << KGlobal().locale()->translate( cur.value( "name" ).toString() );
  }

  return re;
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
