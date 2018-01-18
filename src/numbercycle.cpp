/***************************************************************************
                   numbercycle.h  - document number cycles
                             -------------------
    begin                : Jan 15 2009
    copyright            : (C) 2009 by Klaas Freitag
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

#include "numbercycle.h"

NumberCycle::NumberCycle()
{

}

NumberCycle::NumberCycle( dbID _id )
  :id( _id )
{
}

void NumberCycle::setName( const QString& n )
{
  mName = n;
}

QString NumberCycle::name()
{
  return mName;
}

void NumberCycle::setTemplate( const QString& t )
{
  mTemplate = t;
}

QString NumberCycle::getTemplate()
{
  return mTemplate;
}

void NumberCycle::setCounter( int c )
{
  mCounter = c;
}

int  NumberCycle::counter()
{
  return mCounter;
}

QString NumberCycle::defaultName()
{
  return QString( "default" );
}
