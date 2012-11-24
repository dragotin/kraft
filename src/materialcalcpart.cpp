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
#include <klocale.h>
#include <kdebug.h>

#include "materialcalcpart.h"
#include "stockmaterialman.h"
#include "stockmaterial.h"
#include "unitmanager.h"
#include <kglobal.h>

MaterialCalcPart::MaterialCalcPart()
  : CalcPart(),
    m_calcID( 0 )
{
}

MaterialCalcPart::MaterialCalcPart( long mCalcID, long matID, int percent, double amount  )
    : CalcPart( percent), m_calcID( mCalcID ), m_calcAmount(amount)
{
  m_mat = new StockMaterial();
  getMatFromID(matID);
  setName(m_mat->name());
}

MaterialCalcPart::MaterialCalcPart(long matID, int percent, double amount)
    : CalcPart( percent), m_calcID(0), m_calcAmount(amount)
{
  m_mat = new StockMaterial();
  if( m_mat ) {
      getMatFromID(matID);
      setName(m_mat->name());
  }
}

MaterialCalcPart::~MaterialCalcPart( )
{
  //We won't delete m_mat because it comes straight from stockmaterialman
}

void MaterialCalcPart::getMatFromID(long matID)
{
  delete m_mat;
  m_mat = StockMaterialMan::self()->getMaterial(matID);
}

QString MaterialCalcPart::getType() const
{   //This seems to be bad
    return KALKPART_MATERIAL;
}

Geld MaterialCalcPart::basisKosten()
{
  double d = m_calcAmount / m_mat->getAmountPerPack();
  return m_mat->salesPrice() * d;
}

StockMaterial * MaterialCalcPart::getMaterial()
{
  return m_mat;
}

bool MaterialCalcPart::setCalcAmount( double newAmount )
{
    m_calcAmount = newAmount;
    bool updated = true;
    setDirty(true);

    return updated;
}

/* END */

