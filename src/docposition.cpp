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

DocPositionBase::DocPositionBase(const DocPositionBase& b )
  : KraftObj(),
    m_dbId( b.m_dbId ),
    m_position( b.m_position ),
    m_text( b.m_text ),
    mToDelete( b.mToDelete ),
    mTaxType( TaxFull ),
    mType( b.mType )
{
    const QMap<QString, KraftAttrib> attrib = b.attributes();
    for (const auto& m:attrib) {
        setAttribute(m);
    }
}

DocPositionBase& DocPositionBase::operator=( const DocPositionBase& dp )
{
  if ( this == &dp ) return *this;

  m_dbId = dp.m_dbId;
  m_position = dp.m_position;
  m_text = dp.m_text;
  mToDelete = dp.mToDelete;
  mType = dp.mType;

  const QMap<QString, KraftAttrib> attrib = dp.attributes();
  for (const auto& m:attrib) {
      setAttribute(m);
  }
  mTaxType = dp.mTaxType;

  return *this;
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
    switch (type()) {
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

const QString DocPosition::Kind{"kind"};
const QString DocPosition::Discount{"discount"};
const QString DocPosition::Tags{"tags"};
const QString DocPosition::ExtraDiscountTagRequired{"discountTagRequired"};

DocPosition::DocPosition(): DocPositionBase()
  ,m_amount( 1.0 ), mWidget( 0 )

{
  m_text = QString();
}

DocPosition::DocPosition( const PositionType& t )
  : DocPositionBase( t ), mWidget( 0 )
{

}

Geld DocPosition::overallPrice()
{
    Geld g;

    if (type() == DocPositionBase::PositionType::Position) {
        g = unitPrice() * amount();
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
      DocPosition *dp = static_cast<DocPosition*>(it.next());
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

    if( !dp->toDelete() && dp->taxTypeNumeric() == DocPositionBase::TaxFull ) {
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
    DocPosition *dp = static_cast<DocPosition*>( it.next() );

    if( !dp->toDelete() && dp->taxTypeNumeric() == DocPositionBase::TaxReduced ) {
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

#if 0
QDomElement DocPositionList::domElement( QDomDocument& doc )
{
  QDomElement topElem = doc.createElement( "positions" );
  QDomElement posElem;

  int num = 1;

  DocPositionListIterator it( *this );
  while( it.hasNext() ) {
    DocPosition *dpb = static_cast<DocPosition*>( it.next() );

    if( dpb->type() == DocPositionBase::Position ) {
      DocPosition *dp = static_cast<DocPosition*>(dpb);

      posElem = doc.createElement( "position" );
      posElem.setAttribute( "number", num++ );
      topElem.appendChild( posElem );
      posElem.appendChild( xmlTextElement( doc, "text", dp->text() ) );

      double am = dp->amount();
      QString h = QString::number(am, 'f', 2 );
      posElem.appendChild( xmlTextElement( doc, "amount", h ));

      Einheit e = dp->unit();
      posElem.appendChild( xmlTextElement( doc, "unit", e.einheit( am ) ) );

      Geld g = dp->unitPrice();
      posElem.appendChild( xmlTextElement( doc, "unitprice", QString::number(g.toDouble(), 'f', 2 )));

      Geld sum(g * am);

      posElem.appendChild( xmlTextElement( doc, "sumprice", QString::number(sum.toDouble(), 'f', 2 ) ) );
    }
  }
  return topElem;
}
#endif

int DocPositionList::compareItems ( DocPosition *dp1, DocPosition *dp2 )
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


QDomElement DocPositionList::xmlTextElement( QDomDocument& doc, const QString& name, const QString& value )
{
    QDomElement elem = doc.createElement( name );
    QDomText t = doc.createTextNode( value );
    elem.appendChild( t );
    return elem;
}

DocPositionBase *DocPositionList::positionFromId( int id )
{
  DocPosition *dp = 0;

  DocPositionListIterator it( *this );
  while( it.hasNext() ) {
    dp = static_cast<DocPosition*>( it.next() );

    if( dp->dbId() == id ) {
      break;
    }
  }
  return dp;
}


