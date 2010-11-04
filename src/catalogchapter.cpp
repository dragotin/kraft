/***************************************************************************
             catalogchapter.h  - a simle catalog chapter object
                             -------------------
    begin                : Thu Nov 4 2010
    copyright            : (C) 2010 by Klaas Freitag
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

#include "catalogchapter.h"

CatalogChapter::CatalogChapter()
{

}

CatalogChapter::CatalogChapter( int id, const QString& name, int parent, const QString& desc )
    :mName( name ),
    mId( dbID(id) ),
    mDescription( desc ),
    mParentId( parent )
{

}

QString CatalogChapter::name() const
{
  return mName;
}

QString CatalogChapter::description() const
{
  return mDescription;
}

dbID CatalogChapter::id() const
{
  return mId;
}

dbID CatalogChapter::parentId() const
{
  return mParentId;
}
