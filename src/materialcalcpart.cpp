/***************************************************************************
             materialcalcpart  -
                             -------------------
    begin                : 2004-09-05
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

// include files for KDE
#include <klocale.h>
#include <kdebug.h>

#include "materialcalcpart.h"
#include "stockmaterialman.h"
#include "stockmaterial.h"
#include "unitmanager.h"
#include <kglobal.h>


MaterialCalcPart::MaterialCalcPart(long mCalcID, const QString& name, int percent )
    : CalcPart( name, percent ),
      m_calcID( mCalcID )
{

}

MaterialCalcPart::MaterialCalcPart(const QString& name, int percent)
    : CalcPart( name, percent ),
      m_calcID(0)
{

}

MaterialCalcPart::~MaterialCalcPart( )
{

}

void MaterialCalcPart::addMaterial( double amount, long matID )
{
    addMaterial( amount, StockMaterialMan::getMaterial(matID));
}

void MaterialCalcPart::addMaterial( double amount, StockMaterial* mat)
{
    if( mat == 0 ) return;

    m_amounts.insert( mat, new QVariant(amount) );
    setDirty(true);
}

void MaterialCalcPart::removeMaterial( StockMaterial *mat )
{
    m_amounts.remove( mat );
    setDirty(true);
}

QString MaterialCalcPart::getType() const
{
    return KALKPART_MATERIAL;
}

Geld MaterialCalcPart::basisKosten()
{
    Geld gg;

    QPtrDictIterator<QVariant> it( m_amounts );
    for( ; it.current(); ++it )
    {
        StockMaterial *mat = (StockMaterial*) it.currentKey();

        gg += getPriceForMaterial(mat);

            }
    return gg;
}

/*
 * return a list of all materials calculated in this calcpart
 */
StockMaterialList MaterialCalcPart::getCalcMaterialList()
{
    StockMaterialList reList;

    QPtrDictIterator<QVariant> it( m_amounts );
    for( ; it.current(); ++it )
    {
        reList.append( (StockMaterial*) it.currentKey());
    }

    return reList;
}

/*
 * checkt ob es das Material in der Liste der Kalkulierten
 * enthalten ist.
 */

bool MaterialCalcPart::containsMaterial( long materialID )
{
    bool isThere = false;

    // StockMaterialList list = getCalcMaterialList();
    StockMaterialList li = getCalcMaterialList();
    kdDebug() << "Anzahl listeintraege: " << li.count() << endl;
    StockMaterialListIterator it( li );

    StockMaterial* mat = it.current();
    kdDebug() << "Materialpointer: " << mat << endl;

    while ( (!isThere) && (mat != 0) )
    {
        ++it;
        long myMatID = mat->getID();
        kdDebug() << "comparing "<<myMatID<< " to " << materialID << endl;
        if( myMatID == materialID )
            isThere = true;
        mat = it.current();
    }
    return isThere;
}


/*
 * Cost of the calculated amount of the specific material in this
 * calculation part. Note that one template may have more than one
 * material calculation part!
 * This is the costs of the material, i.e. it is calculated with
 * the price to be payed to buy the material (EPreis!)
 */
Geld MaterialCalcPart::getCostsForMaterial( StockMaterial *mat)
{
    Geld g;

    if( mat && mat->getAmountPerPack() > 0 )
    {
        double d = getCalcAmount( mat ) / mat->getAmountPerPack();
        g = mat->purchPrice() * d;

        // kdDebug() << "Cost for material " << mat->getName() << ": " <<
        // g.toString() << endl;
    }
    return g;
}

/*
 * Price of the calculated amount of the specific material in this
 * calculation part. Note that one template may have more than one
 * material calculation part!
 * This is the price of the material, i.e. it is calculated with
 * the price the customer has to pay for this material (VPreis!)
 */
Geld MaterialCalcPart::getPriceForMaterial( StockMaterial *mat)
{
    Geld g;

    if( mat && mat->getAmountPerPack() > 0 )
    {
        // double d = vAmount.toDouble() / mat->getAmountPerPack();
        double d = getCalcAmount( mat ) / mat->getAmountPerPack();
        g = mat->salesPrice() * d;

        // kdDebug() << "Cost for material " << mat->getName() << ": " <<
        //    g.toString() << endl;
    }
    return g;
}

/**
 * returns the amount of calculated material for the given type.
 * Note: to distinguish between a not existing material and a
 * material with amount zero, this method returns zero in case
 * the material is there but with amount 0 and -1 if the material
 * is not in the calcpartlist at all.
 */
double MaterialCalcPart::getCalcAmount( StockMaterial* mat )
{
    double am = -1.0;

    if( mat && m_amounts[mat])
    {
        QVariant v = *(m_amounts[mat]);
        am = v.toDouble();
    }
    return am;
}


bool MaterialCalcPart::setCalcAmount( StockMaterial* mat, double newAmount )
{
    bool updated = false;
    if( mat && m_amounts[mat] )
    {
        QVariant& v = *(m_amounts[mat]);
        double prevAmount = v.toDouble();

        if( prevAmount != newAmount )
        {
            v = newAmount;
            updated = true;
            setDirty(true);
        }
    }
    return updated;
}

/* END */

