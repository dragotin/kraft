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
#include <math.h>

#include <klocale.h>
#include <kdebug.h>

#include "calcpart.h"

CalcPart::CalcPart( ):
    m_prozentPlus(0),
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
/** costs for this calculation part */
Geld CalcPart::kosten() {
   /* Hier die virtuelle Funktion basisKosten aufrufen,
    * die in abgeleiteten Klassen die Basiskosten berechnet.
    * Der prozentuale Aufschlag kommt dann hier.
    */
   Geld g = basisKosten();
   // kdDebug() << "Basecosts: " << g.toString() << endl;
   double prozente = getProzentPlus();

   if( fabs(prozente) > 0.0 )
   {
        Geld aufschlag = g * double(prozente/100.0);
        // kdDebug() << "Have Money: " << g.toString() << " und " << prozente << " macht Aufschlag: " << aufschlag.toString() << endl;
        g += aufschlag;
   }
   // kdDebug() << "Overall sum: " << g.toString() << endl;
   return g;
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
