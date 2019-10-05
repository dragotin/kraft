/***************************************************************************
                              portalview.h
                           -------------------
    begin                : 2004-05-09
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

#ifndef _PORTALVIEW_H
#define _PORTALVIEW_H

#include <QUrl>
#include <QStackedWidget>
#include <QListWidget>

// include files
#include "docguardedptr.h"

/**
 *
 */
class QWidget;
class AllDocsView;
class dbID;
class PortalHtmlView;
class ArchDocDigest;

class PortalView : public QWidget
{
  Q_OBJECT

public:
  PortalView (QWidget *parent=0, const char *name=0 );
  ~PortalView();

  AllDocsView* docDigestView() { return _allDocsView; }
  void systemInitError( const QString& );
  QString ptag( const QString&,  const QString& c = QString() ) const;

public slots:
  void slotBuildView();
  void fillCatalogDetails();
  void fillSystemDetails();
  void displaySystemsTab();

protected slots:
  void slotCreateDocument();

private slots:
  void changePage(QListWidgetItem *current);

signals:
  void openKatalog( const QString& );
  void katalogToXML( const QString& );
  void createDocument();
  void openDocument( const QString& );
  void copyDocument( const QString& );
  void viewDocument( const QString& );
  void openArchivedDocument( const ArchDocDigest& );
  void documentSelected( const QString& );
  void archivedDocSelected( const ArchDocDigest& );

private:
  QString printKatLine( const QString&, int ) const;
  void createIcons();
  QWidget *katalogDetails();
  QWidget *systemDetails();
  QWidget *documentDigests();

  QString systemView( const QString& ) const;

  AllDocsView   *_allDocsView;
  PortalHtmlView  *mCatalogBrowser;
  PortalHtmlView  *mSystemBrowser;

  QListWidget     *_contentsWidget;
  QStackedWidget  *_pagesWidget;

  int _sysPageIndx;
};

#endif

/* END */

