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

#include "docdigest.h"
#include "docguardedptr.h"

class KListView;
class KListViewItem;
class FilterHeader;
class QPopupMenu;
class QPushButton;
class dbID;
class ArchDocDigest;

class DocDigestView : public QWidget
{
  Q_OBJECT

public:
  DocDigestView( QWidget *parent );
  ~DocDigestView();

  KListViewItem* addChapter( const QString&, DocDigestList, KListViewItem *chapParent = 0 );

  QString currentDocumentId();
  KListView *listview() {
    return mListView;
  }

  QPopupMenu *contextMenu() {
    return mContextMenu;
  }
  ArchDocDigest currentArchiveDoc() const;

public slots:
  void slotNewDoc( DocGuardedPtr );
  void slotUpdateDoc( DocGuardedPtr );
  void slotDocOpenRequest( QListViewItem * );
  void slotRMB( QListViewItem*, const QPoint&, int );
  void slotBuildView();

protected slots:
  void slotOpenCurrentDoc();
  void slotCurrentChanged( QListViewItem* );
  void setupListViewItemFromDoc( DocGuardedPtr , QListViewItem* );
signals:
  void createDocument();
  void openDocument( const QString& );
  void openArchivedDocument( const ArchDocDigest& );
  void docSelected( const QString& );
  void archivedDocSelected( const ArchDocDigest& );
  void printDocument( const QString& );
private:
  KListView *mListView;
  KListViewItem *mAllDocsParent;
  KListViewItem *mLatestDocsParent;
  KListViewItem *mTimeLineParent;

  FilterHeader *mFilterHeader;
  QPopupMenu *mContextMenu;
  QPushButton *mNewDocButton;
  QMap<QListViewItem*, QString> mDocIdDict;
  QMap<QListViewItem*, ArchDocDigest>    mArchIdDict;
};

#endif
