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
{

}

void UnitManager::load()
{
  QSqlQuery q( "SELECT unitID, unitShort, unitLong, unitPluShort, unitPluLong FROM units");

  while( q.next())
  {
    int unitID = q.value(0).toInt();
    Einheit e( unitID,
               q.value(1).toString(),
               q.value(2).toString(),
               q.value(3).toString(),
               q.value(4).toString() );
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
  foreach( Einheit e, mUnits ) {
    if( e.id() == id ) return e;
  }
  return Einheit();
}

int UnitManager::getUnitIDSingular( const QString& einheitStr )
{
  if( mUnits.size() == 0 ) load();

  foreach( Einheit tmp, mUnits ) {

    if( tmp.einheitSingular() == einheitStr ||
        tmp.einheitPlural()   == einheitStr ) {
      // qDebug() << "Thats it, returning " << tmp.id() << endl;
      return tmp.id();
    }
  }
  return -1;
}

UnitManager::~UnitManager( )
{
}

/* END */


