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
#include <qvaluelist.h>
#include <qstring.h>
#include <qdom.h>

// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>

// application specific includes
#include "einheit.h"
#include "geld.h"
#include "docposition.h"
#include "positionwidget.h"
#include "positionviewwidget.h"
#include "defaultprovider.h"

/**
@author Klaas Freitag
*/

DocPositionBase::DocPositionBase() : QObject(),
                                     m_dbId( -1 ),
                                     mToDelete( false ),
                                     mTaxType( TaxFull ),
                                     mType( Position ),
                                     mAttribs( QString::fromLatin1( "Position" ) )

{

}

DocPositionBase::DocPositionBase( const PositionType& t )
  : QObject(),
    m_dbId( -1 ),
    mToDelete( false ),
    mTaxType( TaxFull ),
    mType( t ),
    mAttribs( QString::fromLatin1( "Position" ) )
{

}

DocPositionBase::DocPositionBase(const DocPositionBase& b )
  : QObject(),
    m_dbId( b.m_dbId ),
    m_position( b.m_position ),
    m_text( b.m_text ),
    mToDelete( b.mToDelete ),
    mTaxType( TaxFull ),
    mType( b.mType ),
    mAttribs( b.mAttribs )
{

}

DocPositionBase& DocPositionBase::operator=( const DocPositionBase& dp )
{
  if ( this == &dp ) return *this;

  m_dbId = dp.m_dbId;
  m_position = dp.m_position;
  m_text = dp.m_text;
  mToDelete = dp.mToDelete;
  mType = dp.mType;
  mAttribs = dp.mAttribs;
  mTaxType = dp.mTaxType;

  return *this;
}

void DocPositionBase::setAttribute( const Attribute& attrib )
{
  if ( attrib.name().isEmpty() )
    kdDebug()  << "WRN: Can not save attribute with empty name!" << endl;
  else
    mAttribs[ attrib.name() ] = attrib;
}

AttributeMap DocPositionBase::attributes()
{
  return mAttribs;
}

void DocPositionBase::setAttributeMap( AttributeMap attmap )
{
  mAttribs = attmap;
}

void DocPositionBase::loadAttributes()
{
  if ( m_dbId == -1 ) {
    kdDebug() << "Can not load attributes, no valid database id!" << endl;
    return;
  }
  mAttribs.load( m_dbId );
}

void DocPositionBase::removeAttribute( const QString& name )
{
  if ( !name.isEmpty() )
    mAttribs.markDelete( name );
}

QString DocPositionBase::attribute( const QString& attName ) const
{
  Attribute att = mAttribs[ attName ];

  return att.value().toString();
}

void DocPositionBase::setTag( const QString& tag )
{
  if ( tag.isEmpty() ) return;

  if ( mAttribs.contains( DocPosition::Tags ) ) {
    if ( hasTag( tag ) ) return;
    Attribute att = mAttribs[DocPosition::Tags];
    QStringList li =  att.value().toStringList();

    li.append( tag );
    att.setValue( QVariant( li ) );
    setAttribute( att );
  } else {
    QStringList li;
    li.append( tag );
    Attribute a( DocPosition::Tags );
    a.setValueRelation( "tagTemplates", "tagTmplID", "name" );
    a.setListValue( true );
    a.setPersistant( true );
    a.setValue( QVariant( li ) );
    setAttribute( a );
  }
}

void DocPositionBase::removeTag( const QString& tag )
{
  if ( hasTag( tag ) ) {
    Attribute att = mAttribs[DocPosition::Tags];
    QStringList li =  att.value().toStringList();

    li.remove( tag );
    att.setValue( QVariant( li ) );
    setAttribute( att );
  }
}

bool DocPositionBase::hasTag( const QString& tag )
{
  if ( ! mAttribs.contains( DocPosition::Tags ) ) {
    return false;
  }
  Attribute att = mAttribs[DocPosition::Tags];
  QStringList li =  att.value().toStringList();
  QStringList search = li.grep( tag, false ); // ignore case
  if ( ! search.isEmpty() ) {
    return true;
  }
  return false;
}

QStringList DocPositionBase::tags()
{
  QStringList tags;
  if ( mAttribs.contains( DocPosition::Tags ) ) {
    kdDebug() << mAttribs[DocPosition::Tags].toString() << endl;
    tags = mAttribs[DocPosition::Tags].value().toStringList();
  }
  return tags;
}

void DocPositionBase::setTaxType( TaxType tt )
{
  mTaxType = tt;
#if 0
  if ( type == 1 )
    mTaxType = TaxNone;
  else if ( type == 2 )
    mTaxType = TaxReduced;
  else if ( type == 3 )
    mTaxType = TaxFull;
  else {
    kdDebug() << "Ambigous vat type " << type << endl;
    mTaxType = TaxInvalid;
  }
#endif
}

int DocPositionBase::taxTypeNumeric()
{
  if ( mTaxType == TaxNone )
    return 1;
  else if ( mTaxType == TaxReduced )
    return 2;
  else if ( mTaxType == TaxFull )
    return 3;

  kdDebug() << "ERR: Vat-type ambigous!" << endl;
  return 0; // Invalid
}

// ##############################################################

const QString DocPosition::Kind( QString::fromLatin1( "kind" ) );
const QString DocPosition::Discount( QString::fromLatin1( "discount" ) );
const QString DocPosition::Tags( QString::fromLatin1( "tags" ) );
const QString DocPosition::ExtraDiscountTagRequired( QString::fromLatin1( "discountTagRequired" ) );

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
    AttributeMap atts = attributes();
    // all kinds beside from no kind mean  that the position is not
    // counted for the overall price. That's a FIXME
    if ( ! atts.contains( DocPosition::Kind ) ) {
      g = unitPrice() * amount();
    }
    return g;
}


DocPosition& DocPosition::operator=( const DocPosition& dp )
{
  if ( this == &dp ) return *this;

  DocPositionBase::operator=( dp );
  m_unit = dp.m_unit;
  m_unitPrice = dp.m_unitPrice;
  m_amount = dp.m_amount;
  mWidget = dp.mWidget;

  return *this;
}

// ##############################################################

DocPositionList::DocPositionList()
  : QPtrList<DocPositionBase>(), mLocale( 0 )
{
  setAutoDelete( true );
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

    DocPositionBase *dp;
    for ( dp = first(); dp; dp = next() ) {
          g += static_cast<DocPosition*>(dp)->overallPrice();
    }
    return g;
}

Geld DocPositionList::taxSum( double fullTax, double reducedTax )
{
    Geld sum;

    if ( fullTax < 0 ) {
      kdError() << "Full Tax is not loaded!" << endl;
    }

    DocPositionBase *dp;
    for ( dp = first(); dp; dp = next() ) {
      Geld g = static_cast<DocPosition*>(dp)->overallPrice();
      Geld tax;
      if ( dp->taxTypeNumeric() == DocPositionBase::TaxFull ) {
        tax = ( g * fullTax );
      } else if ( dp->taxTypeNumeric() == DocPositionBase::TaxReduced ) {
        tax = ( g * reducedTax );
      } else if ( dp->taxTypeNumeric() == DocPositionBase::TaxNone ) {
        kdDebug() << "no tax for pos " << dp->dbId().toString() << endl;
      } else {
        kdDebug() << "Unknown or invalid tax type for pos " << dp->dbId().toString() << endl;
      }
      sum += tax / 100.0;
    }
    return sum;
}

QString DocPositionList::posNumber( DocPositionBase* pos )
{
  return QString::number( 1+findRef( pos ) );
}

void DocPositionList::setLocale( KLocale* loc )
{
  mLocale = loc;
}

QDomElement DocPositionList::domElement( QDomDocument& doc )
{
    QDomElement topElem = doc.createElement( "positions" );
    QDomElement posElem;

    if ( !mLocale ) mLocale = DefaultProvider::self()->locale();

    int num = 1;
    DocPositionBase *dpb = 0;

    for ( dpb = first(); dpb; dpb = next() ) {
      if( dpb->type() == DocPositionBase::Position ) {
        DocPosition *dp = static_cast<DocPosition*>(dpb);

        posElem = doc.createElement( "position" );
        posElem.setAttribute( "number", num++ );
        topElem.appendChild( posElem );
        posElem.appendChild( xmlTextElement( doc, "text", dp->text() ) );

        double am = dp->amount();
        QString h = mLocale->formatNumber( am, 2 );
        posElem.appendChild( xmlTextElement( doc, "amount", h ));

        Einheit e = dp->unit();
        posElem.appendChild( xmlTextElement( doc, "unit", e.einheit( am ) ) );

        Geld g = dp->unitPrice();
        posElem.appendChild( xmlTextElement( doc, "unitprice", g.toString( mLocale ) ) );

        posElem.appendChild( xmlTextElement( doc, "sumprice", Geld( g*am).toString( mLocale ) ) );
      }
    }
    return topElem;
}

int DocPositionList::compareItems ( QPtrCollection::Item item1, QPtrCollection::Item item2 )
{
  DocPositionBase *dpb1 = static_cast<DocPositionBase*>( item1 );
  DocPositionBase *dpb2 = static_cast<DocPositionBase*>( item2 );

  QString pos1 = dpb1->positionNumber();
  QString pos2 = dpb2->positionNumber();

  int p1 = pos1.toInt();
  int p2 = pos2.toInt();

  int res = 0;
  if( p1 > p2 ) res = 1;
  if( p1 < p2 ) res = -1;

  // kdDebug()<< "In sort: comparing " << p1 << " with " << p2 << " = " << res << endl;
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
  DocPositionBase *dp = 0;

  for( dp = first(); dp ; dp = next() ) {
    if( dp->dbId() == id ) {
      break;
    }
  }
  return dp;
}


