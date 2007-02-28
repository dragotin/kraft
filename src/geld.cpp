/***************************************************************************
       geld  - A class that represents money and supports calculation
                             -------------------
    begin                : 2004-16-08
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

// include files for Qt

// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>

#include "geld.h"


Geld::Geld( )
{
    m_cent = 0;
}

Geld::Geld( long l )
{
    m_cent = l;
}

Geld::Geld( double g )
{
    m_cent = qRound(100*g);
}

Geld& Geld::operator=(const long l)
{
    m_cent = l;
    return *this;
}

Geld& Geld::operator=(const double d)
{
    m_cent = qRound( 100.0 * d );
    return *this;
}

Geld& Geld::operator=(const Geld& g)
{
  if ( this == &g ) return *this;

  m_cent = g.m_cent;
  return *this;
}

Geld& Geld::operator+=(const Geld& g)
{
    m_cent += g.m_cent;
    return *this;
}

Geld Geld::operator/(const double divisor)
{
    // FIXME
    Geld g( this->m_cent / divisor / 100 );
    return g;
}

Geld Geld::operator*(const long mult)
{
    // FIXME
    Geld g( this->m_cent * mult / 100);
    return  g;
}

Geld Geld::operator*(const double mult)
{
    Geld g(double(this->m_cent) * mult / 100);
    return g;
}

bool Geld::operator!=(Geld g)
{
    return g.m_cent != m_cent;
}

QString Geld::toString() const
{
    return KGlobal().locale()->formatMoney(m_cent/100.0);
}

QString Geld::toHtmlString() const
{
  QString re = toString();
  re.replace( " ",  "&nbsp;" );
  return re;
}

double Geld::toDouble()
{
    return m_cent/100.0;
}

long Geld::toLong()
{
    return m_cent;
}

Geld::~Geld( )
{

}

/* END */

