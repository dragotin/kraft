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
#include "archdoc.h"

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

QString ArchDocPosition::taxMarkerHelper() const
{
    QString re;

    if ( mTaxType == DocPositionBase::TaxReduced ) {
        re = ArchDoc::taxMarkerReduced();
    } else if ( mTaxType == DocPositionBase::TaxNone) {
        re = ArchDoc::taxMarkerNoTax();
    } else if ( mTaxType == DocPositionBase::TaxFull) {
        re = ArchDoc::taxMarkerFull();
    }
    return re;
}

// ==================================================================

ArchDocPositionList::ArchDocPositionList()
    : QList<ArchDocPosition>()
{

}

Geld ArchDocPositionList::sumPrice() const
{
    Geld g;

    const_iterator it;
    for ( it = begin(); it != end(); ++it ) {
      g += ( *it ).nettoPrice();
    }

    return g;
}

Geld ArchDocPositionList::taxSum() const
{
    Geld allTaxSum = fullTaxSum();
    allTaxSum += reducedTaxSum();
    return allTaxSum;
}

Geld ArchDocPositionList::fullTaxSum() const
{
    Geld g;
    const_iterator it;
    for ( it = begin(); it != end(); ++it ) {
        if( (*it).taxType() == DocPositionBase::TaxFull) {
            g += (*it).nettoPrice();
        }
    }
    const Geld ftSum(g.percent(_fullTax).toLong());
    return ftSum;
}

Geld ArchDocPositionList::reducedTaxSum() const
{
    Geld g;
    const_iterator it;
    for ( it = begin(); it != end(); ++it ) {
        if( (*it).taxType() == DocPositionBase::TaxReduced) {
            g += (*it).nettoPrice();
        }
    }
    const Geld rtSum(g.percent(_reducedTax).toLong());
    return rtSum;
}

DocPositionBase::TaxType ArchDocPositionList::listTaxation() const
{
    int fullTax = 0;
    int noTax = 0;
    int redTax = 0;

    DocPositionBase::TaxType ret = DocPositionBase::TaxType::TaxNone;

    const_iterator it;
    for ( it = begin(); it != end(); ++it ) {
        if( (*it).taxType() == DocPositionBase::TaxFull) {
            fullTax++;
        } else if( (*it).taxType() == DocPositionBase::TaxReduced ) {
            redTax++;
        } else if( (*it).taxType() == DocPositionBase::TaxNone ) {
            noTax++;
        }
    }

    int cnt = count();
    if (noTax == cnt) {
        ret = DocPositionBase::TaxType::TaxNone;
    } else if (redTax == cnt) {
        ret = DocPositionBase::TaxType::TaxReduced;
    } else if (fullTax == cnt) {
        ret = DocPositionBase::TaxType::TaxFull;
    } else
        ret = DocPositionBase::TaxType::TaxIndividual;

    return ret;
}

bool ArchDocPositionList::hasIndividualTaxes() const
{
    qDebug() << "Has INDIVIDUAL taxes.";
    return (listTaxation() == DocPositionBase::TaxType::TaxIndividual);
}

void ArchDocPositionList::setTaxes(double fullTax, double reducedTax)
{
    _fullTax = fullTax;
    _reducedTax = reducedTax;
}
