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

// include files for Qt
#include <QSqlQuery>

// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>

#include "stockmaterialman.h"

StockMaterialMan* StockMaterialMan::self()
{
  K_GLOBAL_STATIC(StockMaterialMan, mSelf);
  return mSelf;
}

StockMaterialMan::StockMaterialMan( )
{

}


StockMaterialMan::~StockMaterialMan( )
{

}

void StockMaterialMan::load( )
{
  QSqlQuery cur( "SELECT matID, chapterID, material, unitID, perPack, "
                 "priceIn, priceOut, modifyDate, enterDate FROM stockMaterial "
                 "ORDER BY material" );

  while( cur.next())
  {
    long matID = cur.value( 0 ).toLongLong();
    long matChap = cur.value( 1 ).toLongLong();
    QString material = cur.value( 2 ).toString();
    int unitID = cur.value( 3 ).toInt();
    double perPack = cur.value( 4 ).toDouble();
    double pIn = cur.value( 5 ).toDouble();
    double pOut = cur.value( 6 ).toDouble();
    QDate lastmod = cur.value( 7 ).toDate();
    QDate entered = cur.value( 8 ).toDate();
    Geld   gIn(pIn);
    Geld   gOut(pOut);

    StockMaterial *mat = new StockMaterial( matID, matChap, material, unitID,
                                            perPack, gIn, gOut );
    mat->setEnterDate( entered );
    mat->setLastModified( lastmod );

    mMaterials.append( mat );
  }
}

StockMaterial* StockMaterialMan::getMaterial( long id )
{
  StockMaterial *sm = findMaterial( id );
  return sm;
}

StockMaterial* StockMaterialMan::findMaterial( long id )
{
  if( mMaterials.isEmpty() )
    load();

  foreach( StockMaterial *sm, mMaterials ) {
    if( sm->getID() == id )
      return sm;
  }

  return 0;
}

/* END */
