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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "einheit.h"

/**
 *
 */

// FIXME: How to identify the unit for piece?
#define PIECE_UNIT_ID 6

class UnitManager
{
  public:

    virtual ~UnitManager();
    static UnitManager* self();

    Einheit getUnit( int id );
    QStringList allUnits();
    int getUnitIDSingular( const QString& einheit );

  private:
    UnitManager();
    Einheit::List mUnits;
    static UnitManager* mSelf;

    void load();
};

#endif

/* END */

