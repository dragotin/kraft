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

#include <grantlee/metatype.h>

// include files for KDE

// application specific includes
#include "einheit.h"
#include "geld.h"
#include "dbids.h"
#include "docposition.h"
#include "defaultprovider.h"

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

    QString text() const { return mText; }

    QString unit() const { return mUnit; }

    Geld unitPrice() const { return mUnitPrice; }
    Geld nettoPrice() const;

    double amount() const { return mAmount; }
    DocPositionBase::TaxType taxType() const { return mTaxType; }
    Geld   tax( double fullTax, double reducedTax ) const;
    Geld   fullTax( double fullTax ) const;
    Geld   reducedTax( double reducedTax ) const;

    QString kind() const { return mKind; }
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
    Geld sumPrice() const;
    Geld taxSum( double, double ) const;
    Geld fullTaxSum( double ) const;
    Geld reducedTaxSum( double ) const;
};


Q_DECLARE_METATYPE(ArchDocPosition)
Q_DECLARE_METATYPE(ArchDocPositionList)


// Read-only introspection of Person object.
GRANTLEE_BEGIN_LOOKUP(ArchDocPosition)
if ( property == "itemNumber" )
    return object.posNumber();
else if ( property == "text" )
    return object.text();
else if ( property == "unit" )
    return object.unit();
else if ( property == "unitPrice" ) {
    return object.unitPrice().toString();
}
else if ( property == "nettoPrice" )
    return object.nettoPrice().toString();
else if ( property == "amount" ) {
    QLocale *loc = DefaultProvider::self()->locale();
    return loc->toString(object.amount());
} else if ( property == "taxTypeStr" )
    return "taxType";
else if ( property == "itemType" )
    return object.kind();
else
    return QStringLiteral("undefined");
GRANTLEE_END_LOOKUP



GRANTLEE_BEGIN_LOOKUP(ArchDocPositionList)
if (property == "sumPrice")
    return object.sumPrice().toString();
else if (property == "taxSum")
    return object.taxSum(7.0, 19.0).toString();
else if (property == "fullTaxSum")
    return object.fullTaxSum(19.0).toString();
else if (property == "reducedTaxSum")
    return object.reducedTaxSum(19.0).toString();
else if (property == "reducedTaxSum")
    return object.reducedTaxSum(19.0).toString();
else
    return QStringLiteral("Undefined");
GRANTLEE_END_LOOKUP


#endif

