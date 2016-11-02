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

// include files
#include "stockmaterial.h"

/**
 *
 */


class StockMaterialMan
{

public:

  virtual ~StockMaterialMan();

  static StockMaterialMan* self();
  StockMaterial* getMaterial( long );
  StockMaterialMan();

private:
  StockMaterial* findMaterial( long );


  void load();
  StockMaterialList mMaterials;
};

#endif

/* END */

