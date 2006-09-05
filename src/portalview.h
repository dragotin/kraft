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

/**
 *
 */
class QWidget;
class QVBox;
class QListViewItem;
class DocDigestView;

class PortalView : public KJanusWidget
{
    Q_OBJECT

public:
    PortalView (QWidget *parent=0, const char *name=0, int face=Plain);
    ~PortalView();
    DocDigestView* docDigestView() { return mDocDigestView; }
public slots:
    void slotBuildView();

protected slots:
    void slUrlClicked( const QString& );

signals:
    void openKatalog( const QString& );
    void deleteKatalog( const QString& );
    void katalogToXML( const QString& );
    void createDocument();
    void openDocument( const QString& );
    void documentSelected( const QString& );
    void printDocument( const QString& );

private:
    QString printKatLine( const QString& ) const;
    void katalogDetails( QWidget *);
    void archiveDetails( QWidget * );
    void systemDetails( QWidget *);
    void documentDigests( QWidget * );

    QVBox *m_docBox;
    QVBox *m_katalogBox;
    QVBox *m_sysBox;
    QVBox *mArchiveBox;
    DocDigestView *mDocDigestView;
};

#endif

/* END */

