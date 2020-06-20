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

#ifndef _UNITMANAGER_H
#define _UNITMANAGER_H

#include "einheit.h"

/**
 *
 */

// FIXME: How to identify the unit for piece?
#define PIECE_UNIT_ID 6

class UnitManager
{
  public:
    UnitManager();

    virtual ~UnitManager();
    static UnitManager* self();

    Einheit getUnit( int id );
    Einheit getPauschUnit();
    QStringList allUnits();
    int getUnitIDSingular( const QString& einheit );

    // Workaround: since the unit table does not have an auto update id coloum,
    // this function calculates the next free unit id to save a new one.
    int nextFreeId();

  private:
    Einheit::List mUnits;

    void load();


};

#endif

/* END */

