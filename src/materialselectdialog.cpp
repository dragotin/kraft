/***************************************************************************
       materialselectdialog - select material for inserting into template
                             -------------------
    begin                : 2006-12-17
    copyright            : (C) 2006 by Klaas Freitag
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

#include <klocale.h>
#include <kdebug.h>
#include <kvbox.h>
#include <kdialog.h>

#include <QLabel>

#include "materialkatalogview.h"
#include "materialselectdialog.h"
#include "materialkataloglistview.h"
#include "katalogman.h"
#include "matkatalog.h"
#include "katalog.h"
#include "filterheader.h"

MaterialSelectDialog::MaterialSelectDialog( QWidget *parent, const char *name )
  : KDialog( parent )
{
  setObjectName( name );
  setModal( true );
  setCaption( i18n("Select Material for Calculation" ) );
  setButtons( KDialog::Ok | KDialog::Cancel );

  KVBox *page = new KVBox( this );
  setMainWidget( page );
  QLabel *label = new QLabel( i18n( "Select Material for Calculation" ),
                              page);
  label->setObjectName("caption");

  mFilter = new FilterHeader( 0, page );
  mKatalogListView = new MaterialKatalogListView( page );
  mFilter->setListView( mKatalogListView );
  mKatalogListView->setCheckboxes( true );

  Katalog *kat = KatalogMan::self()->getKatalog( MaterialKatalogView::MaterialCatalogName );

  if ( ! kat ) {
    kat = new MatKatalog( MaterialKatalogView::MaterialCatalogName );
    KatalogMan::self()->registerKatalog( kat );
  }
  mKatalogListView->addCatalogDisplay( MaterialKatalogView::MaterialCatalogName );
}


MaterialSelectDialog::~MaterialSelectDialog()
{

}

void MaterialSelectDialog::accept()
{
  kDebug() << "++ Material selected!" << endl;

  QTreeWidgetItemIterator it( mKatalogListView, QTreeWidgetItemIterator::Checked );
  while (*it) {
    kDebug() << "T: " << (*it)->text( 0 ) << endl;
    QTreeWidgetItem *item = *it;
    if( !( mKatalogListView->isChapter( item ) || mKatalogListView->isRoot( item ))) {
      StockMaterial *mat = static_cast<StockMaterial*>( mKatalogListView->itemData( item ) );
      if ( mat ) {
        emit materialSelected( mat->getID(), 1 );
      }
    }
    ++it;
  }

  KDialog::accept();
}

#include "materialselectdialog.moc"
