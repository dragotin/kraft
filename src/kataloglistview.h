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

#include <qptrdict.h>
#include <qintdict.h>

#include <klistview.h>
/**
  *@author Klaas Freitag
  */

class TemplKatalog;
class KListViewItem;
class QPixmap;
class QPopupMenu;
class KPopupMenu;
class DocPosition;

class KatalogListView : public KListView  {
    Q_OBJECT
public:
    KatalogListView( QWidget *parent = 0, bool enableCheckboxes = false );
    ~KatalogListView();

    virtual void addCatalogDisplay( const QString& );
    virtual void* currentItemData();
    virtual void* itemData( QListViewItem* );

    bool isChapter(KListViewItem*);
    bool isRoot(KListViewItem*);

    virtual void setupChapters();

    QPopupMenu *contextMenu();
  // virtual DocPosition itemToDocPosition( QListViewItem *it = 0 ) = 0;

public slots:
    virtual void slFreshupItem( QListViewItem*, void*, bool remChildren = false );
    virtual void slChangeChapter( KListViewItem* , int );
    virtual void slotRMB( KListView*, QListViewItem*, const QPoint& );

protected:

    virtual QPixmap getCatalogIcon();

    KListViewItem *chapterItem( const QString& chapName );

    KListViewItem *m_root;
    QPtrDict<void> m_dataDict;
    QIntDict<KListViewItem> m_catalogDict;
    QString m_catalogName;
    QPopupMenu *mMenu;
};

#endif
