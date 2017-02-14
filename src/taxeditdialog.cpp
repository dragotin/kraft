/***************************************************************************
                   taxeditdialog.h  - edit tax rates
                             -------------------
    begin               : Apr 9 2009
    copyright            : (C) 2009 by Klaas Freitag
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

#include <QDateTime>
#include <QSqlTableModel>
#include <QDataWidgetMapper>

#include <QDialog>
#include <QDebug>

#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "taxeditdialog.h"

TaxEditDialog::TaxEditDialog( QSqlTableModel *taxModel, QWidget *parent )
 : QDialog( parent )
{
  setObjectName( "TAX_EDIT_DIALOG" );
  setModal( true );
  setWindowTitle( i18n( "Edit Tax Rates" ) );

  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  QWidget *w = new QWidget;
  mainLayout->addWidget(w);
  mBaseWidget = new Ui::TaxEditBase( );
  mBaseWidget->setupUi( w );
  mBaseWidget->mDateWidget->setDate( QDate::currentDate() );

  mBaseWidget->mFullTax->setSuffix( "%" );
  mBaseWidget->mReducedTax->setSuffix( "%" );

  mBaseWidget->mFullTax->setRange( 0,100.0 );
  mBaseWidget->mFullTax->setDecimals( 1 );
  mBaseWidget->mReducedTax->setRange( 0, 100.0 );
  mBaseWidget->mReducedTax->setDecimals( 1 );

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);

  QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  mainLayout->addWidget(buttonBox);

  this->model = taxModel;

  mapper = new QDataWidgetMapper(this);

  mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);
  mapper->setModel(taxModel);
  mapper->addMapping(mBaseWidget->mFullTax, 1);
  mapper->addMapping(mBaseWidget->mReducedTax, 2);
  mapper->addMapping(mBaseWidget->mDateWidget, 3);
  model->insertRow(model->rowCount());
  mapper->toLast();
}

void TaxEditDialog::accept()
{
  mapper->submit();
  //Check if the inserted date already exists, if so update the existing record and delete this record
  for(int i = 0; i < model->rowCount() - 1; ++i)
  {
    if (model->index(i, 3).data(Qt::DisplayRole).toDate() == mBaseWidget->mDateWidget->date() )
    {
      //Check if the row isn't removed
      QString headerdata = model->headerData(i, Qt::Vertical, Qt::DisplayRole).toString();
      if(headerdata != "!")
      {
        model->setData(model->index(i, 1, QModelIndex()), mBaseWidget->mFullTax->value(), Qt::EditRole);
        model->setData(model->index(i, 2, QModelIndex()), mBaseWidget->mReducedTax->value(), Qt::EditRole);
        model->removeRow(model->rowCount()-1);
      }
    } 
  }

  QDialog::accept();
}

void TaxEditDialog::reject()
{
  model->removeRow(model->rowCount()-1);

  QDialog::reject();
}


