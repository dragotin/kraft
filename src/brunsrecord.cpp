/***************************************************************************
                    brunsrecord.h - One Bruns Plant record
                             -------------------
    begin                : 2005-07
    copyright            : (C) 2005 by Klaas Freitag
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
#include <kdebug.h>

#include "brunsrecord.h"
#include "geld.h"

// *********************************************** BrunsSize
BrunsSize::BrunsSize() :
formNo(0),
grothNo(0),
rootPack(0),
qualityAdd(0),
formAdd(0),
goodsGroup(0),
sizeNo( 0 ),
sizeAddNo( 0 )
{

}

// *********************************************** BrunsRecord


BrunsRecord::BrunsRecord(int d)
    : CatalogTemplate(),
      artNo(d),
      passNeeded( false ),
      plantGroup( 0 ),
      artId( 0 )
{

}


BrunsRecord::~BrunsRecord()
{

}

void BrunsRecord::addSize( const BrunsSize& size )
{
    m_sizes.append(size);
}

void BrunsRecord::clearSizes()
{
    m_sizes.clear();
}

void BrunsRecord::debugOut()
{
    kDebug() << artNo << "  dt. Name: " << dtName << ", lt. Name. " << ltName << endl;
}

bool BrunsRecord::save()
{
  return true;
}

Geld BrunsRecord::unitPrice()
{
  Geld g;
  return g;
}
