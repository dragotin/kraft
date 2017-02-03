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
#include <KConfigGroup>
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

MaterialSelectDialog::MaterialSelectDialog( QWidget *parent, const char *name )
  : QDialog( parent )
{
  setObjectName( name );
  setModal( true );
  setWindowTitle( i18n("Select Material for Calculation" ) );
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);

  QVBoxLayout *page = new QVBoxLayout;
  QLabel *label = new QLabel( i18n( "Select Material for Calculation" ));
  page->addWidget(label);
  label->setObjectName("caption");

  mKatalogListView = new MaterialKatalogListView;
  FilterHeader *filter = new FilterHeader(this, mKatalogListView);
  page->addWidget(filter);
  page->addWidget(mKatalogListView);
  mKatalogListView->setCheckboxes( true );
  mainLayout->addLayout(page);

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  page->addWidget(buttonBox);

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

