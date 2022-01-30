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
#include <QMenu>
#include <QSqlQuery>

#include "kraftcat_export.h"
#include "catalogtemplate.h"

/**
  *@author Klaas Freitag
  */

class TemplKatalog;
class QPixmap;
class DocPosition;
class Katalog;
class CatalogChapter;

class KRAFTCAT_EXPORT KatalogListView : public QTreeWidget
{
  Q_OBJECT
public:
  KatalogListView( QWidget *parent = 0 );
  ~KatalogListView();

  virtual void addCatalogDisplay( const QString& );
  virtual void* currentItemData();
  virtual void* itemData( QTreeWidgetItem* );

  CatalogTemplateList selectedTemplates();

  bool isChapter(QTreeWidgetItem*);
  bool isRoot(QTreeWidgetItem*);
  QString selectedCatalogChapter();

  virtual void setupChapters();

  QMenu *contextMenu();
  // virtual DocPosition itemToDocPosition( QListViewItem *it = 0 ) = 0;

  // Save the header state of the tree view
  virtual void saveState() = 0;
  void updateChapterSort(int catChapterId);

signals:
  void templateHoovered( CatalogTemplate* );
  void sequenceUpdateProgress( int );
  void sequenceUpdateMaximum( int );

public slots:

  virtual void setCheckboxes( bool );

  virtual void slotFreshupItem( QTreeWidgetItem*, void*, bool remChildren = false );
  virtual void slotCreateNewChapter();
  virtual void slotEditCurrentChapter();
  virtual void slotRemoveCurrentChapter();

  virtual void contextMenuEvent( QContextMenuEvent* );
  virtual void slotRedraw();

  virtual void setSelectFromMode();

  virtual void removeTemplateItem( QTreeWidgetItem* );

protected slots:
  virtual void slotItemEntered( QTreeWidgetItem*, int);
  // run an update of the sort key in a chapter.
  void updateSort(QTreeWidgetItem *chapter);

protected:
  virtual void startUpdateItemSequence() = 0;
  virtual void updateItemSequence(QTreeWidgetItem *item, int seqNo) = 0;
  virtual void endUpdateItemSequence();

  virtual Katalog* catalog();
  void dropEvent( QDropEvent* );

  bool             mCheckboxes;
  QTreeWidgetItem* tryAddingCatalogChapter( const CatalogChapter& );

  QTreeWidgetItem *m_root;
  QHash<QTreeWidgetItem*, void*> m_dataDict;
  QHash<int, QTreeWidgetItem*> mChapterDict;
  QString m_catalogName;
  QStringList mOpenChapters;
  QMenu *mMenu;
  QFont mChapterFont;
  QSqlQuery *_query;
};

#endif
