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

#include <QtCore>
#include <QtGui>

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
#include "defaultprovider.h"
#include "materialcalcpart.h"
#include "stockmaterial.h"
#include "templkatalog.h"
#include "timecalcpart.h"
#include "dbids.h"
#include "catalogchapter.h"
#include "addeditchapterdialog.h"

KatalogListView::KatalogListView( QWidget *parent, bool ) : QTreeWidget(parent),
    m_root(0),
    mSortChapterItem(0),
    mMenu(0)
{
    setSelectionMode(QAbstractItemView::SingleSelection );
    setAlternatingRowColors( true );
    //Initialize common style options
    QPalette palette;
    palette.setColor( QPalette::AlternateBase, QColor("#e0fdd1") );
    setPalette( palette );

    setRootIsDecorated(false);
    setAnimated(true);
    header()->setResizeMode(QHeaderView::ResizeToContents);

    // custom style
    const QString style = DefaultProvider::self()->getStyleSheet( "templcatalog");
    setStyleSheet( style );

    // Drag and Drop
    setSelectionMode( QAbstractItemView::SingleSelection );
    setDragDropMode( QAbstractItemView::InternalMove );
    setDragEnabled( true );
    setAcceptDrops( true ); // currently only internal moves
    setDropIndicatorShown( true );

    // setSorting(-1);
    mMenu = new KMenu( this );
    mMenu->addTitle( i18n("Template Catalog") );

    mChapterFont = font();
    mChapterFont.setBold( true );
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
    mChapterDict.clear();
  }

  kDebug() << "Creating root item!" <<  endl;
  QStringList list;
  list << cat->getName();
  m_root = new QTreeWidgetItem( this, list );
  m_root->setIcon( 0, SmallIcon("kraft"));
  m_root->setExpanded(true);
  m_root->setFont( 0, mChapterFont );

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
    if( mChapterDict.contains( parentChapter ) ) {
      katItem = new QTreeWidgetItem( mChapterDict[parentChapter], QStringList( chapter.name() ) );
      katItem->setToolTip( 0, chapter.description() );
    }
  }
  if( katItem ) {
    mChapterDict.insert( id, katItem );

    if( !chapter.description().isEmpty() )
      katItem->setToolTip( 0, chapter.description() );

    katItem->setIcon( 0, chapter.icon() );
    katItem->setFont( 0, mChapterFont );
    // Store the parent-ID in the item data
    m_dataDict[katItem] = new CatalogChapter( chapter );

    if ( mOpenChapters.contains( chapter.name() ) ) {
      katItem->setExpanded( true );
    }
  }
  return katItem;
}

void* KatalogListView::itemData( QTreeWidgetItem *item )
{
  if ( item && m_dataDict.contains( item ) ) {
    return m_dataDict[item];
  }
  return 0;
}

void* KatalogListView::currentItemData()
{
  if( currentItem() ) {
    return itemData( currentItem() );
  } else {
    return 0;
  }
}

bool KatalogListView::isChapter( QTreeWidgetItem *item )
{
  QHashIterator<int, QTreeWidgetItem*> it( mChapterDict );
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

void KatalogListView::slotEditCurrentChapter()
{
  QTreeWidgetItem *item = currentItem();
  if( ! isChapter( item )) {
    kDebug() << "Can only edit chapters!" << endl;
  }
  CatalogChapter *chap = static_cast<CatalogChapter*>( itemData( item ) );

  AddEditChapterDialog dia( this );
  dia.setEditChapter( *chap );
  if( dia.exec() ) {
    QString name = dia.name();
    QString desc = dia.description();

    if( name != chap->name() || desc != chap->description() ) {
      chap->setName( name );
      chap->setDescription( desc );
      chap->saveNameAndDesc();

      item->setText( 0, name);
      item->setToolTip( 0, desc );
      catalog()->refreshChapterList();
    }
  }
}

void KatalogListView::slotRemoveCurrentChapter()
{
  QTreeWidgetItem *item = currentItem();
  if( ! isChapter( item )) {
    kDebug() << "Can only edit chapters!" << endl;
  }

}

void KatalogListView::slotCreateNewChapter()
{
  QTreeWidgetItem *parentItem = currentItem();
  if( ! (isChapter( parentItem ) || isRoot( parentItem ) ) ) {
    kDebug() << "Not an chapter item selected, returning";
    return;
  }

  AddEditChapterDialog dia( this );
  dbID parentId = 0;

  if( ! isRoot( parentItem ) ) {
    CatalogChapter *parentChapter = static_cast<CatalogChapter*>(currentItemData());
    dia.setParentChapter( *parentChapter );
    parentId = parentChapter->id();
  }

  if( dia.exec() ) {
    QString name = dia.name();
    QString desc = dia.description();

    CatalogChapter c;
    c.setName( name );
    c.setDescription( desc );
    c.setParentId( parentId );
    c.save( catalog()->id() );
    catalog()->refreshChapterList();
    QTreeWidgetItem *newItem = tryAddingCatalogChapter( c );
    if( newItem ) {
      this->scrollToItem( newItem );
      this->setCurrentItem( newItem );
    }
  }
}

void KatalogListView::dropEvent( QDropEvent *event )
{
  if (event->source() == this && (event->dropAction() == Qt::MoveAction ||
                                  dragDropMode() == QAbstractItemView::InternalMove)) {
    QModelIndex topIndex;
    int col = -1;
    int row = -1;
    QModelIndex dropIndx = indexAt( event->pos() );
    QTreeWidgetItem *droppedOnItem = itemFromIndex( dropIndx );
    if( ! droppedOnItem ) {
      event->ignore();
      return;
    }
    row = dropIndx.row();
    col = dropIndx.column();
    topIndex = dropIndx.parent();

    QList<QModelIndex> idxs = selectedIndexes();
    QList<QPersistentModelIndex> indexes;
    for (int i = 0; i < idxs.count(); i++)
      indexes.append(idxs.at(i));

    if (indexes.contains(topIndex))
      return;

    // When removing items the drop location could shift
    QPersistentModelIndex dropRow = model()->index(row, col, topIndex);

    // Remove the items
    QList<QTreeWidgetItem *> taken;
    for (int i = indexes.count() - 1; i >= 0; --i) {
      QTreeWidgetItem *parent = itemFromIndex(indexes.at(i));
      if (!parent || !parent->parent()) {
        taken.append(takeTopLevelItem(indexes.at(i).row()));
      } else {
        taken.append(parent->parent()->takeChild(indexes.at(i).row()));
      }
    }

    // insert them back in at their new positions
    for (int i = 0; i < indexes.count(); ++i) {
      // Either at a specific point or appended
      if (row == -1) {
        QTreeWidgetItem *parent = itemFromIndex(topIndex);
        if( isChapter( droppedOnItem ) || isRoot( droppedOnItem ))
          parent = droppedOnItem;
          parent->insertChild(parent->childCount(), taken.takeFirst());
      } else {
        int r = 1+(dropRow.row() >= 0 ? dropRow.row() : row); // insert behind the row element

        QTreeWidgetItem *parent = itemFromIndex(topIndex);
        if( isChapter( droppedOnItem )|| isRoot( droppedOnItem )) {
          // Needs to be inserted right after the subcatalogs
          parent = droppedOnItem;
          int cnt = 0;
          while( cnt < parent->childCount() && isChapter(parent->child(cnt))) { cnt++; }
          r = cnt;
        }
        if( parent ) {
          parent->insertChild(qMin(r, parent->childCount()), taken.takeFirst());
          mSortChapterItem = parent;
        }
      }

      event->accept();
      // Don't want QAbstractItemView to delete it because it was "moved" we already did it
      event->setDropAction(Qt::CopyAction);
    }
  }

  QTreeView::dropEvent(event);
  QTimer::singleShot( 0, this, SLOT( slotUpdateSequence() ) );
}


void KatalogListView::slotChangeChapter( QTreeWidgetItem* item, int newChapter )
{
    if( ! item ) return;

    // QTreeWidgetItem *parent = item->parent();

    /* Alten parent zuklappen falls noch offen */
    QTreeWidgetItem *newChapFolder = mChapterDict[newChapter];
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

void KatalogListView::slotUpdateSequence()
{
  kDebug() << "Updating sequence";
  if( mSortChapterItem )
    mSortChapterItem->setExpanded( true );
  mSortChapterItem = 0;
}



void KatalogListView::slotRedraw()
{
  // remember all currently open chapters
  QHashIterator<int, QTreeWidgetItem*> it( mChapterDict );

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
  mChapterDict.clear();
  addCatalogDisplay( m_catalogName );
  mOpenChapters.clear();
}


