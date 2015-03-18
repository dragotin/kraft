/***************************************************************************
          archdocposition.cpp  - a position in an archived document
                             -------------------
    begin                : Sep. 2006
    copyright            : (C) 2006 by Klaas Freitag
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
#include <QList>
#include <QString>

// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>

// application specific includes
#include "einheit.h"
#include "geld.h"
#include "archdocposition.h"

/**
@author Klaas Freitag
*/

ArchDocPosition::ArchDocPosition()
  : mAmount( 0 )
{

}

Geld ArchDocPosition::nettoPrice()
{
  return mOverallPrice;
}

Geld ArchDocPosition::fullTax( double fullTax )
{
  Geld tax;

  if ( mTaxType == DocPositionBase::TaxFull ) {
    tax = mOverallPrice * fullTax;
  }
  return tax / 100.0;
}

Geld ArchDocPosition::reducedTax( double reducedTax )
{
  Geld tax;

  if ( mTaxType == DocPositionBase::TaxReduced ) {
    tax = mOverallPrice * reducedTax;
  }
  return tax / 100.0;
}


Geld ArchDocPosition::tax( double fullTax, double reducedTax )
{
  Geld tax;

  if ( mTaxType == DocPositionBase::TaxFull ) {
    tax = mOverallPrice * fullTax;
  } else if ( mTaxType == DocPositionBase::TaxReduced ) {
    tax = mOverallPrice * reducedTax;
  }
  return tax / 100.0;
}

ArchDocPositionList::ArchDocPositionList()
    : QList<ArchDocPosition>()
{

}

Geld ArchDocPositionList::sumPrice()
{
    Geld g;

    iterator it;
    for ( it = begin(); it != end(); ++it ) {
      g += ( *it ).nettoPrice();
    }

    return g;
}

Geld ArchDocPositionList::taxSum( double fullTax, double reducedTax )
{
    Geld reduced;
    Geld gfullTax;

    iterator it;
    for ( it = begin(); it != end(); ++it ) {
        if( (*it).taxType() == DocPositionBase::TaxFull) {
            gfullTax += (*it).nettoPrice();
        } else if( (*it).taxType() == DocPositionBase::TaxReduced ) {
            reduced += (*it).nettoPrice();
        }
    }
    const Geld fullTaxSum( gfullTax.percent(fullTax).toLong() + reduced.percent(reducedTax).toLong() );
    return fullTaxSum;
}

Geld ArchDocPositionList::fullTaxSum( double fullTax )
{
    const Geld g = taxSum( fullTax, 0.0);

    return g;
}

Geld ArchDocPositionList::reducedTaxSum( double reducedTax )
{
    const Geld g = taxSum( 0.0, reducedTax);

    return g;
}


