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

#include "docdigest.h"
#include "docguardedptr.h"
#include "models/documentproxymodels.h"

class FilterHeader;
class KMenu;
class QPushButton;
class dbID;
class ArchDocDigest;
class QContextMenuEvent;
class QToolBox;
class HtmlView;

class DocDigestView : public QWidget
{
  Q_OBJECT

public:
  DocDigestView( QWidget *parent = 0 );
  ~DocDigestView();

  int currentDocumentRow() const;
  int currentArchivedRow() const;
  QString currentDocumentId( ) const;
  QList<KMenu*> contextMenus();

public slots:

  void slotBuildView();

protected:
  void contextMenuEvent( QContextMenuEvent* );
  QList<QTreeView *> initializeTreeWidgets();

protected slots:
  void slotDocOpenRequest( QModelIndex );
  void slotCurrentChanged( QModelIndex, QModelIndex );
  void slotCurrentChangedToolbox ( int index );
  void slotShowDocDetails( DocDigest );

signals:
  void createDocument();
  void openDocument( const QString& );
  void viewDocument( const QString& );
  void copyDocument( const QString& );
  void docSelected( const QString& );
  void openArchivedDocument( const QString& , const QString& );
  void archivedDocSelected( const QString&, const QString&  );

private:

  QTreeView *mAllView;
  QTreeView *mLatestView;
  QTreeView *mTimeView;

  QModelIndex mCurrentlySelected;

  DocumentFilterModel *mAllDocumentsModel;
  DocumentFilterModel *mLatestDocModel;
  TimelineModel *mTimelineModel;

  QList<QTreeView *> treeviewlist;

  FilterHeader *mFilterHeader;
  KMenu *mTimelineMenu;
  KMenu *mAllMenu;
  KMenu *mLatestMenu;

  HtmlView    *mShowDocDetailsView;
  QToolBox    *mToolBox;
  QPushButton *mNewDocButton;
  QMap<QTreeWidgetItem*, QString> mDocIdDict;
  QMap<QTreeWidgetItem*, ArchDocDigest> mArchIdDict;
  QString      mTemplFile;
};

#endif
