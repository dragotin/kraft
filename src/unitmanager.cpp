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

#include <stdlib.h>

// include files for Qt
#include <qsqlcursor.h>

// include files for KDE
#include <kraftdb.h>
#include <klocale.h>
#include <kdebug.h>

// include files for standard functions
#include <stdlib.h>

#include "unitmanager.h"
#include "einheit.h"

UnitManager::UnitManager( )
{
}

void UnitManager::load()
{
  int max = 5;
  if( ! m_units )
    m_units = new EinheitValueVector();
  m_dummy = new Einheit();
  m_units->resize(max+1);

  QSqlCursor cur("units");

  // Create an index that sorts from high values for einheitID down.
  // that makes at least on resize of the vector.
  QSqlIndex indx = cur.index( "unitID" );
  indx.setDescending ( 0, true );

  cur.select(indx);
  while( cur.next())
  {
    int unitID = cur.value("unitID").toInt();
    // resize if index is to big.
    if( unitID > max )
    {
      max = unitID;
      m_units->resize( max+1);
    }

    Einheit e( unitID,
               cur.value("unitShort").toString(),
               cur.value("unitLong").toString(),
               cur.value("unitPluShort").toString(),
               cur.value("unitPluLong").toString() );
    m_units->at(unitID) = e;
  }
}

QStringList UnitManager::allUnits()
{
    QStringList list;

    if(! m_units ) load();
    EinheitValueVector::iterator it;
    for( it = m_units->begin(); it && it != m_units->end(); ++it )
    {
        QString uSing = (*it).einheitSingular();
        if( !uSing.isEmpty())
            list << (*it).einheitSingular();
    }
    return list;
}


Einheit& UnitManager::getUnit( int id )
{
    if( ! m_units || m_units->isEmpty() ) load();

    // kdDebug() << "Searching unit ID " << id << endl;
    bool ok;

    Einheit re;
    if( id >= 0 && ((unsigned)abs(id)) < m_units->size()) {
        re = m_units->at(id, &ok );
        if( ! ok ) {
            kdDebug() << "No Unit for id " << id << endl;
        } else {
            return m_units->at(id, &ok );
        }
    }
    return *m_dummy;
}

int UnitManager::getUnitIDSingular( const QString& einheitStr )
{
    if( ! m_units || m_units->isEmpty() ) load();

    for( uint i = 0; i < m_units->size(); i++ )
    {
        Einheit tmp = m_units->at(i);

        if( tmp.einheitSingular() == einheitStr ||
            tmp.einheitPlural()   == einheitStr ) {
          // kdDebug() << "Thats it, returning " << tmp.id() << endl;
            return tmp.id();
        }
    }
    return -1;
}

UnitManager::~UnitManager( )
{
  delete m_units;
  delete m_dummy;
}

EinheitValueVector *UnitManager::m_units = 0;
Einheit *UnitManager::m_dummy = 0;
/* END */


