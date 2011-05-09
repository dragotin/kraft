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
#include <QTableView>

#include "docdigest.h"
#include "docguardedptr.h"
#include "models/documentproxymodels.h"

class KMenu;
class QPushButton;
class dbID;
class ArchDocDigest;
class QContextMenuEvent;
class QToolBox;
class DocDigestDetailView;
class KTreeViewSearchLine;

class DocDigestView : public QWidget
{
  Q_OBJECT

public:
  DocDigestView( QWidget *parent = 0 );
  ~DocDigestView();

  int currentDocumentRow() const;

  QString currentDocumentId( ) const;
  QList<KMenu*> contextMenus();

public slots:

  void slotBuildView();
  void slotUpdateView();

protected:
  void contextMenuEvent( QContextMenuEvent* );
  void initializeTreeWidgets();

protected slots:
  void slotDocOpenRequest( QModelIndex );
  void slotCurrentChanged( QModelIndex, QModelIndex );
  void slotCurrentChangedToolbox ( int index );
  void slotOpenLastPrinted();

signals:
  void createDocument();
  void openDocument( const QString& );
  void viewDocument( const QString& );
  void copyDocument( const QString& );
  void docSelected( const QString& );
  void openArchivedDocument( const ArchDocDigest& );

private:

  QTableView *mAllView;
  QTableView *mLatestView;
  QTreeView *mTimeView;

  DocDigestDetailView *mLatestViewDetails;
  DocDigestDetailView *mAllViewDetails;
  DocDigestDetailView *mTimeLineViewDetails;

  QModelIndex mCurrentlySelected;

  DocumentFilterModel *mAllDocumentsModel;
  DocumentFilterModel *mLatestDocModel;
  TimelineModel       *mTimelineModel;

  QList<QAbstractItemView*> mTreeViewList;

  KTreeViewSearchLine *mFilterHeader;

  KMenu *mTimelineMenu;
  KMenu *mAllMenu;
  KMenu *mLatestMenu;

  int                    mOldToolboxIndex;
  QToolBox               *mToolBox;
  QPushButton            *mNewDocButton;
  QVector<QAbstractItemView*>    mTreeViewIndex;
  ArchDocDigest          mLatestArchivedDigest;
};

#endif
