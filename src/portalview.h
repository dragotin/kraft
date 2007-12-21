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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kurl.h>

// include files
#include <kjanuswidget.h>

#include "docguardedptr.h"

/**
 *
 */
class QWidget;
class QVBox;
class QListViewItem;
class DocDigestView;
class dbID;
class PortalHtmlView;
class ArchDocDigest;

class PortalView : public KJanusWidget
{
    Q_OBJECT

public:
    PortalView (QWidget *parent=0, const char *name=0, int face=Plain);
    ~PortalView();
    DocDigestView* docDigestView() { return mDocDigestView; }
    void systemInitError( const QString& );
  QString ptag( const QString&,  const QString& c = QString() ) const;
public slots:
    void slotBuildView();
    void fillCatalogDetails();
    void fillSystemDetails();

    void slotDocumentCreated( DocGuardedPtr );
    void slotDocumentUpdate( DocGuardedPtr );

protected slots:
    void slUrlClicked( const QString& );
    void slotDigestItemSelected( QListViewItem* );
    void slotCreateDocument();

signals:
    void openKatalog( const QString& );
    void deleteKatalog( const QString& );
    void katalogToXML( const QString& );
    void createDocument();
    void openDocument( const QString& );
    void openArchivedDocument( const ArchDocDigest& );
    void documentSelected( const QString& );
    void archivedDocSelected( const ArchDocDigest& );
    void printDocument( const QString& );

private:
    QString printKatLine( const QString&, int ) const;
    void katalogDetails( QWidget *);
    void archiveDetails( QWidget * );
    void systemDetails( QWidget *);
    void documentDigests( QWidget * );
  QString systemViewHeader() const;

    QVBox *m_docBox;
    QVBox *m_katalogBox;
    QVBox *m_sysBox;
    QVBox *mArchiveBox;
    DocDigestView *mDocDigestView;
    PortalHtmlView  *mCatalogBrowser;
    PortalHtmlView  *mSystemBrowser;

    KListViewItem *mAllDocsParent;
    KListViewItem *mLatestDocsParent;
    KListViewItem *mTimeLineParent;

  int mDocDigestIndex;
  int mCatalogIndex;
  int mSystemIndex;
};

#endif

/* END */

