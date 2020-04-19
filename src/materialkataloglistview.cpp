/***************************************************************************
             materialkataloglistview  - template katalog listview.
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
#include <QMap>
#include <QLocale>
#include <QDebug>
#include <QMenu>
#include <QHeaderView>

#include <KLocalizedString>

#include "matkatalog.h"
#include "stockmaterial.h"
#include "materialkataloglistview.h"
#include "katalogman.h"
#include "docposition.h"
#include "kataloglistview.h"
#include "kraftsettings.h"

MaterialKatalogListView::MaterialKatalogListView(QWidget *parent )
  : KatalogListView( parent )
{
  setColumnCount( 6 );

  QStringList headers;
  headers << i18n("Material");
  headers << i18n("Pack");
  headers << i18n("Unit");
  headers << i18n("Purchase");
  headers << i18n("Sale");
  headers << i18n("Last Modified");

  setHeaderLabels( headers );

  QByteArray headerState = QByteArray::fromBase64( KraftSettings::self()->materialCatViewHeader().toAscii() );
  header()->restoreState(headerState);
  contextMenu()->setTitle(i18n("Material Catalog"));
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
    // qDebug () << "No catalog in listview available!" << endl;
    return;
  }
  // qDebug () << "setting up meterial chapters --------*********************************+++!" << endl;
  setupChapters();

  const QList<CatalogChapter> chapters = catalog->getKatalogChapters();
  foreach( CatalogChapter theChapter, chapters ) {
    if( mChapterDict.contains( theChapter.id().toInt() ) ) {
      QTreeWidgetItem *katItem = mChapterDict[theChapter.id().toInt()];

      // hole alle Brunsrecords per Chapter und mach weiter....
      StockMaterialList records = catalog->getRecordList( theChapter );
      StockMaterialListIterator it( records );
      while( it.hasNext() ) {
        StockMaterial *mat = it.next();
        addMaterialToView( katItem,  mat );
      }
    }
  }
}

QTreeWidgetItem* MaterialKatalogListView::addMaterialToView( QTreeWidgetItem *parent, StockMaterial *mat )
{
  if ( !mat ) return 0;
  if ( !parent ) parent = m_root;

  QTreeWidgetItem *recItem = new QTreeWidgetItem( parent );
  Qt::ItemFlags flags;
  if ( mCheckboxes ) {
    recItem->setCheckState(0, Qt::Unchecked);
  }
  //recItem->setFlags( flags );
  recItem->setText( 0, mat->getText() );
  // recItem->setMultiLinesEnabled( true );  FIXME
  slFreshupItem( recItem,  mat, catalog()->locale() );
  m_dataDict.insert( recItem, mat );

  return recItem;
}

void MaterialKatalogListView::slFreshupItem( QTreeWidgetItem *item, void* templ, QLocale *loc )
{
  StockMaterial *mat = static_cast<StockMaterial*>( templ );

  if ( item && mat ) {
    Einheit e = mat->unit();
    // qDebug () << "Setting material name " << e.einheitSingular() << endl;
    item->setText( 0, mat->getText() );
    item->setText( 1, QString::number( mat->getAmountPerPack() ) );
    item->setText( 2, e.einheit( mat->getAmountPerPack() ) );
    item->setText( 3, mat->purchPrice().toString() );
    item->setText( 4, mat->salesPrice().toString() );
    item->setText( 5, mat->lastModified() );
  } else {
    // qDebug () << "Unable to freshup item - data invalid" << endl;
  }
}

DocPosition MaterialKatalogListView::itemToDocPosition( QTreeWidgetItem *item )
{
  DocPosition pos;
  if ( ! item ) {
    item = currentItem();
  }

  if ( ! item ) return pos;

  // FIXME
  StockMaterial *mat = static_cast<StockMaterial*>( m_dataDict[item] );
  if ( mat ) {
    pos.setText( mat->getText() );
    pos.setUnit( mat->unit() );
    pos.setUnitPrice( mat->salesPrice() );
  }

  return pos;
}

void MaterialKatalogListView::saveState()
{
    QByteArray state = this->header()->saveState();

    KraftSettings::self()->setMaterialCatViewHeader(state.toBase64());
}

