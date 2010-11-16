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

#include <QtCore>
#include <QtGui>
#include <QtSql>

#include <kiconloader.h>
#include <kdebug.h>

#include "catalogchapter.h"

CatalogChapter::CatalogChapter()
  : mSortKey(0)
{

}

CatalogChapter::CatalogChapter( int id, const QString& name, int parent, const QString& desc )
    :mName( name ),
    mId( dbID(id) ),
    mDescription( desc ),
    mParentId( parent ),
    mSortKey(0)
{

}

QString CatalogChapter::name() const
{
  return mName;
}

void CatalogChapter::setName( const QString& name )
{
  mName = name;
}

QString CatalogChapter::description() const
{
  return mDescription;
}

void CatalogChapter::setDescription( const QString& d )
{
  mDescription = d;
}


dbID CatalogChapter::id() const
{
  return mId;
}

dbID CatalogChapter::parentId() const
{
  return mParentId;
}

void CatalogChapter::setParentId( const dbID &id )
{
  mParentId = id;
}

QPixmap CatalogChapter::icon() const
{
  return SmallIcon("folder-documents");
}

void CatalogChapter::setIcon( const QPixmap & )
{
  // do nothing for now
}

int CatalogChapter::sortKey() const
{
  return mSortKey;
}

void CatalogChapter::setSortKey( int key )
{
  mSortKey = key;
}

void CatalogChapter::save( const dbID& catalogSetID )
{
  kDebug() << "Inserting new chapter " << name() << sortKey() << endl;
  QSqlQuery q;
  q.prepare("INSERT INTO CatalogChapters (catalogSetID, chapter, description, sortKey, parentChapter)"
            "VALUES(:catalogSetID, :chapter, :desc, :sortKey, :parentChapter)");
  q.bindValue( ":catalogSetID",  catalogSetID.toString() );
  q.bindValue( ":chapter",       this->name() );
  q.bindValue( ":desc",          this->description() );
  q.bindValue( ":sortKey",       this->sortKey() );
  q.bindValue( ":parentChapter", this->parentId().toInt() );
  q.exec();

}

void CatalogChapter::saveNameAndDesc()
{
  QSqlQuery q;
  q.prepare("UPDATE CatalogChapters SET chapter = :newchapter, description = :desc WHERE chapterID = :id");
  q.bindValue(":id", mId.toInt() );
  q.bindValue(":desc", this->description() );
  q.bindValue(":newchapter", this->name() );
  q.exec();
}
