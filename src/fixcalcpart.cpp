/***************************************************************************
                          fixcalcpart.cpp  -
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

#include "fixcalcpart.h"

FixCalcPart::FixCalcPart()
  :CalcPart(),
   m_amount( 0 )
{

}

FixCalcPart::FixCalcPart(QString name, Geld preis, int prozent )
: CalcPart(name, prozent),
  m_fixPreis(preis),
  m_amount(1.0)
{
    // setProzentPlus(prozent);
}

void FixCalcPart::setMenge( double val )
{
    if( val != m_amount )
    {
        m_amount = val;
        setDirty(true);
    }
}

QString FixCalcPart::getType() const
{
    return KALKPART_FIX;
}

Geld FixCalcPart::unitPreis()
{
    return m_fixPreis;
}

void FixCalcPart::setUnitPreis( Geld g )
{
    if( g != m_fixPreis )
    {
        m_fixPreis = g;
        setDirty(true);
    }
}

FixCalcPart::~FixCalcPart()
{

}

Geld FixCalcPart::basisKosten()
{
    Geld g = m_fixPreis;
    g = (Geld) m_fixPreis*m_amount;
    return g;
}
