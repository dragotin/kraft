/***************************************************************************
                          kataloglistview.cpp  -
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

#include <qstringlist.h>
#include <qheader.h>
#include <qpixmap.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kpopupmenu.h>

#include "kraftglobals.h"
#include "katalog.h"
#include "katalogman.h"
#include "kataloglistview.h"
#include "materialcalcpart.h"
#include "stockmaterial.h"
#include "templkatalog.h"
#include "zeitcalcpart.h"

KatalogListView::KatalogListView( QWidget *parent, bool ) : KListView(parent),
    m_root(0),
    mMenu(0)
{
    setItemMargin(4);
    setSelectionMode(QListView::Single);
    setRootIsDecorated(false);
    setSorting(-1);
    mMenu = new KPopupMenu( this );
    mMenu->insertTitle( i18n("Template Catalog") );

    connect( this, SIGNAL( contextMenu( KListView*, QListViewItem *, const QPoint& ) ),
               this, SLOT( slotRMB( KListView*, QListViewItem *, const QPoint & ) ) );
}

void KatalogListView::slotRMB( KListView*, QListViewItem* item, const QPoint& point )
{
  if( ! item ) return;

  // fill the document list with a list of the open docs
  mMenu->popup( point );
}

KatalogListView::~KatalogListView()
{

}

QPopupMenu *KatalogListView::contextMenu()
{
  return mMenu; // ->contextMenu();
}

void KatalogListView::addCatalogDisplay( const QString& name)
{
    m_catalogName = name;
}

void KatalogListView::setupChapters()
{
    Katalog *catalog = KatalogMan::self()->getKatalog(m_catalogName);
    if( ! catalog ) return;

    if( m_root ) {
      delete m_root;
      m_catalogDict.clear();
    }

    kdDebug() << "Creating root item!" <<  endl;
    m_root = new KListViewItem(this, catalog->getName());
    m_root->setPixmap(0, SmallIcon("kraft"));
    m_root->setOpen(true);
    repaint();
    const QStringList chapters = catalog->getKatalogChapters( true );
    kdDebug() << "Have count of chapters: " << chapters.size() << endl;
    QPixmap icon = getCatalogIcon();

    for ( QStringList::ConstIterator it = chapters.end(); it != chapters.begin();  ) {
      --it;
      QString chapter = *it;

      kdDebug() << "Creating katalog chapter item for " << chapter << endl;
      KListViewItem *katItem = new KListViewItem( m_root, chapter );
      katItem->setText( 4, QString::number( catalog->chapterID( chapter ) ) );
      m_catalogDict.insert( catalog->chapterID(chapter), katItem );

      katItem->setPixmap( 0, icon );
      if ( mOpenChapters.contains( chapter ) ) {
        katItem->setOpen( true );
      }
    }

    #if 0
    KListViewItem *katItem = new KListViewItem( m_root, catalog->chapterName( dbID() ) );
    katItem->setText( 4, QString::number( -1 ) );
    m_catalogDict.insert( -1, katItem );
    katItem->setPixmap( 0,  SmallIcon( "folder_inbox" ) );
    #endif

}

KListViewItem *KatalogListView::chapterItem( const QString& chapName )
{
    Katalog *kat = KatalogMan::self()->getKatalog(m_catalogName);
    int chapID = kat->chapterID(chapName);

    return m_catalogDict[chapID];
}

QPixmap KatalogListView::getCatalogIcon()
{
    return SmallIcon("contents");
}

void* KatalogListView::itemData( QListViewItem *item )
{
  if ( item ) {
    return m_dataDict[item];
  }
  return 0;
}

void* KatalogListView::currentItemData()
{
  return itemData( currentItem() );
}

bool KatalogListView::isChapter( KListViewItem *item )
{
  QIntDictIterator<KListViewItem> it( m_catalogDict );
  for ( ; it.current(); ++it )
    if ( it.current() == item ) return true;

  return false;
}

bool KatalogListView::isRoot( KListViewItem *item )
{
    return (item == m_root );
}

void KatalogListView::slFreshupItem( QListViewItem*, void *, bool )
{

}

void KatalogListView::slChangeChapter( KListViewItem* item, int newChapter )
{
    if( ! item ) return;

    QListViewItem *parent = item->parent();

    /* Alten parent zuklappen falls noch offen */
    QListViewItem *newChapFolder = m_catalogDict[newChapter];
    if( ! newChapFolder ) {
        kdDebug() << "Can not find new chapter folder for chap id " << newChapter << endl;
    } else {
        setOpen( parent, false );
        setOpen( newChapFolder, true);

        parent->takeItem(item);
        newChapFolder->insertItem(item);

        ensureItemVisible(item);
    }
}

void KatalogListView::slotRedraw()
{
  // remember all currently open chapters
  QIntDictIterator<KListViewItem> it( m_catalogDict );

  for ( ; it.current(); ++it ) {
    if ( it.current()->isOpen() ) {
      kdDebug() << "Adding open Chapter " << it.current()->text( 0 ) << endl;
      mOpenChapters << it.current()->text( 0 );
    }
  }

  clear();
  m_root = 0;
  m_dataDict.clear();
  m_catalogDict.clear();
  addCatalogDisplay( m_catalogName );
  mOpenChapters.clear();
}

#include "kataloglistview.moc"

