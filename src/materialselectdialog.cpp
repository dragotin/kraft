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

#include <qlabel.h>

#include "materialkatalogview.h"
#include "materialselectdialog.h"
#include "materialkataloglistview.h"
#include "katalogman.h"
#include "matkatalog.h"
#include "katalog.h"
#include "filterheader.h"

MaterialSelectDialog::MaterialSelectDialog( QWidget *parent, const char *name )
  : KDialogBase( parent, name, true,
                 i18n( "Select Material for Calculation" ),
                 Ok|Cancel, Ok, true )
{
  QVBox *page = makeVBoxMainWidget();
  ( void ) new QLabel( i18n( "Select Material for Calculation" ),
                              page, "caption" );

  mFilter = new FilterHeader( 0, page );
  mKatalogListView = new MaterialKatalogListView( page, true );
  mFilter->setListView( mKatalogListView );

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

void MaterialSelectDialog::slotClose()
{
  KDialogBase::slotClose();
}

void MaterialSelectDialog::slotOk()
{
  kdDebug() << "++ Material selected!" << endl;

  QListViewItemIterator it( mKatalogListView, QListViewItemIterator::Checked );
  while ( it.current() ) {
    kdDebug() << "T: " << ( *it.current() ).text( 0 ) << endl;
    StockMaterial *mat = static_cast<StockMaterial*>( mKatalogListView->itemData( *it ) );
    if ( mat ) {
      emit materialSelected( mat->getID(), 1 );
    }
    ++it;
  }

  KDialogBase::slotOk();
}

#include "materialselectdialog.moc"
