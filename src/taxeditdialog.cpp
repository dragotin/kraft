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

#include <QLabel>
#include <QToolTip>
#include <q3vbox.h>
#include <QDateTime>
#include <QSqlQuery>

#include <kdialog.h>
#include <klocale.h>
#include <kdebug.h>
#include <kdatewidget.h>
#include <knuminput.h>
#include <kvbox.h>

#include "taxeditdialog.h"

TaxEditDialog::TaxEditDialog( QWidget *parent )
 :KDialog( parent ) // , "TAX_EDIT", true, i18n( "Edit Tax Rates" ), Ok|Cancel )
{
  setObjectName( "TAX_EDIT_DIALOG" );
  setModal( true );
  setCaption( i18n( "Edit Tax Rates" ) );
  setButtons( Ok|Cancel );

  showButtonSeparator( true );

  KVBox *w = new KVBox( parent );
  setMainWidget( w );

  mBaseWidget = new Ui::TaxEditBase( );
  mBaseWidget->setupUi( w );

  mBaseWidget->mDateWidget->setDate( QDate::currentDate() );

  mBaseWidget->mFullTax->setSuffix( "%" );
  mBaseWidget->mReducedTax->setSuffix( "%" );

  mBaseWidget->mFullTax->setRange( 0,10.0 );
  mBaseWidget->mReducedTax->setRange( 0, 10.0 );

  mBaseWidget->mFullTax->setDecimals( 1 );
  mBaseWidget->mReducedTax->setDecimals( 1 );
}

TaxRecord TaxEditDialog::newTaxRecord()
{
  TaxRecord record;
  record.fullTax = mBaseWidget->mFullTax->value();
  record.reducedTax = mBaseWidget->mReducedTax->value();
  record.date = mBaseWidget->mDateWidget->date();

  return record;
}

#include "taxeditdialog.moc"

