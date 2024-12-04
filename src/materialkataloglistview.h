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

class QTreeWidgetItem;
class StockMaterial;
class QLocale;
class DocPositionBase;
/**
A listview that presents the contents of the Bruns Catalog

@author Klaas Freitag
*/
class MaterialKatalogListView : public KatalogListView
{
public:
  MaterialKatalogListView(QWidget *parent=0 );

  ~MaterialKatalogListView();
  void addCatalogDisplay( const QString& katName );
  DocPositionBase itemToDocPosition( QTreeWidgetItem *it = nullptr );
  QTreeWidgetItem* addMaterialToView( QTreeWidgetItem*, StockMaterial* );

  void saveState();

public Q_SLOTS:
  void slFreshupItem( QTreeWidgetItem *, void*, QLocale* = 0  );

protected:
  void startUpdateItemSequence();

  void updateItemSequence(QTreeWidgetItem *item, int seqNo);

};

#endif
