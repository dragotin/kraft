/***************************************************************************
           archdocposition.h  - a position in an archived document
                             -------------------
    begin                : Sep 2006
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
#ifndef ARCHDOCPOSITION_H
#define ARCHDOCPOSITION_H

// include files for Qt
#include <QString>
#include <QList>

// include files for KDE

// application specific includes
#include "einheit.h"
#include "geld.h"
#include "dbids.h"
#include "docposition.h"

class ArchDoc;
/**
@author Klaas Freitag
*/

class ArchDocPosition
{
    friend class ArchDoc;
  public:
    ArchDocPosition();
    ~ArchDocPosition(){};

    QString posNumber() const { return mPosNo; }

    QString text() const { return mText; } ;

    QString unit() const { return mUnit; }

    Geld unitPrice() const { return mUnitPrice; }
    Geld nettoPrice();

    double amount() { return mAmount; }
    DocPositionBase::TaxType taxType() { return mTaxType; }
    Geld   tax( double fullTax, double reducedTax );
    Geld   fullTax( double fullTax );
    Geld   reducedTax( double reducedTax );

    QString kind() { return mKind; }
  private:
    QString mText;
    QString mPosNo;
    QString mUnit;
    QString mKind;
    Geld    mUnitPrice;
    Geld    mOverallPrice;
    double  mAmount;
    DocPositionBase::TaxType mTaxType;
    // No calculation yet
};

class ArchDocPositionList : public QList<ArchDocPosition>
{
  public:
    ArchDocPositionList();
    Geld sumPrice();
    Geld taxSum( double, double );
    Geld fullTaxSum( double );
    Geld reducedTaxSum( double );
};


#endif

