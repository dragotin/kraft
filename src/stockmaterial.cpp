/***************************************************************************
             material  -
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

// include files for Qt
#include <qsqlcursor.h>

// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <kabc/addressee.h>

#include "stockmaterial.h"
#include "unitmanager.h"

StockMaterial::StockMaterial( )
{

}

StockMaterial::StockMaterial( int dbid, int matChap, QString mat, int unitID,
                              double perPack, Geld pIn, Geld pOut ):
    m_name(mat),
    m_chapter(matChap),
    m_amount(perPack),
    m_dbid(dbid),
    m_ePrice(pIn),
    m_vPrice(pOut)
{
    m_unit = UnitManager::getUnit(unitID);
}

StockMaterial::~StockMaterial( )
{

}

QString StockMaterial::getName() const
{
    return m_name;
}

QString StockMaterial::getDescription() const
{
    return m_descr;
}

double StockMaterial::getAmountPerPack()
{
    return m_amount;
}

void StockMaterial::setAmountPerPack( double am )
{
    m_amount = am;
}

Einheit StockMaterial::getUnit( )
{
    return m_unit;
}

void StockMaterial::setUnit( const Einheit& e )
{
    m_unit = e;
}

int StockMaterial::getID()
{
    return m_dbid;
}

void StockMaterial::setID( int id )
{
    m_dbid = id;
}

KABC::Addressee StockMaterial::getSupplier()
{
    KABC::Addressee a;
    return a;
}

void StockMaterial::setSupplier( KABC::Addressee *a )
{
    if( a )
        m_delivererUID = a->uid();
}

Geld StockMaterial::getEPreis()
{
    return m_ePrice;
}

Geld StockMaterial::getVPreis()
{
    return m_vPrice;
}

void StockMaterial::setEPreis( Geld g )
{
    m_ePrice = g;
}

void StockMaterial::setVPreis( Geld g )
{
    m_vPrice = g;
}

/* END */

