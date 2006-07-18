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

// include files
#include <qvaluevector.h>

#include "einheit.h"
/**
 *
 */

class UnitManager
{

public:
    UnitManager();
    ~UnitManager();

    static Einheit& getUnit( int id );
    static QStringList allUnits();
    static int getUnitIDSingular( const QString& einheit );
private:
    static void load();
    static EinheitValueVector *m_units;

    static Einheit *m_dummy;
};

#endif

/* END */

