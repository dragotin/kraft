/***************************************************************************
             unitmanager  -
                             -------------------
    begin                : 2004-05-05
    copyright            : (C) 2004 by Klaas Freitag
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

#include <QSqlQuery>

// include files for KDE
#include <kraftdb.h>
#include <QDebug>

#include "unitmanager.h"
#include "einheit.h"

Q_GLOBAL_STATIC(UnitManager, mSelf)

UnitManager* UnitManager::self()
{
  return mSelf;
}

UnitManager::UnitManager( )
    : mFullTax( -1 ),
      mReducedTax( -1 )
{

}

#if 0
void UnitManager::clearTaxCache()
{
  mFullTax = -1;
  mReducedTax = -1;
}
#endif


/* These tax related functions are just dropped here for the moment because they
 * are pretty similar to the units in terms of reading from DB etc. Clear FIXME.
 */

double UnitManager::tax( const QDate& date )
{
  if ( mFullTax < 0 || date != mTaxDate )
    readTaxes( date );
  return mFullTax;
}

double UnitManager::reducedTax( const QDate& date )
{
  if ( mReducedTax < 0 || date != mTaxDate )
    readTaxes( date );
  return mReducedTax;
}

bool UnitManager::readTaxes( const QDate& date )
{
  QString sql;
  QSqlQuery q;
  sql = "SELECT fullTax, reducedTax, startDate FROM taxes ";
  sql += "WHERE startDate <= :date ORDER BY startDate DESC LIMIT 1";

  q.prepare( sql );
  QString dateStr = date.toString( "yyyy-MM-dd" );
  // qDebug () << "** Datestring: " << dateStr;
  q.bindValue( ":date", dateStr );
  q.exec();

  if ( q.next() ) {
    mFullTax    = q.value( 0 ).toDouble();
    mReducedTax = q.value( 1 ).toDouble();
    mTaxDate = date;
    // qDebug () << "* Taxes: " << mFullTax << "/" << mReducedTax << " from " << q.value( 2 ).toDate();
  }
  return ( mFullTax > 0 && mReducedTax > 0 );
}

/* ============================================================================================= */

void UnitManager::load()
{
  QSqlQuery q( "SELECT unitID, unitShort, unitLong, unitPluShort, unitPluLong, ec20 FROM units");

  while( q.next())
  {
    int unitID = q.value(0).toInt();
    Einheit e( unitID,
               q.value(1).toString(),
               q.value(2).toString(),
               q.value(3).toString(),
               q.value(4).toString(),
               q.value(5).toString());
    mUnits.append(e);
  }
}

int UnitManager::nextFreeId()
{
    int id = 0;
    if( mUnits.size() == 0 ) {
        load();
    }
    foreach( Einheit u, mUnits ) {
        if( u.id() > id ) {
            id = u.id();
        }
    }
    return id+1;
}

QStringList UnitManager::allUnits()
{
  QStringList list;

  if(mUnits.size() == 0 ) load();
  foreach( Einheit e, mUnits ) {
    QString uSing = e.einheitSingular();
    if( !uSing.isEmpty())
      list << uSing;
  }
  return list;
}

Einheit UnitManager::getPauschUnit()
{
    int id = getUnitIDSingular(QStringLiteral("pausch."));
    if (id > -1)
        return getUnit(id);
    return Einheit();
}

Einheit UnitManager::getUnit( int id )
{
  if( mUnits.size() == 0 ) load();

  // qDebug() << "Searching unit ID " << id << endl;
  for( Einheit e: mUnits ) {
    if( e.id() == id ) return e;
  }
  return Einheit();
}

Einheit UnitManager::getUnit(const QString& singularUnit)
{
    if( mUnits.size() == 0 ) load();

    // qDebug() << "Searching unit ID " << id << endl;
    for( Einheit e: mUnits ) {
      if( e.einheitSingular() == singularUnit)
          return e;
    }
    return Einheit();
}

int UnitManager::getUnitIDSingular( const QString& einheitStr )
{
  if( mUnits.size() == 0 ) load();

  foreach( Einheit tmp, mUnits ) {

    if( tmp.einheitSingular() == einheitStr ||
        tmp.einheitPlural()   == einheitStr ) {
      // qDebug() << "Thats it, returning " << tmp.id();
      return tmp.id();
    }
  }
  return -1;
}

QString UnitManager::getECE20(const QString& einheitStr)
{
    if( mUnits.size() == 0 ) load();

    for( Einheit tmp: mUnits ) {
      if( tmp.einheitSingular() == einheitStr ||
          tmp.einheitPlural()   == einheitStr ) {
        // qDebug() << "Thats it, returning " << tmp.id();
        return tmp.ec20();
      }
    }
    return QString();
}

UnitManager::~UnitManager( )
{
}

/* END */


