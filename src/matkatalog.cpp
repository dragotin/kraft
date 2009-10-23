/***************************************************************************
                      matkatalog  - the material catalog
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

// include files for Qt
#include <qsql.h>
#include <q3sqlcursor.h>

// include files for KDE
#include <klocale.h>
#include <kdebug.h>

#include "matkatalog.h"
#include "kraftdb.h"

MatKatalog::MatKatalog( const QString& name)
    : Katalog(name)
{

}

MatKatalog::MatKatalog()
    : Katalog( QString( "Material" ))
{

}

void MatKatalog::reload( dbID )
{
  mAllMaterial.clear();
  load();
}

int MatKatalog::load()
{
  Katalog::load();
  int cnt = 0;

  Q3SqlCursor cur( "stockMaterial" ); // Specify the table/view name
  cur.setMode( Q3SqlCursor::ReadOnly );
  cur.select(); // We'll retrieve every record
  while ( cur.next() ) {
    cnt++;
    int id = cur.value( "matID" ).toInt();
    int chapterID = cur.value( "chapterID" ).toInt();
    const QString material = cur.value( "material" ).toString();
    int unitID = cur.value( "unitID" ).toInt();
    double pPack = cur.value( "perPack" ).toDouble();
    double priceIn = cur.value( "priceIn" ).toDouble();
    double priceOut = cur.value( "priceOut" ).toDouble();
    QDate lastMod = cur.value( "modifyDate" ).toDate();
    QDate entered = cur.value( "enterDate" ).toDate();

    StockMaterial *mat = new StockMaterial( id, chapterID, material, unitID,
                                            pPack, Geld( priceIn ), Geld( priceOut ) );
    mat->setEnterDate( entered );
    mat->setLastModified( lastMod );
    mAllMaterial.append( mat );

  }

  return cnt;
}

StockMaterialList MatKatalog::getRecordList( const QString& chapter )
{
  StockMaterialList list;

  int chapID = chapterID( chapter );
  StockMaterialListIterator it( mAllMaterial );
  
  while( it.hasNext() ) {
    StockMaterial *mat = it.next();

    if ( mat->chapter() == chapID ) {
      list.append( mat );
    }
  }
  return list;

}

void MatKatalog::addNewMaterial( StockMaterial *mat )
{
  mAllMaterial.append( mat );
}


MatKatalog::~MatKatalog( )
{

}

/* END */

