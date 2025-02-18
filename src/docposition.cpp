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

DocPosition::DocPosition() : KraftObj(),
    m_dbId( -1 ),
    mToDelete( false ),
    mTaxType( Tax::Full ),
    mType( Type::Position ),
    m_amount{1.0}

{

}

DocPosition::DocPosition(const Type &t )
    : KraftObj(),
      m_dbId( -1 ),
      mToDelete( false ),
      mTaxType( Tax::Full ),
      mType( t ),
      m_amount{1.0}
{

}

DocPosition::Tax DocPosition::taxType()
{
    return mTaxType;
}

void DocPosition::setTaxType( Tax tt )
{
    mTaxType = tt;
}

void DocPosition::setTaxType( int tt )
{
    mTaxType = (Tax) tt;
}

void DocPosition::setTaxType(const QString& taxStr)
{
    mTaxType = Tax::Invalid;
    if (taxStr == QStringLiteral("Full") )
        mTaxType = Tax::Full;
    else if (taxStr == QStringLiteral("Reduced"))
        mTaxType = Tax::Reduced;
    else if (taxStr == QStringLiteral("None"))
        mTaxType = Tax::None;
}

int DocPosition::taxTypeNumeric()
{
    if ( mTaxType == Tax::None )
        return 1;
    else if ( mTaxType == Tax::Reduced )
        return 2;
    else if ( mTaxType == Tax::Full )
        return 3;

    // qDebug () << "ERR: Vat-type ambigous!";
    return 0; // Invalid
}

QString DocPosition::typeStr()
{
    // { Position, ExtraDiscount, Text, Demand, Alternative }
    return typeToString(type());
}

DocPosition::Type DocPosition::typeStrToType(const QString& t)
{
    if (t == typeToString(Type::ExtraDiscount))
        return Type::ExtraDiscount;
    else if (t == typeToString(Type::Alternative))
        return Type::Alternative;
    else if (t == typeToString(Type::Demand))
        return Type::Demand;
    else if (t == typeToString(Type::Text))
        return Type::Text;

    return Type::Position;

}

QString DocPosition::typeToString(DocPosition::Type t)
{
    switch (t) {
    case DocPosition::Type::ExtraDiscount:
        return QStringLiteral("ExtraDiscount");
        break;
    case DocPosition::Type::Alternative:
        return QStringLiteral("Alternative");
        break;
    case DocPosition::Type::Demand:
        return QStringLiteral("Demand");
        break;
    case DocPosition::Type::Position:
        return QStringLiteral("Normal");
        break;
    case DocPosition::Type::Text:
        return QStringLiteral("Text");
    default:
        return QString();
        break;
    }
}

// ##############################################################

const QString DocPosition::Kind{"kind"};
const QString DocPosition::Discount{"discount"};
const QString DocPosition::Tags{"tags"};
const QString DocPosition::ExtraDiscountTagRequired{"discountTagRequired"};

Geld DocPosition::overallPrice()
{
    Geld g;

    // only calculate the sum for normal items and discount items
    if (type() == Type::Position ||
            type() == Type::ExtraDiscount) {
        g = unitPrice() * amount();
    } else {
        // qDebug() << "Skipping price in overallPrice because of item type";
    }

    return g;
}

// ##############################################################

DocPositionList::DocPositionList()
    : QList<DocPosition*>()
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
        DocPosition *dp = it.next();
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
        DocPosition *dp = static_cast<DocPosition*>( it.next() );

        auto tt = dp->taxType();
        if( !dp->toDelete() && tt == DocPosition::Tax::Full ) {
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
        DocPosition *dp = it.next();

        if( !dp->toDelete() && dp->taxType() == DocPosition::Tax::Reduced ) {
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

QString DocPositionList::posNumber( DocPosition* pos )
{
    return QString::number( 1+indexOf( pos ) );
}


int DocPositionList::compareItems ( DocPosition *dp1, DocPosition *dp2 )
{
    //DocPosition *dpb1 = static_cast<DocPosition*>( item1 );
    //DocPosition *dpb2 = static_cast<DocPosition*>( item2 );

    int sortkey1 = dp1->positionNumber();
    int sortkey2 = dp2->positionNumber();

    int res = 0;
    if( sortkey1 > sortkey2 ) res = 1;
    if( sortkey2 < sortkey1 ) res = -1;

    // qDebug()<< "In sort: comparing " << p1 << " with " << p2 << " = " << res;
    return res;
}

DocPosition::Tax DocPositionList::listTaxation() const
{
    int fullTax = 0;
    int noTax = 0;
    int redTax = 0;

    DocPosition::Tax ret = DocPosition::Tax::None;

    const_iterator it;
    for ( it = begin(); it != end(); ++it ) {
        if( (*it)->taxType() == DocPosition::Tax::Full) {
            fullTax++;
        } else if( (*it)->taxType() == DocPosition::Tax::Reduced ) {
            redTax++;
        } else if( (*it)->taxType() == DocPosition::Tax::None ) {
            noTax++;
        }
    }

    int cnt = count();
    if (noTax == cnt) {
        ret = DocPosition::Tax::None;
    } else if (redTax == cnt) {
        ret = DocPosition::Tax::Reduced;
    } else if (fullTax == cnt) {
        ret = DocPosition::Tax::Full;
    } else
        ret = DocPosition::Tax::Individual;

    return ret;
}

bool DocPositionList::hasIndividualTaxes() const
{
    bool re = listTaxation() == DocPosition::Tax::Individual;
    return re;
}


QDomElement DocPositionList::xmlTextElement( QDomDocument& doc, const QString& name, const QString& value )
{
    QDomElement elem = doc.createElement( name );
    QDomText t = doc.createTextNode( value );
    elem.appendChild( t );
    return elem;
}

DocPosition *DocPositionList::positionFromId( int id )
{
    DocPosition *dp{nullptr};

    DocPositionListIterator it(*this);
    while( it.hasNext() ) {
        dp = static_cast<DocPosition*>( it.next() );

        if( dp->dbId() == id ) {
            break;
        }
    }
    return dp;
}


