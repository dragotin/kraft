/***************************************************************************
 inserttemplatedialog.cpp  - small dialog to insert templates into documents
                             -------------------
    begin                : Sep 2006
    copyright            : (C) 2006 Klaas Freitag
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "inserttempldialog.h"

// include files for Qt
#include <qvbox.h>
#include <qtextedit.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>

// include files for KDE
#include <knuminput.h>

#include <klocale.h>
#include <kdebug.h>

#include "inserttmplbase.h"
#include "templtopositiondialogbase.h"
#include "katalog.h"
#include "einheit.h"
#include "unitmanager.h"
#include "defaultprovider.h"

InsertTemplDialog::InsertTemplDialog( QWidget *parent )
  : TemplToPositionDialogBase( parent )
{
  QWidget *w = makeVBoxMainWidget();

  mBaseWidget = new insertTmplBase( w );
  mBaseWidget->dmUnitCombo->insertStringList( UnitManager::allUnits() );

  mBaseWidget->mPriceVal->setSuffix( DefaultProvider::self()->currencySymbol() );

  mBaseWidget->mPriceVal->setMinValue( 0 );
  mBaseWidget->mPriceVal->setMaxValue( 1000000 );
  mBaseWidget->mPriceVal->setPrecision( 2 );

  // hide the chapter combo by default
  mBaseWidget->mKeepGroup->hide();
  enableButtonSeparator( false );
}

void InsertTemplDialog::setDocPosition( DocPosition *dp, bool isNew )
{
  if ( dp ) {
    mParkPosition = *dp;

    mBaseWidget->dmTextEdit->setText( mParkPosition.text() );

    mBaseWidget->dmAmount->setValue( mParkPosition.amount() );
    mBaseWidget->dmUnitCombo->setCurrentText( mParkPosition.unit().einheit( 1.0 ) );
    mBaseWidget->mPriceVal->setValue( mParkPosition.unitPrice().toDouble() );

    if ( mParkPosition.text().isEmpty() ) {
      mBaseWidget->dmHeaderText->setText( i18n( "Create a new Position" ) );
    } else {
      mBaseWidget->dmHeaderText->setText( i18n( "Create a new Position from Template" ) );
    }
    if ( isNew ) {
      mBaseWidget->dmTextEdit->setFocus();
    } else {
      mBaseWidget->dmAmount->setFocus();
    }
  }
}

QComboBox *InsertTemplDialog::getPositionCombo()
{
  return mBaseWidget->dmPositionCombo;
}

DocPosition InsertTemplDialog::docPosition()
{
  mParkPosition.setText( mBaseWidget->dmTextEdit->text() );
  mParkPosition.setAmount( mBaseWidget->dmAmount->value() );
  mParkPosition.setUnitPrice( Geld( mBaseWidget->mPriceVal->value() ) );
  int uid = UnitManager::getUnitIDSingular( mBaseWidget->dmUnitCombo->currentText() );

  mParkPosition.setUnit( UnitManager::getUnit( uid ) );
  // mParkPosition.setPosition( itemPos );

  return mParkPosition;
}


InsertTemplDialog::~InsertTemplDialog()
{

}

void InsertTemplDialog::setCatalogChapters( const QStringList& chapters )
{
  if ( chapters.count() > 0 ) {
    mBaseWidget->mKeepGroup->show();
    mBaseWidget->mComboChapter->insertStringList( chapters );
    mBaseWidget->mComboChapter->setCurrentText( Katalog::UnsortedChapter );
  }
}

QString InsertTemplDialog::chapter() const
{
  return mBaseWidget->mComboChapter->currentText();
}

#include "inserttempldialog.moc"

/* END */

