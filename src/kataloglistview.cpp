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

#include <QPixmap>
#include <QStringList>
#include <QHeaderView>
#include <QContextMenuEvent>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kmenu.h>

#include "kraftglobals.h"
#include "katalog.h"
#include "katalogman.h"
#include "kataloglistview.h"
#include "materialcalcpart.h"
#include "stockmaterial.h"
#include "templkatalog.h"
#include "timecalcpart.h"
#include "dbids.h"
#include "catalogchapter.h"


KatalogListView::KatalogListView( QWidget *parent, bool ) : QTreeWidget(parent),
    m_root(0),
    mMenu(0)
{
    // setItemMargin(4);
    setSelectionMode(QAbstractItemView::SingleSelection );
    setAlternatingRowColors( true );
    setRootIsDecorated(false);
    setAnimated(true);
    header()->setResizeMode(QHeaderView::ResizeToContents);
    // setSorting(-1);
    mMenu = new KMenu( this );
    mMenu->addTitle( i18n("Template Catalog") );
}

KatalogListView::~KatalogListView()
{

}

KMenu *KatalogListView::contextMenu()
{
  return mMenu; // ->contextMenu();
}

void KatalogListView::addCatalogDisplay( const QString& name)
{
    m_catalogName = name;
}

void KatalogListView::contextMenuEvent( QContextMenuEvent * event )
{
  mMenu->popup( event->globalPos() );
}

Katalog* KatalogListView::catalog()
{
  return KatalogMan::self()->getKatalog( m_catalogName );
}

void KatalogListView::setupChapters()
{
  Katalog *cat = catalog();
  if( ! cat ) return;

  if( m_root ) {
    delete m_root;
    m_catalogDict.clear();
  }

  kDebug() << "Creating root item!" <<  endl;
  QStringList list;
  list << cat->getName();
  m_root = new QTreeWidgetItem( this, list );
  m_root->setIcon( 0, SmallIcon("kraft"));
  m_root->setExpanded(true);
  // m_root->setDragEnabled( false );
  // m_root->setDropEnabled( false );

  repaint();
  const QList<CatalogChapter> chapters = cat->getKatalogChapters( true );
  kDebug() << "Have count of chapters: " << chapters.size() << endl;

  QList<CatalogChapter> strayCats;

  foreach( CatalogChapter chapter, chapters ) {
    QTreeWidgetItem *item = tryAddingCatalogChapter( chapter );
    if( ! item ) {
      strayCats.append( chapter );
    } else {
      kDebug() << "Creating katalog chapter item for " << chapter.name() << endl;
    }
  }

  int oldStrayCatCount = strayCats.count() + 1; // to survive the first while condition
  while( strayCats.count() && strayCats.count() < oldStrayCatCount ) {
    QList<CatalogChapter> newStrayCats;
    oldStrayCatCount = strayCats.count();
    // loop as long as the overall number of straycats goes down in every round
    foreach( CatalogChapter chapter, strayCats ) {
      QTreeWidgetItem *katItem = tryAddingCatalogChapter( chapter );
      if( katItem ) {
        kDebug() << "Sucessfully added catalog chapter from strayCats";
      } else {
        newStrayCats.append( chapter );
        kDebug() << "Failed to add a catalog chapter from stryCats";
      }
    }
    strayCats = newStrayCats;
  }
}


QTreeWidgetItem *KatalogListView::tryAddingCatalogChapter( const CatalogChapter& chapter )
{
  int parentChapter = chapter.parentId().toInt();
  int id = chapter.id().toInt();
  QTreeWidgetItem *katItem = 0;
  if( parentChapter == 0 ) {
    katItem = new QTreeWidgetItem( m_root, QStringList( chapter.name() ) );
  } else {
    if( m_catalogDict.contains( parentChapter ) ) {
      katItem = new QTreeWidgetItem( m_catalogDict[parentChapter], QStringList( chapter.name() ) );
    }
  }
  if( katItem ) {
    m_catalogDict.insert( id, katItem );

    katItem->setIcon( 0, chapter.icon() );
    if ( mOpenChapters.contains( chapter.name() ) ) {
      katItem->setExpanded( true );
    }
  }
  return katItem;
}

QTreeWidgetItem *KatalogListView::chapterItem( const QString& chapName )
{
    Katalog *kat = catalog();
    dbID chapID = kat->chapterID(chapName);

    return m_catalogDict[chapID.toInt()];
}

void* KatalogListView::itemData( QTreeWidgetItem *item )
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

bool KatalogListView::isChapter( QTreeWidgetItem *item )
{
  QHashIterator<int, QTreeWidgetItem*> it( m_catalogDict );
  while( it.hasNext() ) {
    it.next();
    if ( it.value() == item ) return true;
  }

  return false;
}

bool KatalogListView::isRoot( QTreeWidgetItem *item )
{
    return (item == m_root );
}

void KatalogListView::slotFreshupItem( QTreeWidgetItem*, void *, bool )
{

}

void KatalogListView::slotChangeChapter( QTreeWidgetItem* item, int newChapter )
{
    if( ! item ) return;

    // QTreeWidgetItem *parent = item->parent();

    /* Alten parent zuklappen falls noch offen */
    QTreeWidgetItem *newChapFolder = m_catalogDict[newChapter];
    if( ! newChapFolder ) {
        kDebug() << "Can not find new chapter folder for chap id " << newChapter << endl;
    } else {
        item->setExpanded( false );
        newChapFolder->setExpanded( true);

        QTreeWidgetItem *newItem = new QTreeWidgetItem( newChapFolder );

        *newItem = *item;
        delete item;

        scrollToItem(item);
    }
}

void KatalogListView::slotRedraw()
{
  // remember all currently open chapters
  QHashIterator<int, QTreeWidgetItem*> it( m_catalogDict );

  while( it.hasNext() ) {
    it.next();
    if ( it.value()->isExpanded() ) {
      kDebug() << "Adding open Chapter " << it.value()->text( 0 ) << endl;
      mOpenChapters << it.value()->text( 0 );
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

