/***************************************************************************
                          brunskatalogview.cpp
                             -------------------
    begin                : 2005-07-26
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

#include <QLayout>
#include <QLabel>
#include <QSplitter>

#include <kdebug.h>
#include <klocale.h>

#include "katalogman.h"
#include "materialkatalogview.h"
#include "materialkataloglistview.h"
#include "stockmaterial.h"
#include "matkatalog.h"
#include "materialtempldialog.h"

const QString MaterialKatalogView::MaterialCatalogName( "Material" );


MaterialKatalogView::MaterialKatalogView()
 : KatalogView(),
   m_materialListView(0),
   m_details(0)
{

}


MaterialKatalogView::~MaterialKatalogView()
{
}

void MaterialKatalogView::createCentralWidget( QBoxLayout *box, QWidget *w )
{
    m_materialListView = new MaterialKatalogListView( w );
    box->addWidget( m_materialListView );
    KatalogView::createCentralWidget( box, w );
}

Katalog* MaterialKatalogView::getKatalog( const QString& name )
{
    kDebug() << "GetKatalog of material!" << endl;
    Katalog *k = KatalogMan::self()->getKatalog( name );
    if( ! k ) {
        k = new MatKatalog( name );
        KatalogMan::self()->registerKatalog( k );
    }
    return k;
}

void MaterialKatalogView::slEditTemplate()
{
  MaterialKatalogListView *listview = static_cast<MaterialKatalogListView*>(getListView());

  kDebug() << "Editing the material" << endl;

  if( listview )
  {
    StockMaterial *currTempl = static_cast<StockMaterial*> ( listview->currentItemData() );
    if( currTempl ) {
      QTreeWidgetItem *item = (QTreeWidgetItem*) listview->currentItem();
      openDialog( item, currTempl, false );
    }
  }
}

void MaterialKatalogView::slNewTemplate()
{
  KatalogListView *listview = getListView();
  if( !listview ) return;
  MaterialKatalogListView *matListView = static_cast<MaterialKatalogListView*>(listview);

  StockMaterial *newMat = new StockMaterial();
  newMat->setName( i18n( "<new material>" ) );
  QTreeWidgetItem *parentItem = static_cast<QTreeWidgetItem*>( listview->currentItem() );
  if ( parentItem ) {
    if ( ! ( matListView->isRoot( parentItem ) || matListView->isChapter( parentItem ) ) ) {
      parentItem = ( QTreeWidgetItem* ) parentItem->parent();
    }
  }

  if( parentItem ) {
    // try to find out which catalog is open/current
    CatalogChapter *chap = static_cast<CatalogChapter*>(listview->itemData( parentItem ));
    newMat->setChapter( chap->id().toInt() );
  }

  mNewItem = matListView->addMaterialToView( parentItem, newMat );
  openDialog( mNewItem, newMat, true );

}

void MaterialKatalogView::slDeleteTemplate()
{

}

void MaterialKatalogView::openDialog( QTreeWidgetItem *listitem, StockMaterial *tmpl, bool isNew )
{
  mDialog = new MaterialTemplDialog( this );
  mNewItem = listitem;

  listitem->setSelected( true );
  // listitem->ensureItemVisible( true );

  connect( mDialog, SIGNAL( editAccepted( StockMaterial* ) ),
           this, SLOT( slotEditOk( StockMaterial* ) ) );
  connect( mDialog, SIGNAL( editRejected( ) ),
           this, SLOT( slotEditRejected() ) );

  mDialog->setMaterial( tmpl, MaterialCatalogName, isNew );
  mDialog->show();
}

void MaterialKatalogView::slotEditRejected()
{
  if ( mNewItem ) {
    delete mNewItem;
    mNewItem = 0;
  }
}

void MaterialKatalogView::slotEditOk( StockMaterial *mat )
{
  KatalogListView *listview = getListView();
  if( !listview ) return;
  MaterialKatalogListView *templListView = static_cast<MaterialKatalogListView*>(listview);
  kDebug() << "****** slotEditOk for Material" << endl;

  if( mDialog ) {
    MatKatalog *k = static_cast<MatKatalog*>( getKatalog( MaterialCatalogName ) );
    if ( mDialog->templateIsNew() ) {
      KLocale *locale = 0;
      if ( k ) {
        k->addNewMaterial( mat );
        locale = k->locale();
      }
      if( mNewItem ) {
        mNewItem->setSelected( true );
        templListView->slFreshupItem( mNewItem, mat, locale );
        // templListView->ensureItemVisible( mNewItem );
      }
    }
  }
  mNewItem = 0;
}

#include "materialkatalogview.moc"
