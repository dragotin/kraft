/***************************************************************************
                   prefsunits.cpp  - the units tab in the prefs dialog
                             -------------------
    begin                : Feb 26 2010
    copyright            : (C) 2010 by Thomas Richard
    email                : thomas.richard@proan.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSqlTableModel>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QDataWidgetMapper>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QPushButton>
#include <QLocale>

#include <QDebug>
#include <QDialogButtonBox>

#include <klocalizedstring.h>

#include "defaultprovider.h"
#include "impviewwidgets.h"
#include "geld.h"
#include "unitmanager.h"

#include "prefsunits.h"


PrefsUnits::PrefsUnits(QWidget* parent)
  : QWidget(parent)
{
  QVBoxLayout *vboxLay = new QVBoxLayout;

  mUnitsModel = new QSqlTableModel(this);
  mUnitsModel->setTable("units");
  mUnitsModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
  mUnitsModel->select();
  mUnitsModel->setHeaderData(0, Qt::Horizontal, i18n("ID"));
  mUnitsModel->setHeaderData(1, Qt::Horizontal, i18n("Short"));
  mUnitsModel->setHeaderData(2, Qt::Horizontal, i18n("Long"));
  mUnitsModel->setHeaderData(3, Qt::Horizontal, i18n("Short plural"));
  mUnitsModel->setHeaderData(4, Qt::Horizontal, i18n("Long plural"));

  mProxyModel = new QSortFilterProxyModel(this);
  mProxyModel->setSourceModel(mUnitsModel);

  mUnitsTreeView = new ImpTreeView;
  vboxLay->addWidget( mUnitsTreeView );
  mUnitsTreeView->setModel(mProxyModel);
  mUnitsTreeView->hideColumn(0);
  mUnitsTreeView->header()->setResizeMode(QHeaderView::ResizeToContents);

  mUnitsTreeView->setEditTriggers(ImpTreeView::NoEditTriggers);

  connect( mUnitsTreeView, SIGNAL(clicked(QModelIndex)),
           SLOT( slotUnitSelected(QModelIndex) ) );

  connect( mUnitsTreeView, SIGNAL(doubleClicked(QModelIndex)),
           SLOT(slotEditUnit(QModelIndex)));

  QHBoxLayout *butLay = new QHBoxLayout;
  butLay->addStretch( 1 );

  QPushButton *but = new QPushButton( QIcon::fromTheme("list-add"), i18n( "Add" ));
  connect( but, SIGNAL( clicked() ), SLOT( slotAddUnit() ) );
  butLay->addWidget( but );

  mEditUnit = new QPushButton( QIcon::fromTheme("document-edit"), i18n( "Edit" ));
  connect( mEditUnit, SIGNAL( clicked() ), SLOT( slotEditUnit() ) );
  butLay->addWidget( mEditUnit );
  mEditUnit->setEnabled(false);

  mDelUnit = new QPushButton( QIcon::fromTheme("list-remove"), i18n( "Remove" ) );
  connect( mDelUnit, SIGNAL( clicked() ), SLOT( slotDeleteUnit() ) );
  butLay->addWidget( mDelUnit );
  mDelUnit->setEnabled( false );

  vboxLay->addLayout( butLay );
  this->setLayout( vboxLay );
}

PrefsUnits::~PrefsUnits()
{

}

void PrefsUnits::save()
{
  bool ok = mUnitsModel->submitAll();
  if( ! ok ) {
      QString err = mUnitsModel->lastError().text();

      // qDebug () << "SQL Error: " << err;
  }
}

void PrefsUnits::slotAddUnit()
{
  UnitsEditDialog *dialog = new UnitsEditDialog(mUnitsModel, -1, this);
  dialog->show();
}

void PrefsUnits::slotEditUnit(QModelIndex /* index */ )
{
  if ( mUnitsTreeView->currentIndex().isValid() ) {
    int row = mUnitsTreeView->currentIndex().row();
    UnitsEditDialog *dialog = new UnitsEditDialog(mUnitsModel, row, this);
    dialog->show();
  }
}

void PrefsUnits::slotDeleteUnit()
{
  if ( mUnitsTreeView->currentIndex().isValid() )
  {
    int row = mUnitsTreeView->currentIndex().row();
    mUnitsModel->removeRows(row, 1);
  }
}

void PrefsUnits::slotUnitSelected(QModelIndex)
{
  bool state = false;
  if ( mUnitsTreeView->currentIndex().isValid() ) {
    state = true;
  }

  mEditUnit->setEnabled( state );
  mDelUnit->setEnabled( state );
}

UnitsEditDialog::UnitsEditDialog( QAbstractItemModel *model, int row, QWidget *parent )
 : QDialog( parent )
{
  setObjectName( "UNITS_EDIT_DIALOG" );
  setModal( true );
  setWindowTitle( i18n( "Edit a unit" ) );
  QWidget *mainWidget = new QWidget(this);
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  mainLayout->addWidget(mainWidget);

  QWidget *w = new QWidget( this );
  mainLayout->addWidget(w);

  mBaseWidget = new Ui::UnitsEditBase( );
  mBaseWidget->setupUi( w );

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  mainLayout->addWidget(buttonBox);

  mModel = model;

  mapper = new QDataWidgetMapper(this);

  mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
  mapper->setModel(model);
  mapper->addMapping(mBaseWidget->mUnitShort, 1);
  mapper->addMapping(mBaseWidget->mUnitLong, 2);
  mapper->addMapping(mBaseWidget->mUnitPluShort, 3);
  mapper->addMapping(mBaseWidget->mUnitPluLong, 4);

  if(row == -1)
  {
    //Insert a new row at the end
      int row = model->rowCount();
    if( model->insertRow(row) ) {
        int indx = UnitManager::self()->nextFreeId();
        model->setData( model->index(row, 0), indx );
        mapper->toLast();
    }
  }
  else
  {
    mBaseWidget->mLabel->setText(i18n("<h1>Edit unit</h1>"));
    mapper->setCurrentIndex(row);
  }
  mRow = row;
}

void UnitsEditDialog::accept()
{
  bool ok = mapper->submit();
  if(!ok) {
      qDebug () << "UnitsEditDialog Mapper submit result: " << ok;
  }
  QDialog::accept();
  deleteLater();
}

void UnitsEditDialog::reject()
{
  if(mRow == -1)
    mModel->removeRow(mModel->rowCount()-1);
  QDialog::reject();
  deleteLater();
}
