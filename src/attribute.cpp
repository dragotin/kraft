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

#include <QString>
#include <QVariant>
#include <QSqlQuery>
#include <QStringList>

#include <QDebug>

#include "attribute.h"
#include "kraftdb.h"
#include "dbids.h"

Attribute::Attribute()
  :mPersist( true ),
   mListValue( false ),
   mDelete( false )
{

}

Attribute::Attribute( const QString& name )
  :mName( name ),
   mPersist( true ),
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

    // qDebug() << "Column: " << mIdCol << " | table " << mTable << " | string: " << mStringCol << ": " << query;

    if ( listValue() ) {
      QStringList idList;
      QStringList list = var.toStringList();
      for ( QStringList::Iterator valIt = list.begin(); valIt != list.end(); ++valIt ) {
        QString curValue = *valIt;
        // qDebug() << "Searching for " << curValue << " in relation table";
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
      // qDebug() << "ERROR" << q.lastError().text();
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
      QStringList idList = mValue.toStringList();
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
    Attribute att = it.value();
    // qDebug () << ">> oo-  saving attribute with name " << it.key() << " for " << id.toString() << " att-name: " << att.name();

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
        // qDebug () << "oo- skip writing of attribute, value is empty";
      } else {
        // qDebug () << "oo- writing of attribute name " << att.name();
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

    // store the id to be able to drop not longer existent values
    // qDebug () << "adding attribute id " << attribId << " for attribute " << att.name();

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
      // qDebug () << "new values are: " << newValues.join( ", " );

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
            // qDebug () << "Value " << curValue << " is already present with id " << valueMap[curValue];
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
      // qDebug () << "NEW value String: " << newValue;
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
          // qDebug () << "insert new attrib value for non list: " << newValue;

        } else {
          QString oldValue = valueMap.begin().key();
          QString id = valueMap.begin().value();

          if ( newValue != oldValue ) {
            // qDebug () << "Updating " << id << " from " << oldValue << " to " << newValue;
            QSqlQuery updateQuery;
            updateQuery.prepare( "UPDATE attributeValues SET value=:val WHERE id=:id" );
            updateQuery.bindValue( ":val", newValue );
            updateQuery.bindValue( ":id",  id );
            // qDebug () << "do the update!";
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
        QString valId = mapIt.value();
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
    // qDebug () << "Marking attrib " << name << " to delete!";
  }
}

/* remove all Attributes from the database for the given host id
 * this method clears the entire map and should only be called if
 * the whole host is to delete anyway. */
void AttributeMap::dbDeleteAll( dbID id )
{
  // qDebug () << "This is the id for to delete: " << id.toString();
  if ( !id.isOk() ) return;
  QSqlQuery listQuery;
  listQuery.prepare( "SELECT id FROM attributes WHERE hostObject=:hostObject AND hostId=:hostId" );
  listQuery.bindValue( ":hostObject", mHost );
  listQuery.bindValue( ":hostId", id.toString() );
  listQuery.exec();
  // qDebug () << "4-XXXXXXXXXXX " << listQuery.lastError().text();


  while ( listQuery.next() ) {
    dbDeleteAttribute( listQuery.value( 0 ).toString() );
  }
  clear();
}

void AttributeMap::dbDeleteAttribute( const QString& attribId )
{
  if ( attribId.isEmpty() ) return;

  QSqlQuery delQuery;
  // qDebug () << "Deleting attribute id " << attribId;
  delQuery.prepare( "DELETE FROM attributes WHERE id=:id" );
  delQuery.bindValue( ":id", attribId );
  delQuery.exec();
  // qDebug () << "5-XXXXXXXXXXX " << delQuery.lastError().text();

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
  // qDebug () << "6-XXXXXXXXXXX " << delQuery.lastError().text();

}

void AttributeMap::load( dbID id )
{
  QSqlQuery q1;
  q1.prepare("SELECT id, name, valueIsList, relationTable, relationIDColumn, relationStringColumn FROM attributes WHERE hostObject=:hostObject AND hostId=:hostId");
  q1.bindValue(":hostObject", mHost);
  q1.bindValue(":hostId", id.toInt());
  q1.exec();

  checkHost();

  while ( q1.next() ) {
    QString h = q1.value( 1 ).toString();
    bool isList = q1.value( 2 ).toBool();
    QString relTable = q1.value( 3 ).toString();
    QString relIDCol = q1.value( 4 ).toString();
    QString relStrCol = q1.value( 5 ).toString();

    Attribute attr( h );
    attr.setListValue( isList );
    attr.setValueRelation( relTable, relIDCol,  relStrCol );

    QSqlQuery q2;
    q2.prepare("SELECT value FROM attributeValues WHERE attributeId=:id");
    q2.bindValue(":id", q1.value(0).toInt());
    q2.exec();

    QStringList values;
    QString str;
    while ( q2.next() )  {
      if ( isList ) {
        values << q2.value( 0 ).toString();
      } else {
        str = q2.value( 0 ).toString();
        // qDebug() << " attribute string " << h <<": " << str;
      }
    }
    // qDebug() << " attribute list " << h <<": " << values;


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
    // qDebug () << "Host for attributes unset, assuming unknown";
    mHost = "unknown";
  }
}
