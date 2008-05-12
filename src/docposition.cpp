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
#include <qptrlist.h>

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
#include "kraftview.h"

/**
@author Klaas Freitag
*/

DocPositionBase::DocPositionBase() : QObject(),
                                     m_dbId( -1 ),
                                     m_position( 0 ),
                                     mToDelete( false ),
                                     mType( Position ),
                                     mAttribs( QString::fromLatin1( "Position" ) )

{

}

DocPositionBase::DocPositionBase( const PositionType& t )
  : QObject(),
    m_dbId( -1 ),
    m_position( 0 ),
    mToDelete( false ),
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
    mType( b.mType ),
    mAttribs( b.mAttribs )
{

}

DocPositionBase& DocPositionBase::operator=( const DocPositionBase& dp )
{
  kdDebug() << "Copyconstructor BASE " << dp.m_text << endl;
  if ( this == &dp ) return *this;
  m_dbId = dp.m_dbId;
  m_position = dp.m_position;
  m_text = dp.m_text;
  mToDelete = dp.mToDelete;
  mType = dp.mType;
  mAttribs = dp.mAttribs;

  return *this;
}

void DocPositionBase::setAttribute( const Attribute& attrib )
{
  mAttribs[ attrib.name() ] = attrib;
}

AttributeMap DocPositionBase::attributes()
{
  return mAttribs;
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
    mAttribs.remove( name );
}

QString DocPositionBase::attribute( const QString& attName ) const
{
  Attribute att = mAttribs[ attName ];

  return att.value().toString();
}

// ############################################################## Pricing
Pricing::Pricing( DocPosition *pos )
  :mMyPosition( pos ),
   mAmount( 1.0 )
{

}

Pricing::~Pricing()
{

}

Geld Pricing::overallPrice()
{
  Geld g;
  if ( mMyPosition ) {
    AttributeMap atts = mMyPosition->attributes();
    // all kinds besind from no kind mean  that the position is not
    // counted for the overall price. That's a FIXME
    if ( ! atts.contains( DocPosition::Kind ) ) {
      g = unitPrice()*amount();
    }
  }
  return g;
}
// ############################################################## DiscoutPricing
DiscountPricing::DiscountPricing(  DocPosition *dp, KraftView *view )
  : Pricing( dp ), mDiscount( 100.0 ), mKraftView( view )
{

}

DiscountPricing::~DiscountPricing()
{

}

void DiscountPricing::setDiscount( double d )
{
  mDiscount = d;
}

double DiscountPricing::discount()
{
  return mDiscount;
}

void DiscountPricing::setFilterTag( const QString& t )
{
  mTag = t;
}

QString DiscountPricing::tag() const
{
  return mTag;
}

Geld DiscountPricing::unitPrice() const
{
  Geld g;
  if ( mKraftView ) {
   DocPositionList positions = mKraftView->currentPositionList();

    QPtrListIterator<DocPositionBase> it( positions );
    DocPositionBase *dpb;

    while ( ( dpb = it.current() ) != 0 ) {
      ++it;
      if( dpb->type() != DocPosition::Header ) {
        DocPosition *dp = static_cast<DocPosition*>(dpb);

      	kdDebug() << "Position is " << dpb->position() << " and my is "
        	        << mMyPosition << endl;
        /*
         * only count positions that are different from us
         */
        if ( mMyPosition->position() != dpb->position() ) {
          //kdDebug() << "Adding overall Price: " << ( dp->overallPrice() ).toDouble() << endl;
          g += dp->overallPrice();
        }
      } else {
	kdDebug() << "in unitPrice: no valid type !" << endl;
      }
    }
  } else {
    kdDebug() << "unitPrice (DiscountPricing): no kraftview set " << endl;
  }
  g = g.percent( mDiscount );
  kdDebug() << "Returning unit price: " << g.toDouble() << endl;
  return g;
}

Geld DiscountPricing::overallPrice()
{
  return unitPrice();
}

// ############################################################## DocPosition

const QString DocPosition::Kind( QString::fromLatin1( "kind" ) );
const QString DocPosition::Discount( QString::fromLatin1( "discount" ) );


DocPosition::DocPosition()
  : DocPositionBase()
  , mPricing( new Pricing( this ) ) // type defaults to Position
{

}

DocPosition::DocPosition( const PositionType& t )
  : DocPositionBase( t )
{
  if ( t == Position ) {
    mPricing = new Pricing( this );
  } else if ( t == ExtraDiscount ) {
    mPricing = new DiscountPricing( this, 0 );
  } else {
    mPricing = 0;
  }
}

DocPosition::DocPosition( const DocPosition& dp )
  : DocPositionBase( dp )
{
  if ( mType == Position ) {
    mPricing = new Pricing( *( dp.mPricing ) );
  } else if ( mType == ExtraDiscount ) {
    mPricing = new DiscountPricing( *( static_cast<DiscountPricing*>( dp.mPricing )) );
  } else {
    mPricing = 0;
  }

  m_unit = dp.m_unit;
}

DocPosition::~DocPosition()
{
  if ( mPricing ) delete mPricing;
}

DocPosition& DocPosition::operator=( const DocPosition& dp )
{
  if ( this == &dp ) return *this;

  DocPositionBase::operator=( dp );
  m_unit     = dp.m_unit;

  // FIXME: Pricing !!!
  if ( mPricing ) delete mPricing;
  if ( mType == Position ) {
    mPricing = new Pricing( *( dp.mPricing ) );
  } else if ( mType == ExtraDiscount ) {
    mPricing = new DiscountPricing( *( static_cast<DiscountPricing*>( dp.mPricing ) ) );
  } else {
    mPricing = 0;
  }

  return *this;
}

void DocPosition::setPricing( Pricing* p )
{
  if ( mPricing ) {
    delete mPricing;
  }
  mPricing = p;
}


// ############################################################## DocPosition

DocPositionList::DocPositionList()
  : QPtrList<DocPositionBase>(), mLocale( 0 )
{
  setAutoDelete( true );
}

Geld DocPositionList::sumPrice()
{
  Geld g;

  DocPositionBase *dp;
  for ( dp = first(); dp; dp = next() ) {
    if ( ! dp->toDelete() ) { // only count non deleted positions
      if( dp->type() == DocPositionBase::Position ||
          dp->type() == DocPositionBase::ExtraDiscount ) {
        g += static_cast<DocPosition*>(dp)->overallPrice();
      }
    }
  }
  return g;
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

  int p1 = dpb1->position();
  int p2 = dpb2->position();

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


