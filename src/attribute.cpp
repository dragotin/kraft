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
  :mListValue( false ),
   mDelete( false )
{

}

Attribute::Attribute( const QString& name )
  :mName( name ),
   mListValue( false ),
   mDelete( false )
{

}

void Attribute::setRawValue( const QVariant& var )
{
  mValue = var;
}


void Attribute::setValue( const QVariant& var )
{
  if ( useRelationTable() ) {
    QSqlQuery q;
    QString query = "SELECT " + mIdCol +" FROM " + mTable + " WHERE " + mStringCol + "=:string";

    q.prepare( query  );

    // kdDebug() << "Column: " << mIdCol << " | table " << mTable << " | string: " << mStringCol << ": " << query << endl;

    if ( listValue() ) {
      QStringList idList;
      QStringList list = var.toStringList();
      for ( QStringList::Iterator valIt = list.begin(); valIt != list.end(); ++valIt ) {
        QString curValue = *valIt;
        // kdDebug() << "Searching for " << curValue << " in relation table" << endl;
        q.bindValue( ":string", curValue );
        q.exec();
        if ( q.next() ) {
          idList << q.value( 0 ).toString();
        }
      }
      mValue = QVariant( idList );
    } else {
      q.bindValue( ":string", var.toString() );
      q.exec();
      // kdDebug() << "ERROR" << q.lastError().text() << endl;
      if ( q.next() ) {
        mValue = q.value( 0 );
      }
    }
  } else {
    mValue = var;
  }
}

bool Attribute::useRelationTable()
{
  return !( mTable.isEmpty() || mIdCol.isEmpty() || mStringCol.isEmpty() );
}

QVariant Attribute::value()
{
  if ( useRelationTable() ) {
    // get the value from the relation table
    QSqlQuery q;
    QString query = "SELECT " + mStringCol +" FROM " + mTable + " WHERE " + mIdCol + "=:id";
    q.prepare( query  );

    if ( listValue() ) {
      QStringList idList = mValue.asStringList();
      QStringList::Iterator it = idList.begin();
      QStringList list;
      while( it != idList.end() ) {
        q.bindValue( ":id", *it );
        q.exec();
        while ( q.next() ) {
          QString str = q.value( 0 ).toString();
          list.append( str );
        }
        ++it;
      }
      return QVariant( list );
    } else {
      q.bindValue( ":id", mValue.toString() );
      q.exec();
      if ( q.next() ) {
        return QVariant( q.value( 0 ) );
      }
    }
  }
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

void Attribute::setValueRelation( const QString& table, const QString& idColumn,  const QString& stringColumn )
{
  mTable = table;
  mIdCol = idColumn;
  mStringCol = stringColumn;
}

QString Attribute::toString()
{
  QString re;
  re =  "+ Attribute name: " + mName + '\n';
  if ( mListValue ) {
    re += "+ Attribute Value (List): " + mValue.toStringList().join( ", " )+ '\n';
  } else {
    re += "+ Attribute Value (String): " + mValue.toString() + '\n';
  }
  re += "+ Relation Table: " + mTable + '\n';
  re += "+ Relation ID-Column: " + mIdCol + '\n';
  re += "+ Relation StringCol: " + mStringCol + '\n';
  re += "+ List: " + ( mListValue ? QString( "yes" ) : QString( "no" ) ) + '\n';

  return re;
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
  Iterator it = find( name );
  if ( it != end() ) {
    // it is there, check the delete flag.
    if (  ! ( *it ).mDelete ) return true;
  }
  return false;
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

    if ( attribQuery.next() ) {
      // the attrib exists. Check the values

      attribId = attribQuery.value(0).toString();  // the id
      if ( att.value().isNull() || att.mDelete ) {
        // the value is empty. the existing entry needs to be dropped
        dbDeleteAttribute( attribId );
        return;
      }
    } else {
      // the attrib does not yet exist. Create if att value is not null.
      if ( att.value().isNull() ) {
        kdDebug() << "oo- skip writing of attribute, value is empty" << endl;
      } else {
        kdDebug() << "oo- writing of attribute name " << att.name() << endl;
        QSqlQuery insQuery;
        insQuery.prepare( "INSERT INTO attributes (hostObject, hostId, name, valueIsList, relationTable, "
                          "relationIDColumn, relationStringColumn) "
                          "VALUES (:host, :hostId, :name, :valueIsList, :relTable, :relIDCol, :relStringCol )" );
        insQuery.bindValue( ":host", mHost );
        insQuery.bindValue( ":hostId", id.toString() );
        insQuery.bindValue( ":name", att.name() );
        insQuery.bindValue( ":valueIsList", att.listValue() );

        // Write the relation table info. These remain empty for non related attributes.
        insQuery.bindValue( ":relTable", att.mTable );
        insQuery.bindValue( ":relIDCol", att.mIdCol );
        insQuery.bindValue( ":relStringCol", att.mStringCol );

        insQuery.exec();
        dbID attId = KraftDB::self()->getLastInsertID();
        attribId = attId.toString();
      }
    }

    // store the id to be able to drop not longer existant values
    kdDebug() << "adding attribute id " << attribId << " for attribute " << att.name() << endl;

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
      newValues = att.mValue.toStringList();
      kdDebug() << "new values are: " << newValues.join( ", " ) << endl;

      if ( newValues.empty() ) {
        // delete the entire attribute.
        dbDeleteValue( attribId ); // deletes all values
        dbDeleteAttribute( attribId );
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
            kdDebug() << "Value " << curValue << " is already present with id " << valueMap[curValue] << endl;
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
      QString newValue = att.mValue.toString();  // access the attribute object directly to get the numeric
      kdDebug() << "NEW value String: " << newValue << endl;
      // value in case the attribute is bound to a relation table
      if ( newValue.isEmpty() ) {
        // delete the entire attribute
        dbDeleteValue( attribId ); // deletes all values
        dbDeleteAttribute( attribId );
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
      ValueMap::Iterator mapIt;
      for ( mapIt = valueMap.begin(); mapIt != valueMap.end(); ++mapIt ) {
        QString valId = mapIt.data();
        dbDeleteValue( attribId, valId );
      }
    }
  }
}

void AttributeMap::markDelete( const QString& name )
{
  if ( name.isEmpty() || ! contains( name ) )return;
  Iterator it = find( name );
  if ( it != end() ) {
    ( *it ).mDelete = true;
  }
}

/* remove all Attributes from the database for the given host id
 * this method clears the entire map and should only be called if
 * the whole host is to delete anyway. */
void AttributeMap::dbDeleteAll( dbID id )
{
  kdDebug() << "This is the id for to delete: " << id.toString() << endl;
  if ( !id.isOk() ) return;
  QSqlQuery listQuery;
  listQuery.prepare( "SELECT id FROM attributes WHERE hostObject=:hostObject AND hostId=:hostId" );
  listQuery.bindValue( ":hostObject", mHost );
  listQuery.bindValue( ":hostId", id.toString() );
  listQuery.exec();
  kdDebug() << "4-XXXXXXXXXXX " << listQuery.lastError().text() << endl;


  while ( listQuery.next() ) {
    dbDeleteAttribute( listQuery.value( 0 ).toString() );
  }
  clear();
}

void AttributeMap::dbDeleteAttribute( const QString& attribId )
{
  if ( attribId.isEmpty() ) return;

  QSqlQuery delQuery;
  kdDebug() << "Deleting attribute id " << attribId << endl;
  delQuery.prepare( "DELETE FROM attributes WHERE id=:id" );
  delQuery.bindValue( ":id", attribId );
  delQuery.exec();
  kdDebug() << "5-XXXXXXXXXXX " << delQuery.lastError().text() << endl;

  dbDeleteValue( attribId ); // delete all values
}

void AttributeMap::dbDeleteValue( const QString& attribId, const QString& id )
{
  QSqlQuery delQuery;

  if ( id.isEmpty() && ! attribId.isEmpty() ) {
    delQuery.prepare( "DELETE FROM attributeValues WHERE attributeId=" + attribId );
  } else if ( !id.isEmpty() ) {
    delQuery.prepare( "DELETE FROM attributeValues WHERE id="+id );
  }
  delQuery.exec();
  kdDebug() << "6-XXXXXXXXXXX " << delQuery.lastError().text() << endl;

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
    QString relTable = cur.value( "relationTable" ).toString();
    QString relIDCol = cur.value( "relationIDColumn" ).toString();
    QString relStrCol = cur.value( "relationStringColumn" ).toString();

    Attribute attr( h );
    attr.setListValue( isList );
    attr.setValueRelation( relTable, relIDCol,  relStrCol );

    crit = QString( "attributeId=%1" ).arg( cur.value( "id" ).toInt() );
    curValues.select( crit );

    QStringList values;
    QString str;
    while ( curValues.next() )  {
      if ( isList ) {
        values << curValues.value( "value" ).toString();
      } else {
        str = curValues.value( "value" ).toString();
        // kdDebug() << " attribute string " << h <<": " << str  << endl;
      }
    }
    // kdDebug() << " attribute list " << h <<": " << values  << endl;


    if ( isList ) {
      attr.setRawValue( QVariant( values ) );
    } else {
      attr.setRawValue( QVariant( str ) );
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
