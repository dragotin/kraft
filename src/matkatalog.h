/***************************************************************************
             matkatalog  - Materialkatalogklasse
                             -------------------
    begin                : 2004-19-10
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

#ifndef _MATKATALOG_H
#define _MATKATALOG_H

// include files
#include <qstring.h>

#include "stockmaterial.h"
#include "katalog.h"

/**
 *
 */

class MatKatalog : public Katalog
{
public:
  MatKatalog( const QString& name );
  MatKatalog();
  ~MatKatalog();

  int getEntriesPerChapter( const CatalogChapter& ) { return 0; } // FIXME

  int load();
  void reload( dbID );
  void deleteMaterial( int );

  KatalogType type() { return MaterialCatalog; }
  StockMaterialList getRecordList( const CatalogChapter& );
  void addNewMaterial( StockMaterial* );
private:
  StockMaterialList mAllMaterial;
};

#endif

/* END */

