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

// include files for KDE
#include <knuminput.h>

#include <klocale.h>
#include <kdebug.h>

#include "inserttmplbase.h"
#include "templtopositiondialogbase.h"

#include "einheit.h"
#include "unitmanager.h"

InsertTemplDialog::InsertTemplDialog( QWidget *parent )
  : TemplToPositionDialogBase( parent )
{
  QWidget *w = makeVBoxMainWidget();

  mBaseWidget = new insertTmplBase( w );
  mBaseWidget->dmUnitCombo->insertStringList( UnitManager::allUnits() );
}

void InsertTemplDialog::setDocPosition( DocPosition *dp )
{
  if ( dp ) {
    mParkPosition = *dp;

    mBaseWidget->dmTextEdit->setText( mParkPosition.text() );

    mBaseWidget->dmAmount->setValue( mParkPosition.amount() );
    mBaseWidget->dmUnitCombo->setCurrentText( mParkPosition.unit().einheit( 1.0 ) );

    if ( mParkPosition.text().isEmpty() ) {
      mBaseWidget->dmHeaderText->setText( i18n( "Create a new Position" ) );
    } else {
      mBaseWidget->dmHeaderText->setText( i18n( "Create a new Position from Template" ) );
    }
  }
  mBaseWidget->dmAmount->setFocus();
}

QComboBox *InsertTemplDialog::getPositionCombo()
{
  return mBaseWidget->dmPositionCombo;
}

DocPosition InsertTemplDialog::docPosition()
{
  mParkPosition.setText( mBaseWidget->dmTextEdit->text() );
  mParkPosition.setAmount( mBaseWidget->dmAmount->value() );
  int uid = UnitManager::getUnitIDSingular( mBaseWidget->dmUnitCombo->currentText() );

  mParkPosition.setUnit( UnitManager::getUnit( uid ) );
  // mParkPosition.setPosition( itemPos );

  return mParkPosition;
}


InsertTemplDialog::~InsertTemplDialog()
{

}

#include "inserttempldialog.moc"

/* END */

