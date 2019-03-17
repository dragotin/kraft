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
#include <QHash>

// include files for KDE
#include <QDebug>

#include "materialcalcpart.h"
#include "materialkatalogview.h"
#include "katalogman.h"
#include "stockmaterial.h"
#include "unitmanager.h"
#include "matkatalog.h"

MaterialCalcPart::MaterialCalcPart()
  : CalcPart(),
    m_calcID( 0 )
{
}

MaterialCalcPart::MaterialCalcPart( long mCalcID, long matID, int percent, double amount  )
    : CalcPart( percent), m_calcID( mCalcID ),
      m_calcAmount(amount),
      m_mat(nullptr)
{
    getMatFromID(matID); // overwrites m_mat
    if( m_mat ) {
        setName(m_mat->getText());
    }
}

MaterialCalcPart::MaterialCalcPart(long matID, int percent, double amount)
    : CalcPart( percent), m_calcID(0), m_calcAmount(amount)
{
    getMatFromID(matID);
    if( m_mat ) {
        setName(m_mat->getText());
    }
}

MaterialCalcPart::~MaterialCalcPart( )
{
  // do not delete m_mat because it comes straight from stockmaterialman
}

void MaterialCalcPart::getMatFromID(long matID)
{
    MatKatalog *k = static_cast<MatKatalog*>(KatalogMan::self()->getKatalog( MaterialKatalogView::MaterialCatalogName ));
    if( !k ) {
        k = new MatKatalog( MaterialKatalogView::MaterialCatalogName );
        if( k ) {
            KatalogMan::self()->registerKatalog(k);
        }
    }
    if( k ) {
        m_mat = k->materialFromId(matID);
    }
}

QString MaterialCalcPart::getType() const
{
    return KALKPART_MATERIAL;
}

Geld MaterialCalcPart::basisKosten()
{
    Geld g;
    if( m_mat ) {
        g = m_mat->unitPrice() * m_calcAmount;
    }
    return g;
}

StockMaterial * MaterialCalcPart::getMaterial()
{
    return m_mat;
}

bool MaterialCalcPart::setCalcAmount( double newAmount )
{
    // Check for a change first
    if( qAbs(newAmount - m_calcAmount) > 0.00001 ) {
        m_calcAmount = newAmount;
        setDirty(true);
    }
    return isDirty();
}

/* END */

