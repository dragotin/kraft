/***************************************************************************
                          zeitcalcpart.cpp  -
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

#include "zeitcalcpart.h"


ZeitCalcPart::ZeitCalcPart()
  :CalcPart(),
   m_minuten( 0 ),
   m_allowGlobalStundensatz( false )
{

}

ZeitCalcPart::ZeitCalcPart(const QString& name, int minutes, int prozent)
:CalcPart( name, prozent ),
 m_minuten( minutes ),
 m_allowGlobalStundensatz(true)
{

}

ZeitCalcPart::~ZeitCalcPart()
{

}

/** Stundensatz zurckgeben. */
StdSatz& ZeitCalcPart::getStundensatz()
{
    return m_stundensatz;
}

/** Write property of Geld m_stundensatz. */
void ZeitCalcPart::setStundensatz( const StdSatz& _newVal)
{
    // kdDebug() << "stundensatz gesetzt: " << _newVal.toString() << endl;
    m_stundensatz = _newVal;
    setDirty(true);
}

void ZeitCalcPart::setGlobalStdSetAllowed( bool s  )
{
    if( m_allowGlobalStundensatz != s )
    {
        m_allowGlobalStundensatz = s;
        setDirty(true);
    }
}

void ZeitCalcPart::setMinuten( int m )
{
    if( m_minuten != m )
    {
        m_minuten = m;
        setDirty(true);
    }
}


Geld ZeitCalcPart::basisKosten()
{
    StdSatz stdSatz = getStundensatz();

    // Wichtig hier: toDouble, sonst wird wild gecastet !!
    Geld g( stdSatz.getPreis().toDouble() * long(m_minuten) / 60);
    return g;
}

QString ZeitCalcPart::getType() const
{
    return KALKPART_TIME;
}
