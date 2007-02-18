/***************************************************************************
             brunskataloglistview  - template katalog listview.
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
#include <qmap.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

#include "matkatalog.h"
#include "stockmaterial.h"
#include "materialkataloglistview.h"
#include "katalogman.h"
#include "docposition.h"
#include "kataloglistview.h"

MaterialKatalogListView::MaterialKatalogListView( QWidget *w, bool enableCheckboxes )
  : KatalogListView( w, enableCheckboxes ),
    mCheckboxes( enableCheckboxes )
{
  addColumn( i18n("Material" ) );
  int p = addColumn( i18n("Pack" ) );
  setColumnAlignment( p, Qt::AlignRight );
  addColumn( i18n("Unit" ) );
  addColumn( i18n("Purchase" ) );
  addColumn( i18n("Sale" ) );
  addColumn( i18n( "Last Modified" ) );
}


MaterialKatalogListView::~MaterialKatalogListView()
{

}

void MaterialKatalogListView::addCatalogDisplay( const QString& katName )
{
  KatalogListView::addCatalogDisplay(katName);
  Katalog *k = KatalogMan::self()->getKatalog( katName );
  MatKatalog *catalog = static_cast<MatKatalog*>( k );
  if( ! catalog ) {
    kdDebug() << "No catalog in listview available!" << endl;
    return;
  }
  kdDebug() << "setting up chapters!" << endl;
  setupChapters();

  const QStringList chapters = catalog->getKatalogChapters();
  for ( QStringList::ConstIterator it = chapters.begin(); it != chapters.end(); ++it ) {
    QString chapter = *it;
    KListViewItem *katItem = chapterItem(chapter);
    kdDebug() << "KatItem is " << katItem << " for chapter " << chapter << endl;

    // hole alle Brunsrecords per Chapter und mach weiter....
    StockMaterialList records = catalog->getRecordList( chapter );

    StockMaterial *mat;
    for ( mat = records.first(); mat; mat = records.next() ) {
      addMaterialToView( katItem,  mat );
    }
  }
}

QListViewItem* MaterialKatalogListView::addMaterialToView( KListViewItem *parent, StockMaterial *mat )
{
  if ( !mat ) return 0;
  if ( !parent ) parent = m_root;

  QListViewItem *recItem;
  if ( ! mCheckboxes ) {
    recItem = new QListViewItem( parent, mat->name() );
  } else {
    recItem = new QCheckListItem( parent, mat->name(), QCheckListItem::CheckBox );
  }
  recItem->setMultiLinesEnabled( true );
  slFreshupItem( recItem,  mat );
  m_dataDict.insert( recItem, mat );

  return recItem;
}

void MaterialKatalogListView::slFreshupItem( QListViewItem *item, void* templ,  bool )
{
  StockMaterial *mat = static_cast<StockMaterial*>( templ );

  if ( item && mat ) {
    Einheit e = mat->getUnit();
    kdDebug() << "Setting material name " << e.einheitSingular() << endl;
    item->setText( 0, mat->name() );
    item->setText( 1, QString::number( mat->getAmountPerPack() ) );
    item->setText( 2, e.einheit( mat->getAmountPerPack() ) );
    item->setText( 3, mat->purchPrice().toString() );
    item->setText( 4, mat->salesPrice().toString() );
    item->setText( 5, mat->lastModified() );
  } else {
    kdDebug() << "Unable to freshup item - data invalid" << endl;
  }
}

DocPosition MaterialKatalogListView::itemToDocPosition( QListViewItem *item )
{
  DocPosition pos;
  if ( ! item ) {
    item = currentItem();
  }

  if ( ! item ) return pos;

  // FIXME
  StockMaterial *mat = static_cast<StockMaterial*>( m_dataDict[item] );
  if ( mat ) {
    pos.setText( mat->name() );
    pos.setUnit( mat->getUnit() );
    pos.setUnitPrice( mat->salesPrice() );
  }

  return pos;
}
