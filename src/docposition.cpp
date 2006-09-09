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

/**
@author Klaas Freitag
*/

DocPositionBase::DocPositionBase() : QObject(),
                                     m_dbId( -1 ),
                                     mToDelete( false )
{

}


DocPositionBase::DocPositionBase(const DocPositionBase& b )
  : QObject()
{
  m_dbId     = b.m_dbId;
  m_position = b.m_position;
  mToDelete  = b.mToDelete;
}

// ##############################################################

DocPosition::DocPosition(): DocPositionBase()
  ,m_amount( 1.0 )
{
  m_text = QString();
}

Geld DocPosition::overallPrice()
{
    Geld g;

    g = unitPrice()*amount();
    return g;
}

DocPosition& DocPosition::operator=( const DocPosition& dp )
{
  m_text = dp.m_text;
  m_position = dp.m_position;
  m_unit = dp.m_unit;
  m_unitPrice = dp.m_unitPrice;
  m_amount = dp.m_amount;
  m_dbId = dp.m_dbId;

  return *this;
}

DocPositionList::DocPositionList()
    : QPtrList<DocPositionBase>()
{
  setAutoDelete( true );
}

Geld DocPositionList::sumPrice()
{
    Geld g;

    DocPositionBase *dp;
    for ( dp = first(); dp; dp = next() ) {
        if( dp->type() == DocPositionBase::Position ) {
          g += static_cast<DocPosition*>(dp)->overallPrice();
        }
    }
    return g;
}

QDomElement DocPositionList::domElement( QDomDocument& doc )
{
    QDomElement topElem = doc.createElement( "positions" );
    QDomElement posElem;

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
        QString h = KGlobal().locale()->formatNumber( am, 2 );
        posElem.appendChild( xmlTextElement( doc, "amount", h ));

        Einheit e = dp->unit();
        posElem.appendChild( xmlTextElement( doc, "unit", e.einheit( am ) ) );

        Geld g = dp->unitPrice();
        posElem.appendChild( xmlTextElement( doc, "unitprice", g.toString() ) );

        posElem.appendChild( xmlTextElement( doc, "sumprice", Geld( g*am).toString() ) );
      }
    }
    return topElem;
}

int DocPositionList::compareItems ( QPtrCollection::Item item1, QPtrCollection::Item item2 )
{
  DocPositionBase *dpb1 = static_cast<DocPositionBase*>( item1 );
  DocPositionBase *dpb2 = static_cast<DocPositionBase*>( item2 );

  QString pos1 = dpb1->position();
  QString pos2 = dpb2->position();

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
