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


MaterialKatalogListView::MaterialKatalogListView( QWidget *w )
  : KatalogListView( w )
{
  addColumn( i18n("Material" ) );
  addColumn( i18n("Unit" ) );
  addColumn( i18n("Units per Pack" ) );
  addColumn( i18n("Price (In)" ) );
  addColumn( i18n("Price (Out)" ) );
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

KListViewItem* MaterialKatalogListView::addMaterialToView( KListViewItem *parent, StockMaterial *mat )
{
  if ( !mat ) return 0;
  if ( !parent ) parent = m_root;

  KListViewItem *recItem = new KListViewItem( parent, mat->name() );

  slFreshupItem( recItem,  mat );
  m_dataDict.insert( recItem, mat );

  return recItem;
}

void MaterialKatalogListView::slFreshupItem( QListViewItem *item, void* templ,  bool )
{
  StockMaterial *mat = static_cast<StockMaterial*>( templ );

  if ( item && mat ) {
    Einheit e = mat->getUnit();
    kdDebug() << "Setting material name " << mat->name() << endl;
    item->setText( 0, mat->name() );
    item->setText( 1, e.einheitSingular() );
    item->setText( 2, QString::number( mat->getAmountPerPack() ) );
    item->setText( 3, mat->purchPrice().toString() );
    item->setText( 4, mat->salesPrice().toString() );
  } else {
    kdDebug() << "Unable to freshup item - data invalid" << endl;
  }
}

DocPosition MaterialKatalogListView::itemToDocPosition( QListViewItem*  )
{
  DocPosition pos;
  // FIXME
  return pos;
}
