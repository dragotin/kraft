/***************************************************************************
                          templkatalogview.cpp
                             -------------------
    begin                : 2005-07-09
    copyright            : (C) 2005 by Klaas Freitag
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

// include files for QT
#include <QDir>
#include <QPainter>
#include <QLayout>
#include <QDebug>
#include <QMessageBox>

// application specific includes
#include "katalogview.h"
#include "templkatalogview.h"
#include "floskeltemplate.h"
#include "kataloglistview.h"
#include "flostempldialog.h"
#include "templkatalog.h"
#include "templkataloglistview.h"
#include "katalogman.h"
#include "documentman.h"
#include "kraftsettings.h"

#define ID_STATUS_MSG 1

TemplKatalogView::TemplKatalogView(QWidget* parent, const char* name)
    : KatalogView(parent, name),
      m_flosDialog(nullptr),
      m_listview(nullptr)
{

}

TemplKatalogView::~TemplKatalogView()
{
    slotSaveState();
    delete m_flosDialog;
}

Katalog* TemplKatalogView::getKatalog( const QString& name )
{
    Katalog *k = KatalogMan::self()->getKatalog( name );
    if( ! k ) {
        k = new TemplKatalog( name );
        KatalogMan::self()->registerKatalog( k );
    }
    return k;
}

/////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATION
/////////////////////////////////////////////////////////////////////


void TemplKatalogView::slEditTemplate()
{
  TemplKatalogListView* listview = static_cast<TemplKatalogListView*>(getListView());

  if( listview )
  {
    QTreeWidgetItem *item = listview->currentItem();
    if( listview->isChapter(item) ) {
      // check if the chapter is empty. If so, switch to slNewTempalte()
      // if there others, open the chapter.
      if( !listview->isRoot( item ) && item->childCount() == 0 ) {
        slNewTemplate();
      } else {
        // do nothing.
      }
    } else {
      // the clicked item is not a chapter
      FloskelTemplate *currTempl = static_cast<FloskelTemplate*> (listview->currentItemData());
      if( currTempl ) {
        QTreeWidgetItem *item = (QTreeWidgetItem*) listview->currentItem();
        openDialog( item, currTempl, false );
      }
    }
  }
}

void TemplKatalogView::slNewTemplate()
{
  KatalogListView *listView = getListView();
  if( !listView ) return;

  // create new template object
  FloskelTemplate *flosTempl = new FloskelTemplate();
  flosTempl->setText( i18n( "<new template>" ) );

  // find the corresponding parent (==chapter) item
  QTreeWidgetItem *parentItem = static_cast<QTreeWidgetItem*>(listView->currentItem());
  if( parentItem )
  {
    // if it is not a chapter nor root, take the parent
    if( ! (listView->isRoot(parentItem) || listView->isChapter(parentItem)) )
    {
      parentItem = (QTreeWidgetItem*) parentItem->parent();
    }
  }

  if( parentItem ) {
    // try to find out which catalog is open/current
    CatalogChapter *chap = static_cast<CatalogChapter*>( listView->itemData( parentItem ) );
    if( chap ) {
      flosTempl->setChapterId( chap->id().toInt(), true );
    }
  }

  TemplKatalogListView *templListView = static_cast<TemplKatalogListView*>(listView);
  QTreeWidgetItem *item = templListView->addFlosTemplate(parentItem, flosTempl);
  listView->scrollToItem( item );
  listView->setCurrentItem( item );
  openDialog( item, flosTempl, true );
}

void TemplKatalogView::slDeleteTemplate()
{
  // qDebug () << "delete template hit";
  TemplKatalogListView* listview = static_cast<TemplKatalogListView*>(getListView());
  if( listview )
  {
    FloskelTemplate *currTempl = static_cast<FloskelTemplate*> (listview->currentItemData());
    if( currTempl ) {
      int id = currTempl->getTemplID();
      QMessageBox msgBox;
      msgBox.setText(i18n( "Do you really want to delete the template from the catalog?" ));

      msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
      msgBox.setDefaultButton(QMessageBox::Ok);
      int result = msgBox.exec();

      if( result == QMessageBox::Ok)
      {
        // qDebug () << "Delete item with id " << id;
        TemplKatalog *k = static_cast<TemplKatalog*>( getKatalog( m_katalogName ) );

        if( k ) {
          k->deleteTemplate( id );
          listview->removeTemplateItem( listview->currentItem());
        }
      }
    }
  }
}

bool TemplKatalogView::currentItemToDocPosition( DocPosition& pos )
{
  TemplKatalogListView* listview = static_cast<TemplKatalogListView*>(getListView());
  bool res = false;
  if( listview )
  {
    FloskelTemplate *currTempl = static_cast<FloskelTemplate*> (listview->currentItemData());
    if( currTempl ) {
      // create a new position and offer it to the document manager
      pos.setText( currTempl->getText() );
      pos.setUnit( currTempl->unit() );
      pos.setUnitPrice( currTempl->unitPrice() );
      pos.setAmount( 1.0 );
      res = true;
    }
  }
  return res;
}

CalcPartList TemplKatalogView::currentItemsCalcParts()
{
  TemplKatalogListView* listview = static_cast<TemplKatalogListView*>(getListView());
  CalcPartList cpList;

  if( listview )
  {
    FloskelTemplate *currTempl = static_cast<FloskelTemplate*> (listview->currentItemData());
    if ( currTempl ) {
      cpList = currTempl->getCalcPartsList();
    }
  }
  return cpList;
}


void TemplKatalogView::openDialog( QTreeWidgetItem *listitem, FloskelTemplate *tmpl, bool isNew )
{
    if( ! m_flosDialog )
    {
        m_flosDialog = new FlosTemplDialog(this, false);
        connect( m_flosDialog, SIGNAL(editAccepted( FloskelTemplate* )),
                 this, SLOT( slEditOk(FloskelTemplate*)));
        connect( m_flosDialog, SIGNAL(editRejected( )),
                 this, SLOT( slEditRejected()));
    }
    m_flosDialog->setTemplate( tmpl, m_katalogName, isNew );
    m_editListViewItem = listitem;
    m_flosDialog->refreshPrices();
    m_flosDialog->show();
}

void TemplKatalogView::slEditOk(FloskelTemplate* templ)
{
    // the dialog saves the template in its accept-slot.
    KatalogListView *listview = getListView();
    if( !listview ) return;
    TemplKatalogListView *templListView = static_cast<TemplKatalogListView*>(listview);

    if(m_flosDialog ){
      if ( m_flosDialog->templateIsNew() ) {
         TemplKatalog *k = static_cast<TemplKatalog*>( getKatalog( m_katalogName ) );
         if ( k ) k->addNewTemplate( templ );
      }
    }

    if( templListView && m_editListViewItem ) {
      // qDebug () << "Edit was ok, refreshing item in list " << m_editListViewItem << endl;
      templListView->setCurrentItem( m_editListViewItem );
      templListView->slFreshupItem( m_editListViewItem, templ, true );
      templListView->scrollToItem( m_editListViewItem );
    }

    m_editListViewItem = nullptr;
}

void TemplKatalogView::slEditRejected()
{
  // qDebug () << "Rejecting Edit!";
  if ( m_editListViewItem ) {
    delete m_editListViewItem;
    m_editListViewItem = nullptr;
  }
}

void TemplKatalogView::createCentralWidget(QBoxLayout*box, QWidget *w)
{
    // qDebug () << "Creating new Listview" << endl;
    m_listview = new TemplKatalogListView( w );
    box->addWidget(m_listview);

    KatalogView::createCentralWidget( box, w );
}

void TemplKatalogView::saveWindowState( const QByteArray& arr )
{
    KraftSettings::self()->setTemplateCatViewState(arr);
}

QByteArray TemplKatalogView::windowState()
{
    const QByteArray re = QByteArray::fromBase64( KraftSettings::self()->templateCatViewState().toAscii() );

    return re;
}

void TemplKatalogView::saveWindowGeo( const QByteArray& arr )
{
    KraftSettings::self()->setTemplateCatViewGeo( QString::fromAscii(arr) );
}

QByteArray TemplKatalogView::windowGeo()
{
    const QByteArray re = QByteArray::fromBase64( KraftSettings::self()->templateCatViewGeo().toAscii() );

    return re;
}
