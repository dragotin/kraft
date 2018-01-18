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

#include <QDebug>
#include <QDialog>

#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <klocalizedstring.h>

#include "materialkatalogview.h"
#include "materialselectdialog.h"
#include "materialkataloglistview.h"
#include "katalogman.h"
#include "matkatalog.h"
#include "katalog.h"
#include "filterheader.h"

MaterialSelectDialog::MaterialSelectDialog( QWidget *parent)
  : CalcDialogBase( parent )
{
  setWindowTitle( i18n("Add Material to Calculation" ) );
  QVBoxLayout *mainLayout = new QVBoxLayout;

  _centralWidget->setLayout(mainLayout);

  QLabel *label = new QLabel( i18n( "<h1>Add Material to Calculation</h1>" ));
  mainLayout->addWidget(label);

  mKatalogListView = new MaterialKatalogListView;
  FilterHeader *filter = new FilterHeader(this, mKatalogListView);
  mainLayout->addWidget(filter);
  mainLayout->addWidget(mKatalogListView);

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
  // qDebug () << "++ Material selected!" << endl;

  QTreeWidgetItemIterator it( mKatalogListView, QTreeWidgetItemIterator::Checked );
  while (*it) {
    // qDebug () << "T: " << (*it)->text( 0 ) << endl;
    QTreeWidgetItem *item = *it;
    if( !( mKatalogListView->isChapter( item ) || mKatalogListView->isRoot( item ))) {
      StockMaterial *mat = static_cast<StockMaterial*>( mKatalogListView->itemData( item ) );
      if ( mat ) {
        emit materialSelected( mat->getID(), 1 );
      }
    }
    ++it;
  }

  QDialog::accept();
}

