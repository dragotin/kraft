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
#include "zeitcalcpart.h"

KatalogListView::KatalogListView( QWidget *parent, bool ) : QTreeWidget(parent),
    m_root(0),
    mMenu(0)
{
    // setItemMargin(4);
    setSelectionMode(QAbstractItemView::SingleSelection );

    setRootIsDecorated(false);
    // setSorting(-1);
    mMenu = new KMenu( this );
    mMenu->addTitle( i18n("Template Catalog") );

    connect( this, SIGNAL( contextMenu( KTreeView*, QTreeWidgetItem *, const QPoint& ) ),
               this, SLOT( slotRMB( KTreeView*, QTreeWidgetItem *, const QPoint & ) ) );
}

void KatalogListView::slotRMB( QTreeWidget*, QTreeWidgetItem* item, const QPoint& point )
{
  if( ! item ) return;

  // fill the document list with a list of the open docs
  mMenu->popup( point );
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
    const QStringList chapters = cat->getKatalogChapters( true );
    kDebug() << "Have count of chapters: " << chapters.size() << endl;
    QPixmap icon = getCatalogIcon();

    for ( QStringList::ConstIterator it = chapters.end(); it != chapters.begin();  ) {
      --it;
      QString chapter = *it;

      kDebug() << "Creating katalog chapter item for " << chapter << endl;
      QTreeWidgetItem *katItem = new QTreeWidgetItem( m_root, QStringList( chapter) );
      katItem->setText( 4, QString::number( cat->chapterID( chapter ) ) );
      m_catalogDict.insert( cat->chapterID(chapter), katItem );

      katItem->setIcon( 0, icon );
      if ( mOpenChapters.contains( chapter ) ) {
        katItem->setExpanded( true );
      }
    }
}

QTreeWidgetItem *KatalogListView::chapterItem( const QString& chapName )
{
  Katalog *kat = catalog();
    int chapID = kat->chapterID(chapName);

    return m_catalogDict[chapID];
}

QPixmap KatalogListView::getCatalogIcon()
{
    return SmallIcon("contents");
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

