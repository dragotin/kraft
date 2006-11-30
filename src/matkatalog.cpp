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
#include <qsqlcursor.h>

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

int MatKatalog::load()
{
  Katalog::load();
  int cnt = 0;

  QSqlCursor cur( "stockMaterial" ); // Specify the table/view name
  cur.setMode( QSqlCursor::ReadOnly );
  cur.select(); // We'll retrieve every record
  while ( cur.next() ) {
    cnt++;
    int id = cur.value( "matID" ).toInt();
    int chapterID = cur.value( "chapterID" ).toInt();
    QString material = QString::fromUtf8( cur.value( "material" ).toCString() );
    int unitID = cur.value( "unitID" ).toInt();
    double pPack = cur.value( "perPack" ).toDouble();
    double priceIn = cur.value( "priceIn" ).toDouble();
    double priceOut = cur.value( "priceOut" ).toDouble();

    mAllMaterial.append( new StockMaterial( id, chapterID, material, unitID,
                                            pPack, Geld( priceIn ), Geld( priceOut ) ) );
  }

  return cnt;
}

StockMaterialList MatKatalog::getRecordList( const QString& chapter )
{
  StockMaterialList list;
  StockMaterial *mat;

  int chapID = chapterID( chapter );

  for ( mat = mAllMaterial.first(); mat; mat = mAllMaterial.next() ) {
    if ( mat->chapter() == chapID ) {
      list.append( mat );
    }
  }
  return list;

}



MatKatalog::~MatKatalog( )
{

}

/* END */

