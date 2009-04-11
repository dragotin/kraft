/***************************************************************************
                   taxeditdialog.h  - edit tax rates
                             -------------------
    begin                : Apr 9 2009
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

#include<qlabel.h>
#include <qtooltip.h>
#include <qvbox.h>
#include <qdatetime.h>
#include <qsqlquery.h>

#include<kdialog.h>
#include<klocale.h>
#include<kdebug.h>
#include<kdatewidget.h>
#include<knuminput.h>

#include "taxeditdialog.h"

TaxEditDialog::TaxEditDialog( QWidget *parent )
 :KDialogBase( parent, "TAX_EDIT", true, i18n( "Edit Tax Rates" ), Ok|Cancel )
{
  enableButtonSeparator( true );
  QWidget *w = makeVBoxMainWidget();

  mBaseWidget = new TaxEditBase( w );
  mBaseWidget->mDateWidget->setDate( QDate::currentDate() );

  mBaseWidget->mFullTax->setSuffix( "%" );
  mBaseWidget->mReducedTax->setSuffix( "%" );

  mBaseWidget->mFullTax->setRange( 0,10.0 );
  mBaseWidget->mReducedTax->setRange( 0, 10.0 );

  mBaseWidget->mFullTax->setPrecision( 1 );
  mBaseWidget->mReducedTax->setPrecision( 1 );
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

