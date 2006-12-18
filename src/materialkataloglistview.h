/***************************************************************************
             materialkataloglistview  - material katalog listview.
                             -------------------
    begin                : 2006-11-30
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
#ifndef MATERIALKATALOGLISTVIEW_H
#define MATERIALKATALOGLISTVIEW_H

#include <qmap.h>

#include <kataloglistview.h>

class QListViewItem;
class KListViewItem;
class StockMaterial;

/**
A listview that presents the contents of the Bruns Catalog

@author Klaas Freitag
*/
class MaterialKatalogListView : public KatalogListView
{
public:
  MaterialKatalogListView(QWidget *, bool enableCheckboxes = false );

  ~MaterialKatalogListView();
  void addCatalogDisplay( const QString& katName );
  DocPosition itemToDocPosition( QListViewItem *it = 0 );
  QListViewItem* addMaterialToView( KListViewItem *, StockMaterial * );
public slots:
  void slFreshupItem( QListViewItem *, void*,  bool remChildren = false  );

private:
  bool mCheckboxes;
};

#endif
