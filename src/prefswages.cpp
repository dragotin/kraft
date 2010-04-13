/***************************************************************************
                   prefswages.cpp  - the wages tab in the prefs dialog
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
#include <QDataWidgetMapper>
#include <QHeaderView>
#include <QSortFilterProxyModel>


#include <kpushbutton.h>
#include <klocale.h>
#include <kdebug.h>

#include "defaultprovider.h"
#include "impviewwidgets.h"
#include "geld.h"

#include "prefswages.h"


PrefsWages::PrefsWages(QWidget* parent)
  : QWidget(parent)
{
  QVBoxLayout *vboxLay = new QVBoxLayout;

  mWagesModel = new QSqlTableModel(this);
  mWagesModel->setTable("stdSaetze");
  mWagesModel->setSort(3, Qt::AscendingOrder);
  mWagesModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
  mWagesModel->select();
  mWagesModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
  mWagesModel->setHeaderData(1, Qt::Horizontal, tr("Code"));
  mWagesModel->setHeaderData(2, Qt::Horizontal, tr("Price"));
  mWagesModel->setHeaderData(3, Qt::Horizontal, tr("Sortkey"));

  mProxyModel = new QSortFilterProxyModel(this);
  mProxyModel->setSourceModel(mWagesModel);

  mWagesTreeView = new ImpTreeView;
  vboxLay->addWidget( mWagesTreeView );
  mWagesTreeView->setModel(mProxyModel);
  mWagesTreeView->setItemDelegate(new WagesItemDelegate());
  mWagesTreeView->hideColumn(0);
  mWagesTreeView->hideColumn(3);
  mWagesTreeView->header()->stretchLastSection();
  mWagesTreeView->setColumnWidth(1, 200);
  mWagesTreeView->resizeColumnToContents(2);
  mWagesTreeView->resizeColumnToContents(1);
  mWagesTreeView->setEditTriggers(ImpTreeView::NoEditTriggers);

  connect( mWagesTreeView, SIGNAL(clicked(QModelIndex)),
           SLOT( slotWageSelected(QModelIndex) ) );

  connect( mWagesTreeView, SIGNAL(doubleClicked(QModelIndex)),
           SLOT(slotEditWage(QModelIndex)));

  QHBoxLayout *butLay = new QHBoxLayout;
  butLay->addStretch( 1 );

  mUp = new KPushButton( KIcon("arrow-up"), i18n( "Up" ));
  connect( mUp, SIGNAL( clicked() ), SLOT( slotUp() ) );
  butLay->addWidget( mUp );
  mUp->setEnabled(false);

  mDown = new KPushButton( KIcon("arrow-down"), i18n( "Down" ));
  connect( mDown, SIGNAL( clicked() ), SLOT( slotDown() ) );
  butLay->addWidget( mDown );
  mDown->setEnabled(false);

  KPushButton *but = new KPushButton( KIcon("list-add"), i18n( "Add" ));
  connect( but, SIGNAL( clicked() ), SLOT( slotAddWage() ) );
  butLay->addWidget( but );

  mEditWage = new KPushButton( KIcon("document-edit"), i18n( "Edit" ));
  connect( mEditWage, SIGNAL( clicked() ), SLOT( slotEditWage() ) );
  butLay->addWidget( mEditWage );
  mEditWage->setEnabled(false);

  mDelWage = new KPushButton( KIcon("list-remove"), i18n( "Remove" ) );
  connect( mDelWage, SIGNAL( clicked() ), SLOT( slotDeleteWage() ) );
  butLay->addWidget( mDelWage );
  mDelWage->setEnabled( false );

  vboxLay->addLayout( butLay );
  this->setLayout( vboxLay );
}

PrefsWages::~PrefsWages()
{

}

void PrefsWages::save()
{
  mWagesModel->submitAll();
}

void PrefsWages::slotAddWage()
{
  WagesEditDialog *dialog = new WagesEditDialog(mWagesModel, -1, this);
  dialog->show();
}

void PrefsWages::slotEditWage(QModelIndex /* index */ )
{
  if ( mWagesTreeView->currentIndex().isValid() ) {
    int row = mWagesTreeView->currentIndex().row();
    WagesEditDialog *dialog = new WagesEditDialog(mWagesModel, row, this);
    dialog->show();
  }
}

void PrefsWages::slotDeleteWage()
{
  if ( mWagesTreeView->currentIndex().isValid() )
  {
    int row = mWagesTreeView->currentIndex().row();
    mWagesModel->removeRows(row, 1);
  }
}

void PrefsWages::slotWageSelected(QModelIndex)
{
  bool state = false;
  if ( mWagesTreeView->currentIndex().isValid() ) {
    state = true;
  }

  mEditWage->setEnabled( state );
  mDelWage->setEnabled( state );
  mUp->setEnabled( state );
  mDown->setEnabled( state );

  if(mWagesTreeView->currentIndex().row() == 0)
    mUp->setEnabled(false);
  if(mWagesTreeView->currentIndex().row() == (mProxyModel->rowCount() - 1))
    mDown->setEnabled(false);
}

void PrefsWages::slotUp()
{
  if ( mWagesTreeView->currentIndex().isValid() )
  {
    int row = mWagesTreeView->currentIndex().row();
    if(row != 0)
    {
      mProxyModel->setData(mProxyModel->index(row, 3), row, Qt::DisplayRole);
      mProxyModel->setData(mProxyModel->index(row, 3), row, Qt::EditRole);
      mProxyModel->setData(mProxyModel->index(row-1, 3), row + 1, Qt::DisplayRole);
      mProxyModel->setData(mProxyModel->index(row-1, 3), row + 1, Qt::EditRole);
      mProxyModel->sort(3, Qt::AscendingOrder);

      slotWageSelected(mWagesTreeView->currentIndex());
    }
  }
}

void PrefsWages::slotDown()
{
  if ( mWagesTreeView->currentIndex().isValid() )
  {
    int row = mWagesTreeView->currentIndex().row();
    if(row != (mProxyModel->rowCount() - 1))
    {
      mProxyModel->setData(mProxyModel->index(row, 3), row + 2, Qt::DisplayRole);
      mProxyModel->setData(mProxyModel->index(row, 3), row + 2, Qt::EditRole);
      mProxyModel->setData(mProxyModel->index(row+1, 3), row + 1, Qt::DisplayRole);
      mProxyModel->setData(mProxyModel->index(row+1, 3), row + 1, Qt::EditRole);
      mProxyModel->sort(3, Qt::AscendingOrder);

      slotWageSelected(mWagesTreeView->currentIndex());
    }
  }
}

WagesEditDialog::WagesEditDialog( QAbstractItemModel *model, int row, QWidget *parent )
 : KDialog( parent )
{
  setObjectName( "WAGES_EDIT_DIALOG" );
  setModal( true );
  setCaption( i18n( "Edit a wage group" ) );
  setButtons( Ok|Cancel );

  showButtonSeparator( true );

  QWidget *w = new QWidget( this );
  setMainWidget( w );

  mBaseWidget = new Ui::WagesEditBase( );
  mBaseWidget->setupUi( w );

  mBaseWidget->mWage->setSuffix( DefaultProvider::self()->currencySymbol() );

  mBaseWidget->mWage->setMinimum( 0 );
  mBaseWidget->mWage->setMaximum( 100000 );
  mBaseWidget->mWage->setDecimals( 2 );

  mModel = model;

  mapper = new QDataWidgetMapper(this);

  mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
  mapper->setModel(model);
  mapper->addMapping(mBaseWidget->mGroupName, 1);
  mapper->addMapping(mBaseWidget->mWage, 2);

  if(row == -1)
  {
    //Insert a new row at the end
    model->insertRow(model->rowCount());
    mapper->toLast();
  }
  else
  {
    mBaseWidget->mLabel->setText(i18n("<h1>Edit wage group</h1>"));
    mapper->setCurrentIndex(row);
  }
  mRow = row;
}

void WagesEditDialog::accept()
{
  mapper->submit();
  KDialog::accept();
}

void WagesEditDialog::reject()
{
  if(mRow == -1)
    mModel->removeRow(mModel->rowCount()-1);
  KDialog::reject();
}


WagesItemDelegate::WagesItemDelegate(QObject * parent) : QItemDelegate(parent) {}

void WagesItemDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
  if(index.column() == 2)
  {
    Geld wage = index.data(Qt::DisplayRole).toDouble();
    QString string = wage.toString(KGlobal::locale());
    drawDisplay(painter, option, option.rect, string);
  }
  else
  {
    QItemDelegate::paint(painter, option, index);
  }
}
