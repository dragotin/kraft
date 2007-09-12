/***************************************************************************
          archdocposition.cpp  - a position in an archived document
                             -------------------
    begin                : Sep. 2006
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

// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>

// application specific includes
#include "einheit.h"
#include "geld.h"
#include "archdocposition.h"

/**
@author Klaas Freitag
*/


ArchDocPosition::ArchDocPosition()
  : mAmount( 0 )
{

}

Geld ArchDocPosition::overallPrice()
{
  return mOverallPrice;
}

ArchDocPositionList::ArchDocPositionList()
    : QValueList<ArchDocPosition>()
{

}

Geld ArchDocPositionList::sumPrice()
{
    Geld g;

    iterator it;
    for ( it = begin(); it != end(); ++it ) {
      g += ( *it ).overallPrice();
    }

    return g;
}


