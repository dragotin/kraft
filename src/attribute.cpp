/***************************************************************************
                 attribute.cpp - generic attribute object
                             -------------------
    begin                : Aug. 2007
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

#include <qstring.h>
#include <qvariant.h>
#include <qvaluelist.h>
#include <qsqlquery.h>

#include <kdebug.h>

#include "attribute.h"
#include "dbids.h"
#include <qsqlcursor.h>

Attribute::Attribute()
{

}

Attribute::Attribute( const QString& name )
  :mName( name )
{

}

void Attribute::setValue( const QVariant& var )
{
  mValue = var;
}

QVariant Attribute::value() const
{
  return mValue;
}

QString Attribute::name() const
{
  return mName;
}

bool Attribute::persistant()
{
  return mPersist;
}

void Attribute::setPersistant( bool p )
{
  mPersist = p;
}

/*
 * Attribute Map
 */

AttributeMap::AttributeMap()
  :QMap<QString, Attribute>()
{

}

AttributeMap::AttributeMap( const QString& host)
  :QMap<QString, Attribute>(),
   mHost( host )
{

}

bool AttributeMap::hasAttribute( const QString& name )
{
  return contains( name );
}


/*
 * this method saves the attribute together with the host string that
 * defines the type of object that this attribute is associated to (like
 * position or document) and the hosts database id.
 */
void AttributeMap::save( dbID id )
{
  checkHost();

  QSqlQuery qd;
  qd.prepare( "DELETE FROM attributes WHERE hostObject=:hostType AND hostId=:hostId" );
  qd.bindValue( ":hostType", mHost );
  qd.bindValue( ":hostId",   id.toString() );
  qd.exec();

  QSqlQuery qi;
  qi.prepare( "INSERT INTO attributes (hostObject, hostId, name, value) VALUES "
              "(:hostType, :hostId, :name, :value )" );

  qi.bindValue( ":hostType", mHost );
  qi.bindValue( ":hostId",   id.toString() );

  Iterator it;
  for ( it = begin(); it != end(); ++it ) {
    kdDebug() << "+++++++++++++++++++  saving attribute with name " << it.key() << endl;
    if ( it.data().persistant() ) {
      qi.bindValue( ":name", it.key() );
      qi.bindValue( ":value", it.data().value().toString() );
    } else {
      kdDebug() << "+++++++++++++++++++  about to save attribute, but not persistant!" << endl;
    }
    qi.exec();
  }
}

void AttributeMap::load( dbID id )
{
  QSqlCursor cur( "attributes" );
  cur.setMode( QSqlCursor::ReadOnly );
  checkHost();

  QString crit;
  crit = QString( "hostObject='%1' AND hostId=%2" ).arg( mHost ).arg( id.toInt() );
  cur.select( crit );

  while ( cur.next() ) {
    QString h = cur.value( "name" ).toString();
    kdDebug() << "Loading attribute " << h  << endl;
    Attribute attr( h );
    attr.setValue( cur.value( "value" ) );
    attr.setPersistant( true );

    insert( h, attr );
  }
}

void AttributeMap::checkHost()
{
  if ( mHost.isEmpty() ) {
    kdDebug() << "Host for attributes unset, assuming unknown" << endl;
    mHost = "unknown";
  }
}
