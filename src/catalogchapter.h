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

#ifndef CATALOGCHAPTER_H
#define CATALOGCHAPTER_H

#include <QtCore>

#include "kraftcat_export.h"

#include <dbids.h>

class KRAFTCAT_EXPORT CatalogChapter
{
public:
  CatalogChapter();
  CatalogChapter( int, int, const QString&, int, const QString& = QString() );

  QString name() const;
  void setName( const QString& );

  QString description() const;
  void setDescription( const QString& );

  dbID id() const;

  dbID parentId() const;
  void setParentId( const dbID& );

  dbID catalogSetId() const;
  void setCatalogSetId( const dbID& );

  bool removeFromDB();

  int sortKey() const;
  void setSortKey( int );

  void save( );
  void saveNameAndDesc();
  void reparent( const dbID& );

private:
  QString mName;
  dbID    mId;
  dbID    mCatalogSetId;
  QString mDescription;
  dbID    mParentId;
  int     mSortKey;

};


#endif // CATALOGCHAPTER_H
