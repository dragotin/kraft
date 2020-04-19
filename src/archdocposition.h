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
    QString taxMarkerHelper() const;

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
    Geld taxSum() const;
    Geld fullTaxSum() const;
    Geld reducedTaxSum() const;
    DocPositionBase::TaxType listTaxation() const;

    bool hasIndividualTaxes() const;

    void setTaxes(double fullTax, double reducedTax);

private:
    double _fullTax;
    double _reducedTax;
};

Q_DECLARE_METATYPE(ArchDocPosition)
Q_DECLARE_METATYPE(ArchDocPositionList)


// Read-only introspection of Person object.
GRANTLEE_BEGIN_LOOKUP(ArchDocPosition)
if ( property == "itemNumber" )
    return object.posNumber();
else if ( property == "text" )
    return object.text();
else if ( property == "kind" )
    return object.kind();
else if ( property == "unit" )
    return object.unit();
else if ( property == "unitPrice" ) {
    return object.unitPrice().toString();
} else if ( property == "nettoPrice" ) {
    return object.nettoPrice().toString();
} else if ( property == "amount" ) {
    QLocale *loc = DefaultProvider::self()->locale();
    return loc->toString(object.amount());
} else if ( property == "taxType" ) {
    if (object.taxType() == DocPositionBase::TaxType::TaxFull) {
        return QStringLiteral("fullTax");
    } else if (object.taxType() == DocPositionBase::TaxType::TaxReduced) {
        return QStringLiteral("reducedTax");
    } else if (object.taxType() == DocPositionBase::TaxType::TaxNone) {
        return QStringLiteral("noTax");
    }
    return QStringLiteral("Invalid");
} else if ( property == "itemType" ) {
    return object.kind();
} else if ( property == "taxMarker") {
    return object.taxMarkerHelper();
} else {
    return QStringLiteral("undefined");
}
GRANTLEE_END_LOOKUP



GRANTLEE_BEGIN_LOOKUP(ArchDocPositionList)
if (property == "sumPrice")
    return object.sumPrice().toString();
else if (property == "taxSum")
    return object.taxSum().toString();
else if (property == "fullTaxSum")
    return object.fullTaxSum().toString();
else if (property == "reducedTaxSum")
    return object.reducedTaxSum().toString();
else if (property == "reducedTaxSum")
    return object.reducedTaxSum().toString();
else if (property == "hasIndividualTaxes")
    return object.hasIndividualTaxes();
else
    return QStringLiteral("Undefined");
GRANTLEE_END_LOOKUP


#endif

