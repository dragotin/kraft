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
#include <qsqlcursor.h>

// include files for KDE
#include <klocale.h>
#include <kdebug.h>

#include "stockmaterialman.h"


StockMaterialMan::StockMaterialMan( )
{

}


StockMaterialMan::~StockMaterialMan( )
{

}

void StockMaterialMan::load()
{
    QSqlCursor cur("stockMat");

    // Create an index that sorts from high values for einheitID down.
    // that makes at least on resize of the vector.
    QSqlIndex indx = cur.index( "material" );

    cur.select(indx);
    while( cur.next())
    {
        long matID = cur.value("matID").toLongLong();
        long matChap = cur.value("matChapter").toLongLong();
        QString material = cur.value("material").toString();
        int unitID = cur.value("unitID").toInt();
        double perPack = cur.value("perPack").toDouble();
        double pIn = cur.value("priceIn").toDouble();
        double pOut = cur.value("priceOut").toDouble();
        Geld   gIn(pIn);
        Geld   gOut(pOut);

        StockMaterial *mat = new StockMaterial( matID, matChap, material, unitID,
                                                perPack, gIn, gOut );

        m_materials->append( mat );
    }

}

StockMaterial* StockMaterialMan::getMaterial( long id )
{
    StockMaterial *sm = 0;

    if( ! m_materials )
        m_materials = new StockMaterialList();

    if( m_materials->isEmpty() )
        load();

    // TODO: Find the required item
    StockMaterialListIterator it(*m_materials);

    // Search for the required ID.
    // FIXME: Could be speed up using a Dict.
    StockMaterial *mat;
    while ( sm == 0 && (mat = it.current()) != 0 ) {
        ++it;

        if( mat->getID() == id )
            sm = mat;
    }
    return sm;
}

StockMaterialList *StockMaterialMan::m_materials = 0;


/* END */
