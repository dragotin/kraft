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

#include <QWidget>
#include <QMap>
#include <QTreeWidgetItem>

#include <kabc/addressbook.h>

#include "docdigest.h"
#include "docguardedptr.h"

class FilterHeader;
class KMenu;
class QPushButton;
class dbID;
class ArchDocDigest;
class QContextMenuEvent;
class QToolBox;

class DocDigestView : public QWidget
{
  Q_OBJECT

public:
  DocDigestView( QWidget *parent = 0 );
  ~DocDigestView();

  void addItems( QTreeWidget*, DocDigestList, KABC::AddressBook*, QTreeWidgetItem *chapParent = 0 );
  void addArchivedItem( dbID docID, dbID archID);

  QString currentDocumentId();

  QList<KMenu*> contextMenus();

  ArchDocDigest currentArchiveDoc() const;

public slots:
  void slotNewDoc( DocGuardedPtr );
  void slotUpdateDoc( DocGuardedPtr );
  void slotDocOpenRequest( QTreeWidgetItem*, int );
  void slotBuildView();

protected:
  void contextMenuEvent( QContextMenuEvent* );
  QList<QTreeWidget *> initializeTreeWidgets();

protected slots:
  // void slotOpenCurrentDoc();
  void slotCurrentChanged( QTreeWidgetItem*, QTreeWidgetItem* = 0 );
  void slotCurrentChangedToolbox ( int index );
  void setupListViewItemFromDoc( DocGuardedPtr , QTreeWidgetItem* );
  QTreeWidgetItem *addDocToParent( DocGuardedPtr, QTreeWidget*, QTreeWidgetItem* = 0);

signals:
  void createDocument();
  void openDocument( const QString& );
  void viewDocument( const QString& );
  void copyDocument( const QString& );
  void openArchivedDocument( const ArchDocDigest& );
  void docSelected( const QString& );
  void archivedDocSelected( const ArchDocDigest& );
  //void currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*);

private:

  QTreeWidget *mAllView;
  QTreeWidget *mLatestView;
  QTreeWidget *mTimeView;

  FilterHeader *mFilterHeader;
  KMenu *mTimelineMenu;
  KMenu *mAllMenu;
  KMenu *mLatestMenu;

  QToolBox    *mToolBox;
  QPushButton *mNewDocButton;
  QMap<QTreeWidgetItem*, QString> mDocIdDict;
  QMap<QTreeWidgetItem*, ArchDocDigest> mArchIdDict;
};

#endif
