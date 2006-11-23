/***************************************************************************
             stockmaterialman  -
                             -------------------
    begin                : 2004-08-05
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

#ifndef _STOCKMATERIALMAN_H
#define _STOCKMATERIALMAN_H


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// include files
#include "stockmaterial.h"

/**
 *
 */


class StockMaterialMan
{

public:
  StockMaterialMan();
  ~StockMaterialMan();

  static StockMaterial* getMaterial( long );


private:
  static StockMaterial* findMaterial( long );

  static void load( long = 0);
  static StockMaterialList *m_materials;
};

#endif

/* END */

