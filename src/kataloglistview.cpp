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
#include <QMessageBox>

#include <klocalizedstring.h>

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

KatalogListView::KatalogListView( QWidget *parent ) : QTreeWidget(parent),
  mCheckboxes( false ),
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
    // header()->setResizeMode(QHeaderView::ResizeToContents);

    // custom style
    const QString style = DefaultProvider::self()->getStyleSheet( "templcatalog");
    setStyleSheet( style );

    // Drag and Drop for normal mode, is changed in setSelectFromMode
    setSelectionMode( QAbstractItemView::SingleSelection );
    setDragDropMode( QAbstractItemView::InternalMove );
    setDragEnabled( true );
    setAcceptDrops( true ); // currently only internal moves
    setDropIndicatorShown( true );

    // setSorting(-1);
    mMenu = new QMenu( this );

    mChapterFont = font();
    mChapterFont.setBold( true );

    connect( this, SIGNAL(itemActivated( QTreeWidgetItem*,int )),
             this, SLOT( slotItemEntered( QTreeWidgetItem*, int )));
}

KatalogListView::~KatalogListView()
{

}

QMenu *KatalogListView::contextMenu()
{
  return mMenu;
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

void KatalogListView::setSelectFromMode()
{
  setSelectionMode( QAbstractItemView::SingleSelection /* NoSelection */ ); // FIXME: Allow multiple selections later
  setDragDropMode( QAbstractItemView::NoDragDrop );
  setDragEnabled( false );
  setAcceptDrops( false ); // currently only internal moves
  setDropIndicatorShown( false );
  setCheckboxes( true );
}

void KatalogListView::setupChapters()
{
  Katalog *cat = catalog();
  if( ! cat ) return;

  if( m_root ) {
    delete m_root;
    mChapterDict.clear();
  }

  // qDebug () << "Creating root item!" <<  endl;
  QStringList list;
  list << cat->getName();
  m_root = new QTreeWidgetItem( this, list );
  m_root->setIcon( 0, QIcon("kraft"));
  m_root->setExpanded(true);
  m_root->setFont( 0, mChapterFont );
  m_root->setToolTip( 0, QString() );

  repaint();
  const QList<CatalogChapter> chapters = cat->getKatalogChapters( true );
  // qDebug () << "Have count of chapters: " << chapters.size() << endl;

  QList<CatalogChapter> strayCats;

  foreach( CatalogChapter chapter, chapters ) {
    QTreeWidgetItem *item = tryAddingCatalogChapter( chapter );
    if( ! item ) {
      strayCats.append( chapter );
    } else {
      // qDebug () << "Creating katalog chapter item for " << chapter.name() << endl;
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
        // qDebug () << "Successfully added catalog chapter from strayCats";
      } else {
        newStrayCats.append( chapter );
        // qDebug () << "Failed to add a catalog chapter from stryCats";
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

    katItem->setToolTip( 0, chapter.description() );

    // katItem->setIcon( 0, chapter.icon() );
    katItem->setFont( 0, mChapterFont );
    // Store the parent-ID in the item data
    m_dataDict[katItem] = new CatalogChapter( chapter );

    if ( mOpenChapters.contains( chapter.name() ) ) {
      katItem->setExpanded( true );
    }
  }
  return katItem;
}

CatalogTemplateList KatalogListView::selectedTemplates()
{
  CatalogTemplateList templates;

  if( mCheckboxes ) { // checkbox mode
    // add the checkboxed items.
    QTreeWidgetItemIterator it( this, QTreeWidgetItemIterator::Checked );
    while (*it) {
      QTreeWidgetItem *item = *it;
      if( ! (isChapter( item ) || isRoot(item )) ) { // a template, not a chapter.
        void *data = itemData( item );
        if( data )
          templates.append( static_cast<CatalogTemplate*>( data ));
      }
      item->setCheckState( 0, Qt::Unchecked );
      ++it;
    }
  }

  // if no items were added yet, lets go for the selected ones.
  if( ! mCheckboxes || templates.isEmpty() ) {
    QList<QTreeWidgetItem*> items = selectedItems();

    foreach( QTreeWidgetItem* item, items ) {
      if( isChapter(item) && !isRoot(item) ) {
        // for chapters, the children are lined up.
        int kidCnt = item->childCount();
        for( int i=0; i < kidCnt; i++ ) {
          QTreeWidgetItem *kid = item->child(i);
          if( kid && !isChapter(kid) ) {
            // only add normal templates.
            void *data = itemData(kid);
            if( data ) templates.append( static_cast<CatalogTemplate*>(data));
          }
        }
      }
      if( !(isChapter(item) || isRoot(item))) {
        void *data = itemData( item );
        if( data )
          templates.append( static_cast<CatalogTemplate*>(data) );
      }
    }
  }

  return templates;
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
  return itemData( currentItem() );
}

void KatalogListView::removeTemplateItem( QTreeWidgetItem *item )
{
  if( item == mSortChapterItem )
    mSortChapterItem = 0;

  QHashIterator<int, QTreeWidgetItem*> it( mChapterDict );
  while( it.hasNext() ) {
    it.next();
    if ( it.value() == item ) {
      mChapterDict.remove(it.key());
      break;
    }
  }

  m_dataDict.remove( item );
  delete item;
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

void KatalogListView::setCheckboxes( bool cb )
{
  mCheckboxes = cb;
}

void KatalogListView::slotFreshupItem( QTreeWidgetItem*, void *, bool )
{

}

void KatalogListView::slotEditCurrentChapter()
{
  QTreeWidgetItem *item = currentItem();
  if( ! isChapter( item )) {
    // qDebug () << "Can only edit chapters!" << endl;
    return;
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
        // qDebug () << "Can only remove chapters here!" << endl;
    }

    if( item->childCount() > 0 ) {
        QMessageBox msgBox;
        msgBox.setText(i18n( "A catalog chapter can not be deleted as long it has children." ));
        msgBox.setInformativeText(i18n("Chapter can not be deleted"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        return;

    } else {
        CatalogChapter *chap = static_cast<CatalogChapter*>( itemData( item ) );
        if( chap ) {
            int id = chap->id().toInt();
            if( chap->removeFromDB() ) {
                delete item;
                mChapterDict.remove(id);
                delete chap;
            }
        }
    }
}

void KatalogListView::slotCreateNewChapter()
{
  QTreeWidgetItem *parentItem = currentItem();
  if( ! (isChapter( parentItem ) || isRoot( parentItem ) ) ) {
    // qDebug () << "Not an chapter item selected, returning";
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
    c.setCatalogSetId( catalog()->id() );
    c.setParentId( parentId );
    c.save();
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
      QTreeWidgetItem *parent = itemFromIndex(topIndex);
      if (row == -1) {
          if( isChapter( droppedOnItem ) || isRoot( droppedOnItem )) {
              parent = droppedOnItem;
              parent->insertChild(parent->childCount(), taken.takeFirst());
          }
          // the parent chap has changed.
      } else {
          int r = 1+(dropRow.row() >= 0 ? dropRow.row() : row); // insert behind the row element

        dbID newParentId;

        // The item was dropped on a chapter item or root. That causes a change of the
        // parent chapter.
        if( isChapter( droppedOnItem )|| isRoot( droppedOnItem )) {
          parent = droppedOnItem;
          // the parent id has to be updated for all inserted items
          CatalogChapter *parentChap = static_cast<CatalogChapter*>(itemData(parent));
          if( parentChap ) {
            newParentId = parentChap->id();
          } else {
              newParentId = 0;
          }

          // New chapter is inserted after the other subcatalogs, compute the index
          int cnt = 0;
          while( cnt < parent->childCount() && isChapter(parent->child(cnt))) {
            cnt++;
          }
          r = cnt;
          // the parent chapter has changed.
        } else {
          // the item was dropped on another item. Still the parent might have changed.
          CatalogTemplate *tmpl = static_cast<CatalogTemplate*>(itemData(droppedOnItem));
          newParentId = tmpl->chapterId();
        }

        if( parent ) {
          QTreeWidgetItem *dropItem = taken.takeFirst();
          if( newParentId.isOk() ) {
            if( isChapter( dropItem ) )   {
              CatalogChapter* chapDrop = static_cast<CatalogChapter*>(itemData(dropItem));
              chapDrop->reparent( newParentId );
            } else if( isRoot( dropItem )) {
              CatalogChapter* chapDrop = static_cast<CatalogChapter*>(itemData(dropItem));
              chapDrop->reparent( 0 );
            } else {
              // ordinary template, set a new parent chapter
              CatalogTemplate *tmpl = static_cast<CatalogTemplate*>(itemData(dropItem));
              if( tmpl && tmpl->chapterId() != newParentId ) {
                tmpl->setChapterId( newParentId, true );
              }
            }
          }
          parent->insertChild( qMin(r, parent->childCount()), dropItem );
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


void KatalogListView::slotUpdateSequence()
{
  // check the detail implementations in inherited classes
  // qDebug () << "Updating sequence";
  if( mSortChapterItem )
    mSortChapterItem->setExpanded( true );
  mSortChapterItem = 0;
}

void KatalogListView::slotItemEntered( QTreeWidgetItem *item, int )
{
   if( !item ) return;

   if( isRoot( item )) {
    // qDebug () << "Is a root item ";
   } else if( isChapter(item )) {
    // qDebug () << "Is a chapter item ";
   } else {
     CatalogTemplate *tmpl = static_cast<FloskelTemplate*>(itemData(item));
     // qDebug () << "hoovering this template: " << tmpl;
     emit templateHoovered( tmpl );
   }
}

void KatalogListView::slotRedraw()
{
  // remember all currently open chapters
  QHashIterator<int, QTreeWidgetItem*> it( mChapterDict );

  while( it.hasNext() ) {
    it.next();
    if ( it.value()->isExpanded() ) {
      // qDebug () << "Adding open Chapter " << it.value()->text( 0 ) << endl;
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

