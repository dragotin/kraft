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

#include <QList>
#include <QString>
#include <QLocale>
#include <QDebug>

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

Geld ArchDocPosition::nettoPrice() const
{
  return mOverallPrice;
}

Geld ArchDocPosition::fullTax( double fullTax ) const
{
  Geld tax;

  if ( mTaxType == DocPositionBase::TaxFull ) {
    tax = mOverallPrice * fullTax;
  }
  return tax / 100.0;
}

Geld ArchDocPosition::reducedTax( double reducedTax ) const
{
  Geld tax;

  if ( mTaxType == DocPositionBase::TaxReduced ) {
    tax = mOverallPrice * reducedTax;
  }
  return tax / 100.0;
}


Geld ArchDocPosition::tax( double fullTax, double reducedTax ) const
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

Geld ArchDocPositionList::taxSum( double fullTax, double reducedTax ) const
{
    Geld reduced;
    Geld gfullTax;

    const_iterator it;
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


