/***************************************************************************
                          docdigestview.h  -
                             -------------------
    begin                : Wed Mar 15 2006
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
#ifndef DOCDIGESTVIEW_H
#define DOCDIGESTVIEW_H

#include <qwidget.h>
#include <qmap.h>
#include <QTreeWidgetItem>
//Added by qt3to4:
#include <Q3PopupMenu>

#include "docdigest.h"
#include "docguardedptr.h"

class FilterHeader;
class KMenu;
class QPushButton;
class dbID;
class ArchDocDigest;
class QContextMenuEvent;

class DocDigestView : public QWidget
{
  Q_OBJECT

public:
  DocDigestView( QWidget *parent = 0 );
  ~DocDigestView();

  QTreeWidgetItem* addChapter( const QString&, DocDigestList, QTreeWidgetItem *chapParent = 0 );

  QString currentDocumentId();
  QTreeWidget *listview() {
    return mListView;
  }

  KMenu* contextMenu();
  ArchDocDigest currentArchiveDoc() const;

public slots:
  void slotNewDoc( DocGuardedPtr );
  void slotUpdateDoc( DocGuardedPtr );
  void slotDocOpenRequest( QTreeWidgetItem*, int );
  void slotBuildView();

protected:
  void contextMenuEvent( QContextMenuEvent* );

protected slots:
  // void slotOpenCurrentDoc();
  void slotCurrentChanged( QTreeWidgetItem*, QTreeWidgetItem* );
  void setupListViewItemFromDoc( DocGuardedPtr , QTreeWidgetItem* );

signals:
  void createDocument();
  void openDocument( const QString& );
  void viewDocument( const QString& );
  void copyDocument( const QString& );
  void openArchivedDocument( const ArchDocDigest& );
  void docSelected( const QString& );
  void archivedDocSelected( const ArchDocDigest& );

private:
  QTreeWidget *mListView;
  QTreeWidgetItem *mAllDocsParent;
  QTreeWidgetItem *mLatestDocsParent;
  QTreeWidgetItem *mTimeLineParent;

  FilterHeader *mFilterHeader;
  KMenu *mContextMenu;
  QPushButton *mNewDocButton;
  QMap<QTreeWidgetItem*, QString> mDocIdDict;
  QMap<QTreeWidgetItem*, ArchDocDigest>    mArchIdDict;
};

#endif
