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
#include <QSqlQuery>

#include <QDebug>

#include "catalogchapter.h"
#include "kraftdb.h"

CatalogChapter::CatalogChapter()
  : mSortKey(0)
{

}

CatalogChapter::CatalogChapter( int id, int csId, const QString& name,
                                int parent, const QString& desc )
    :mName( name ),
    mId( dbID(id) ),
    mCatalogSetId( dbID(csId) ),
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

dbID CatalogChapter::catalogSetId() const
{
  return mCatalogSetId;
}

void CatalogChapter::setCatalogSetId( const dbID& id )
{
  mCatalogSetId = id;
}


dbID CatalogChapter::parentId() const
{
  return mParentId;
}

void CatalogChapter::setParentId( const dbID &id )
{
  mParentId = id;
}

int CatalogChapter::sortKey() const
{
  return mSortKey;
}

void CatalogChapter::setSortKey( int key )
{
  mSortKey = key;
}

void CatalogChapter::save()
{
  // qDebug () << "Inserting new chapter " << name() << mCatalogSetId.toString() << endl;
  QSqlQuery q;
  q.prepare("INSERT INTO CatalogChapters (catalogSetID, chapter, description, sortKey, parentChapter)"
            "VALUES(:catalogSetID, :chapter, :desc, :sortKey, :parentChapter)");
  q.bindValue( ":catalogSetID",  mCatalogSetId.toString() );
  q.bindValue( ":chapter",       this->name() );
  q.bindValue( ":desc",          this->description() );
  q.bindValue( ":sortKey",       this->sortKey() );
  q.bindValue( ":parentChapter", this->parentId().toInt() );
  q.exec();

  mId = KraftDB::self()->getLastInsertID();
}

bool CatalogChapter::removeFromDB()
{
    // qDebug () << "Removing chapter " << name() << " with id " << mId.toInt();

    QSqlQuery q;
    q.prepare("DELETE FROM CatalogChapters WHERE chapterID=:chapId");

    q.bindValue( ":chapId",  mId.toInt() );
    return q.exec();
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

void CatalogChapter::reparent( const dbID& pId )
{
  dbID parentId( pId );
  setParentId( pId );
  QSqlQuery q;
  q.prepare("UPDATE CatalogChapters SET parentChapter= :p WHERE chapterID = :id");
  q.bindValue(":id", mId.toInt() );
  q.bindValue(":p", parentId.toInt() );
  q.exec();
  // qDebug () << "Reparenting chapter " << mId.toInt() << ", reuslt: " << q.lastError().text();
}
