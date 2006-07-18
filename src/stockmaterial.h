/***************************************************************************
             material  - Material for calculations
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

#ifndef _MATERIAL_H
#define _MATERIAL_H


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// include files
#include <qptrlist.h>

#include <kabc/addressee.h>
#include "kraftglobals.h"
#include "einheit.h"
/**
 *
 */
class Einheit;


class StockMaterial
{
public:
    StockMaterial();
    StockMaterial( int dbid, int matChap, QString mat, int unitID,
                   double perPack, Geld pIn, Geld pOut );
    ~StockMaterial();

    QString getName() const;
    QString getDescription() const;

    double getAmountPerPack();
    void setAmountPerPack( double am );

    Einheit getUnit();
    void setUnit(const Einheit& );

    int getID();
    void setID( int );

    KABC::Addressee getSupplier();
    void setSupplier( KABC::Addressee *supp );

    Geld getEPreis();
    Geld getVPreis();

    void setEPreis( Geld );
    void setVPreis( Geld );

private:
    QString m_name;
    QString m_descr;
    int     m_chapter;

    // per package:
    double  m_amount;
    Einheit m_unit;
    int     m_dbid;

    // FIXME: introduce supplier list
    QString m_delivererUID;
    Geld    m_ePrice;  // price for bying
    Geld    m_vPrice;  // price for selling
};


class StockMaterialList : public QPtrList<StockMaterial>
{
public:
    StockMaterialList() : QPtrList<StockMaterial>() { }
};

typedef QPtrListIterator<StockMaterial> StockMaterialListIterator;

#endif

/* END */

