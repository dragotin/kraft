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
#include "kraftdb.h"
#include "dbids.h"
#include <qsqlcursor.h>

Attribute::Attribute()
  :mListValue( false )
{

}

Attribute::Attribute( const QString& name )
  :mName( name ),
   mListValue( false )
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

void Attribute::setListValue( bool p )
{
  mListValue = p;
}

bool Attribute::listValue()
{
  return mListValue;
}

/*
 * Attribute Map ============================================================
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

void AttributeMap::setHost( const QString& host )
{
  mHost = host;
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

  QSqlQuery attribQuery;
  attribQuery.prepare( "SELECT id, valueIsList FROM attributes WHERE hostObject=:host AND hostId=:hostId AND name=:name" );

  attribQuery.bindValue( ":host", mHost );
  attribQuery.bindValue( ":hostId", id.toString() );

  Iterator it;
  for ( it = begin(); it != end(); ++it ) {
    Attribute att = it.data();
    kdDebug() << ">> oo-  saving attribute with name " << it.key() << " for " << id.toString() << " att-name: " << att.name() << endl;

    attribQuery.bindValue( ":name", att.name() );
    attribQuery.exec();

    QString attribId;
    QStringList seenIds;

    if ( attribQuery.next() ) {
      // the attrib exists. Check the values

      attribId = attribQuery.value(0).toString();  // the id
      if ( att.value().isNull() ) {
        // the value is empty. the existing entry needs to be dropped
        deleteAttribute( attribId );
      }
      // FIXME: check if the listvalue is correct in the existing entry.
    } else {
      // the attrib does not yet exist. Create if att value is not null.
      if ( att.value().isNull() ) {
        kdDebug() << "oo- skip writing of attribute, value is empty" << endl;
      } else {
        kdDebug() << "oo- writing of attribute name " << att.name() << endl;
        QSqlQuery insQuery;
        insQuery.prepare( "INSERT INTO attributes (hostObject, hostId, name, valueIsList) VALUES (:host, :hostId, :name, :valueIsList )" );
        insQuery.bindValue( ":host", mHost );
        insQuery.bindValue( ":hostId", id.toString() );
        insQuery.bindValue( ":name", att.name() );
        insQuery.bindValue( ":valueIsList", att.listValue() );
        insQuery.exec();
        dbID attId = KraftDB::self()->getLastInsertID();
        attribId = attId.toString();
      }
    }

    // store the id to be able to drop not longer existant values
    seenIds << attribId;
    kdDebug() << "adding attribute id " << attribId << endl;

    // now there is a valid entry in the attribute table. Check the values.
    QSqlQuery valueQuery( "SELECT id, value FROM attributeValues WHERE attributeId=" + attribId );

    typedef QMap<QString, QString> ValueMap;
    ValueMap valueMap;

    while ( valueQuery.next() ) {
      QString idStr = valueQuery.value( 0 ).toString(); // id
      QString valStr = valueQuery.value( 1 ).toString(); // value

      valueMap[valStr] = idStr;
    }

    // create a stringlist with the current values of the attribute
    if ( att.listValue() ) {
      QStringList newValues;
      newValues = att.value().toStringList();
      kdDebug() << "new values are: " << newValues.join( ", " ) << endl;

      if ( newValues.empty() ) {
        // delete the entire attribute.
        deleteValue( attribId ); // deletes all values
        deleteAttribute( attribId );
        valueMap.clear();
      } else {
        // we really have new values

        QSqlQuery insValue;
        insValue.prepare( "INSERT INTO attributeValues (attributeId, value) VALUES (:attribId, :val)" );
        insValue.bindValue( ":attribId", attribId );

        // loop over all existing new values of the attribute.
        for ( QStringList::Iterator valIt = newValues.begin(); valIt != newValues.end(); ++valIt ) {
          QString curValue = *valIt;

          if ( valueMap.contains( curValue ) ) {
            // the valueMap is already saved. remove it from the valueMap string
            kdDebug() << "Value <" << curValue << " is already present with id " << valueMap[curValue] << endl;
            valueMap.remove( curValue );
          } else {
            // the value is not yet there, insert it.
            insValue.bindValue( ":val", curValue );
            insValue.exec();
          }
        }
      }
    } else {
      // only a single entry for the attribte, update if needed.
      QString newValue = att.value().toString();
      if ( newValue.isEmpty() ) {
        // delete the entire attribute
        deleteValue( attribId ); // deletes all values
        deleteAttribute( attribId );
        valueMap.clear();
      } else {
        if ( valueMap.empty() ) {
          // there is no entry yet that could be updated.
          QSqlQuery insertQuery;
          insertQuery.prepare( "INSERT INTO attributeValues (attributeId, value ) VALUES (:id, :val)" );
          insertQuery.bindValue( ":id", attribId );
          insertQuery.bindValue( ":val", newValue );

          insertQuery.exec();
          kdDebug() << "insert new attrib value for non list: " << newValue << endl;

        } else {
          QString oldValue = valueMap.begin().key();
          QString id = valueMap.begin().data();

          if ( newValue != oldValue ) {
            kdDebug() << "Updating " << id << " from " << oldValue << " to " << newValue << endl;
            QSqlQuery updateQuery;
            updateQuery.prepare( "UPDATE attributeValues SET value=:val WHERE id=:id" );
            updateQuery.bindValue( ":val", newValue );
            updateQuery.bindValue( ":id",  id );
            kdDebug() << "do the update!" << endl;
            updateQuery.exec();
          }
          valueMap.remove( oldValue );
        }
      }
    }

    // remove all still existing entries in the valueMap because they point to values which are
    // in the db but were deleted from the attribute
    if ( ! valueMap.isEmpty() ) {
      QSqlQuery delValue;

      ValueMap::Iterator mapIt;
      for ( mapIt = valueMap.begin(); mapIt != valueMap.end(); ++mapIt ) {
        QString valId = mapIt.data();
        deleteValue( attribId, valId );
      }
    }
  }
}

void AttributeMap::deleteAttribute( const QString& attribId )
{
  if ( attribId.isEmpty() ) return;

  QSqlQuery delQuery;
  delQuery.prepare( "DELETE FROM attributes WHERE id=:id" );
  delQuery.bindValue( ":id", attribId );
  delQuery.exec();

  deleteValue( attribId ); // delete all values
}

void AttributeMap::deleteValue( const QString& attribId, const QString& id )
{
  QSqlQuery delQuery;

  if ( id.isEmpty() && ! attribId.isEmpty() ) {
    delQuery.prepare( "DELETE FROM attributeValues WHERE attributeId=" + attribId );
  } else if ( !id.isEmpty() ) {
    delQuery.prepare( "DELETE FROM attributeValues WHERE id="+id );
  }
  delQuery.exec();
}

void AttributeMap::load( dbID id )
{
  QSqlCursor cur( "attributes" );
  cur.setMode( QSqlCursor::ReadOnly );
  QSqlCursor curValues(  "attributeValues" );
  curValues.setMode( QSqlCursor::ReadOnly );
  checkHost();

  QString crit;
  crit = QString( "hostObject='%1' AND hostId=%2" ).arg( mHost ).arg( id.toInt() );
  cur.select( crit );

  while ( cur.next() ) {
    QString h = cur.value( "name" ).toString();
    bool isList = cur.value( "valueIsList" ).toBool();

    kdDebug() << "Loading attribute " << h  << endl;
    Attribute attr( h );
    attr.setListValue( isList );

    crit = QString( "attributeId=%1" ).arg( cur.value( "id" ).toInt() );
    curValues.select( crit );

    QStringList values;
    QString str;
    while ( curValues.next() )  {
      if ( isList ) {
        values << curValues.value( "value" ).toString();
      } else {
        str = curValues.value( "value" ).toString();
      }
    }

    if ( isList ) {
      attr.setValue( QVariant( values ) );
    } else {
      attr.setValue( QVariant( str ) );
    }
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
