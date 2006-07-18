/***************************************************************************
             brunskataloglistview  - template katalog listview.
                             -------------------
    begin                : 2005-07-26
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
#ifndef BRUNSKATALOGLISTVIEW_H
#define BRUNSKATALOGLISTVIEW_H

#include <qmap.h>

#include <kataloglistview.h>

class BrunsRecord;
class QListViewItem;
class KListViewItem;

/**
A listview that presents the contents of the Bruns Catalog

@author Klaas Freitag
*/
class BrunsKatalogListView : public KatalogListView
{
public:
    typedef enum { Fruits, Rhodos, Roses, Stauden, Konis, Etc } TopKatalogIds;
    
    BrunsKatalogListView(QWidget *w);

    ~BrunsKatalogListView();
     void addCatalogDisplay( const QString& katName );
     BrunsRecord getRecord( QListViewItem * );
     void setupChapters();
private:

    QMap<QListViewItem*, BrunsRecord> m_itemMap;
    QMap<TopKatalogIds, KListViewItem*> m_topFolderMap;
};

#endif
