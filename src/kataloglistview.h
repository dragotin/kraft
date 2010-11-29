/***************************************************************************
                          floskellistview.h  -
                             -------------------
    begin                : Son Feb 8 2004
    copyright            : (C) 2004 by Klaas Freitag
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

#ifndef KATALOGLISTVIEW_H
#define KATALOGLISTVIEW_H

#include <QPixmap>
#include <QTreeWidgetItem>
#include <QTreeWidget>

#include "kraftcat_export.h"

/**
  *@author Klaas Freitag
  */

class TemplKatalog;
class QPixmap;
class KMenu;
class DocPosition;
class Katalog;
class CatalogChapter;


class KRAFTCAT_EXPORT KatalogListView : public QTreeWidget
{
  Q_OBJECT
public:
  KatalogListView( QWidget *parent = 0, bool enableCheckboxes = false );
  ~KatalogListView();

  virtual void addCatalogDisplay( const QString& );
  virtual void* currentItemData();
  virtual void* itemData( QTreeWidgetItem* );

  bool isChapter(QTreeWidgetItem*);
  bool isRoot(QTreeWidgetItem*);

  virtual void setupChapters();

  KMenu *contextMenu();
  // virtual DocPosition itemToDocPosition( QListViewItem *it = 0 ) = 0;

public slots:
  virtual void slotFreshupItem( QTreeWidgetItem*, void*, bool remChildren = false );
  virtual void slotCreateNewChapter();
  virtual void slotEditCurrentChapter();
  virtual void slotRemoveCurrentChapter();

  virtual void contextMenuEvent( QContextMenuEvent * event );
  virtual void slotRedraw();

protected slots:
  virtual void slotUpdateSequence();

protected:
  virtual Katalog* catalog();
  void dropEvent( QDropEvent* );

  QTreeWidgetItem* tryAddingCatalogChapter( const CatalogChapter& );

  QTreeWidgetItem *m_root;
  QHash<QTreeWidgetItem*, void*> m_dataDict;
  QHash<int, QTreeWidgetItem*> mChapterDict;
  QString m_catalogName;
  QStringList mOpenChapters;
  QTreeWidgetItem *mSortChapterItem;
  KMenu *mMenu;
  QFont mChapterFont;
};

#endif
