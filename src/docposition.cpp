/***************************************************************************
                 docposition.cpp  - a position in a document
                             -------------------
    begin                : Fri Jan 20 2006
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
#include <QString>
#include <qdom.h>
#include <QDebug>
#include <QLocale>

// application specific includes
#include "einheit.h"
#include "geld.h"
#include "docposition.h"
#include "ui_positionwidget.h"
#include "positionviewwidget.h"
#include "defaultprovider.h"
#include "tagman.h"

/**
@author Klaas Freitag
*/

DocPositionBase::DocPositionBase() : KraftObj(),
    m_dbId( -1 ),
    mToDelete( false ),
    mTaxType( TaxFull ),
    mType( Position )

{

}

DocPositionBase::DocPositionBase( const PositionType& t )
    : KraftObj(),
      m_dbId( -1 ),
      mToDelete( false ),
      mTaxType( TaxFull ),
      mType( t )
{

}

DocPositionBase::TaxType DocPositionBase::taxType()
{
    return mTaxType;
}

void DocPositionBase::setTaxType( TaxType tt )
{
    mTaxType = tt;
}

void DocPositionBase::setTaxType( int tt )
{
    mTaxType = (TaxType) tt;
}

void DocPositionBase::setTaxType(const QString& taxStr)
{
    mTaxType = TaxType::TaxInvalid;
    if (taxStr == QStringLiteral("Full") )
        mTaxType = TaxFull;
    else if (taxStr == QStringLiteral("Reduced"))
        mTaxType = TaxReduced;
    else if (taxStr == QStringLiteral("None"))
        mTaxType = TaxNone;
}

int DocPositionBase::taxTypeNumeric()
{
    if ( mTaxType == TaxNone )
        return 1;
    else if ( mTaxType == TaxReduced )
        return 2;
    else if ( mTaxType == TaxFull )
        return 3;

    // qDebug () << "ERR: Vat-type ambigous!";
    return 0; // Invalid
}

QString DocPositionBase::typeStr()
{
    // { Position, ExtraDiscount, Text, Demand, Alternative }
    return typeToString(type());
}

DocPositionBase::PositionType DocPositionBase::typeStrToType(const QString& t)
{
    if (t == typeToString(PositionType::ExtraDiscount))
        return PositionType::ExtraDiscount;
    else if (t == typeToString(PositionType::Alternative))
        return PositionType::Alternative;
    else if (t == typeToString(PositionType::Demand))
        return PositionType::Demand;
    else if (t == typeToString(PositionType::Text))
        return PositionType::Text;

    return PositionType::Position;

}

QString DocPositionBase::typeToString(DocPositionBase::PositionType t)
{
    switch (t) {
    case DocPositionBase::PositionType::ExtraDiscount:
        return QStringLiteral("ExtraDiscount");
        break;
    case DocPositionBase::PositionType::Alternative:
        return QStringLiteral("Alternative");
        break;
    case DocPositionBase::PositionType::Demand:
        return QStringLiteral("Demand");
        break;
    case DocPositionBase::PositionType::Position:
        return QStringLiteral("Normal");
        break;
    case DocPositionBase::PositionType::Text:
        return QStringLiteral("Text");
    default:
        return QString();
        break;
    }
}

// ##############################################################

const QString DocPositionBase::Kind{"kind"};
const QString DocPositionBase::Discount{"discount"};
const QString DocPositionBase::Tags{"tags"};
const QString DocPositionBase::ExtraDiscountTagRequired{"discountTagRequired"};

Geld DocPositionBase::overallPrice()
{
    Geld g;

    // only calculate the sum for normal items and discount items
    if (type() == DocPositionBase::PositionType::Position ||
            type() == DocPositionBase::PositionType::ExtraDiscount) {
        g = unitPrice() * amount();
    } else {
        // qDebug() << "Skipping price in overallPrice because of item type";
    }

    return g;
}

// ##############################################################

DocPositionList::DocPositionList()
    : QList<DocPositionBase*>()
{
    // setAutoDelete( true );
}

Geld DocPositionList::bruttoPrice(double fullTax, double reducedTax )
{
    Geld g = nettoPrice();
    g += taxSum( fullTax, reducedTax );
    return g;
}

Geld DocPositionList::nettoPrice()
{
    Geld g;

    DocPositionListIterator it( *this );
    while( it.hasNext() ) {
        DocPositionBase *dp = it.next();
        if (!dp->toDelete())
            g += dp->overallPrice();
    }
    return g;
}

Geld DocPositionList::fullTaxSum( double fullTax )
{
    Geld sum;

    if ( fullTax < 0 ) {
        qCritical() << "Full Tax is not loaded!";
    }
    DocPositionListIterator it( *this );
    while( it.hasNext() ) {
        DocPositionBase *dp = static_cast<DocPositionBase*>( it.next() );

        auto tt = dp->taxType();
        if( !dp->toDelete() && tt == DocPositionBase::TaxFull ) {
            sum += dp->overallPrice();
        }
    }

    Geld tax;
    if( sum.toLong() > 0 ) {
        tax = sum.percent(fullTax);
    }

    return tax;
}

Geld DocPositionList::reducedTaxSum( double reducedTax )
{
    Geld sum;

    if ( reducedTax < 0 ) {
        qCritical() << "Reduced Tax is not loaded!";
    }
    DocPositionListIterator it( *this );
    while( it.hasNext() ) {
        DocPositionBase *dp = it.next();

        if( !dp->toDelete() && dp->taxType() == DocPositionBase::TaxReduced ) {
            sum += dp->overallPrice();
        }
    }

    Geld tax;
    if(sum.toLong() > 0 ) {
        tax = sum.percent(reducedTax);
    }

    return tax;
}

Geld DocPositionList::taxSum( double fullTax, double redTax )
{
    Geld sum;

    Geld fulltax = fullTaxSum(fullTax);
    Geld reducedtax = reducedTaxSum(redTax);

    sum += fulltax;
    sum += reducedtax;

    return sum;
}

QString DocPositionList::posNumber( DocPositionBase* pos )
{
    return QString::number( 1+indexOf( pos ) );
}


int DocPositionList::compareItems ( DocPositionBase *dp1, DocPositionBase *dp2 )
{
    //DocPositionBase *dpb1 = static_cast<DocPositionBase*>( item1 );
    //DocPositionBase *dpb2 = static_cast<DocPositionBase*>( item2 );

    int sortkey1 = dp1->positionNumber();
    int sortkey2 = dp2->positionNumber();

    int res = 0;
    if( sortkey1 > sortkey2 ) res = 1;
    if( sortkey2 < sortkey1 ) res = -1;

    // qDebug()<< "In sort: comparing " << p1 << " with " << p2 << " = " << res;
    return res;
}

DocPositionBase::TaxType DocPositionList::listTaxation() const
{
    int fullTax = 0;
    int noTax = 0;
    int redTax = 0;

    DocPositionBase::TaxType ret = DocPositionBase::TaxType::TaxNone;

    const_iterator it;
    for ( it = begin(); it != end(); ++it ) {
        if( (*it)->taxType() == DocPositionBase::TaxFull) {
            fullTax++;
        } else if( (*it)->taxType() == DocPositionBase::TaxReduced ) {
            redTax++;
        } else if( (*it)->taxType() == DocPositionBase::TaxNone ) {
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

bool DocPositionList::hasIndividualTaxes() const
{
    bool re = listTaxation() == DocPositionBase::TaxType::TaxIndividual;
    return re;
}


QDomElement DocPositionList::xmlTextElement( QDomDocument& doc, const QString& name, const QString& value )
{
    QDomElement elem = doc.createElement( name );
    QDomText t = doc.createTextNode( value );
    elem.appendChild( t );
    return elem;
}

DocPositionBase *DocPositionList::positionFromId( int id )
{
    DocPositionBase *dp{nullptr};

    DocPositionListIterator it(*this);
    while( it.hasNext() ) {
        dp = static_cast<DocPositionBase*>( it.next() );

        if( dp->dbId() == id ) {
            break;
        }
    }
    return dp;
}


