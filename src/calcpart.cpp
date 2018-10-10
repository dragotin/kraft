/***************************************************************************
                          calcpart.cpp  -
                             -------------------
    begin                : Mit Dez 31 2003
    copyright            : (C) 2003 by Klaas Freitag
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

#include <QDebug>
#include <klocalizedstring.h>

#include "calcpart.h"
#include "fixcalcpart.h"
#include "materialcalcpart.h"
#include "timecalcpart.h"


CalcPart::CalcPart( ):
    m_prozentPlus(0),
    m_dbId(-1),
    m_templId(-1),
    m_dirty(false),
    m_toDelete(false)
{

}

CalcPart::CalcPart( int prozent ):
    m_prozentPlus( prozent ),
    m_dbId(-1),
    m_templId(-1),
    m_dirty(false),
    m_toDelete(false)
{

}


CalcPart::CalcPart(const QString& name, int prozent ) :
m_prozentPlus( prozent ),
m_name( name ),
m_dbId(-1),
m_templId(-1),
m_dirty(false),
m_toDelete(false)
{

}

CalcPart::~CalcPart()
{

}

/** Read property of int m_prozentPlus. */
const double& CalcPart::getProzentPlus()
{
    return m_prozentPlus;
}

/** Write property of int m_prozentPlus. */
void CalcPart::setProzentPlus( const double& _newVal)
{
    if( _newVal != m_prozentPlus )
    {
        m_prozentPlus = _newVal;
        setDirty(true);
    }
}

void CalcPart::setName( const QString& newName )
{
    if( newName != m_name )
    {
        m_name = newName;
        setDirty(true);
    }
}
/** Wird immer reimplementiert */
Geld CalcPart::basisKosten()
{
    Geld g;

    return g;
}

QString CalcPart::getType() const
{
    return i18n("Base");
}

void CalcPart::setToDelete(bool val)
{
    m_toDelete = val;
}

bool CalcPart::isToDelete()
{
    return m_toDelete;
}

/*
 * ===========================================================================
 */
CalcPartList::CalcPartList()
  :QList<CalcPart*>()
{

}

Geld CalcPartList::calcPrice()
{
  return costPerCalcPart( ALL_KALKPARTS );
}

Geld CalcPartList::costPerCalcPart( const QString& calcPart )
{
  CalcPart *cp;
  Geld g;

  /* suche nach einer speziellen Kalkulationsart */
  QListIterator<CalcPart*> i( *this );
  while( i.hasNext()) {
    cp = i.next();

    if( ( calcPart == ALL_KALKPARTS || calcPart == cp->getType() ) && ! cp->isToDelete() )
    {
      g += cp->basisKosten();
    }
  }
  return g;
}

/*
 * Attention: returning non deep copy here !
 */
CalcPartList CalcPartList::getCalcPartsList( const QString& calcPart )
{
  CalcPartList parts;

  if( calcPart == ALL_KALKPARTS )
    return *this;
  else
  {
    CalcPart *cp;
    /* suche nach einer speziellen Kalkulationsart */
    QListIterator<CalcPart*> i( *this );
    while( i.hasNext()) {
      cp = i.next();

      if( calcPart == cp->getType() && ! cp->isToDelete() )
      {
        parts.append(cp);
      }
    }
  }
  return( parts );
}


/*
 * Attention: returning non deep copy here !
 */
CalcPartList CalcPartList::decoupledCalcPartsList()
{
  CalcPartList parts;
  CalcPart *newcp = 0;
  CalcPart *cp;

  QListIterator<CalcPart*> i( *this );
  while( i.hasNext()) {
    cp = i.next();
    if ( cp->getType() == KALKPART_FIX ) {
      newcp = new FixCalcPart(  );
      *newcp = *( static_cast<FixCalcPart*>( cp ) );
    } else if ( cp->getType ()== KALKPART_TIME ) {
      newcp = new TimeCalcPart( );
      *newcp = *( static_cast<TimeCalcPart*>( cp ) );
    } else if ( cp->getType() == KALKPART_MATERIAL ) {
      newcp = new MaterialCalcPart(  );
      *newcp = *( static_cast<MaterialCalcPart*>( cp ) );

    }
    if ( newcp ) newcp->setDbID( -1 );
    parts.append( newcp );

  }
  return( parts );
}

