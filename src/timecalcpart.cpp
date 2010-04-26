/***************************************************************************
                          timecalcpart.cpp  -
                             -------------------
    begin                : Don Jan 1 2004
    copyright            : (C) 2004 by Klaas Freitag
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

#include <klocale.h>
#include <kdebug.h>

#include "timecalcpart.h"


TimeCalcPart::TimeCalcPart()
  :CalcPart(),
   m_minuten( 0 ),
   m_allowGlobalStundensatz( false )
{

}

TimeCalcPart::TimeCalcPart(const QString& name, int minutes, int prozent)
:CalcPart( name, prozent ),
 m_minuten( minutes ),
 m_allowGlobalStundensatz(true)
{

}

TimeCalcPart::~TimeCalcPart()
{

}

StdSatz& TimeCalcPart::getStundensatz()
{
    return m_stundensatz;
}

/** Write property of Geld m_stundensatz. */
void TimeCalcPart::setStundensatz( const StdSatz& _newVal)
{
    // kDebug() << "stundensatz gesetzt: " << _newVal.toString() << endl;
    m_stundensatz = _newVal;
    setDirty(true);
}

void TimeCalcPart::setGlobalStdSetAllowed( bool s  )
{
    if( m_allowGlobalStundensatz != s )
    {
        m_allowGlobalStundensatz = s;
        setDirty(true);
    }
}

void TimeCalcPart::setMinuten( int m )
{
    if( m_minuten != m )
    {
        m_minuten = m;
        setDirty(true);
    }
}


Geld TimeCalcPart::basisKosten()
{
    StdSatz stdSatz = getStundensatz();

    // Wichtig hier: toDouble, sonst wird wild gecastet !!
    Geld g( stdSatz.getPreis().toDouble() * long(m_minuten) / 60);
    return g;
}

QString TimeCalcPart::getType() const
{
    return KALKPART_TIME;
}
