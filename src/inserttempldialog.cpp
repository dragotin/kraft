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
#include "einheit.h"

InsertTemplDialog::InsertTemplDialog( QWidget *parent )
  :KDialogBase( parent, "TEMPL_DIALOG", true, i18n( "Create Position from Template" ),
                Ok | Cancel )
{
  QWidget *w = makeVBoxMainWidget();

  mBaseWidget = new insertTmplBase( w );
}

void InsertTemplDialog::setDocPosition( DocPosition *dp )
{
  if ( dp ) {
    mParkPosition = *dp;

    mBaseWidget->dmTextEdit->setText( mParkPosition.text() );
    // mBaseWidget->dmPositionCombo->
    mBaseWidget->dmAmount->setValue( mParkPosition.amount() );
    mBaseWidget->dmUnitText->setText( mParkPosition.unit().einheit( 1.0 ) );
  }
  mBaseWidget->dmAmount->setFocus();
}

void InsertTemplDialog::setPositionList( DocPositionList list, int intendedPos )
{
  DocPositionBase *dpb;
  QStringList strList;
  strList << i18n( "the Header of the Document as first position" );

  for ( dpb = list.first(); dpb; dpb = list.next() ) {
    DocPosition *dp = static_cast<DocPosition*>( dpb );
    QString h = QString( "%1. %2" ).arg( list.posNumber( dp ) ).arg( dp->text() );
    if ( h.length() > 50 ) {
      h = h.left( 50 );
      h += i18n( "..." );
    }
    strList.append( h );
  }

  mBaseWidget->dmPositionCombo->insertStringList( strList );
  if ( intendedPos > 0 ) --intendedPos;
  mBaseWidget->dmPositionCombo->setCurrentItem( intendedPos );
}

DocPosition InsertTemplDialog::docPosition()
{
  mParkPosition.setText( mBaseWidget->dmTextEdit->text() );
  mParkPosition.setAmount( mBaseWidget->dmAmount->value() );
  // mParkPosition.setPosition( itemPos );

  return mParkPosition;
}

int InsertTemplDialog::insertAfterPosition()
{
  int itemPos = mBaseWidget->dmPositionCombo->currentItem();
  kdDebug() << "Current item selected: " << itemPos << endl;

  return itemPos;
}

InsertTemplDialog::~InsertTemplDialog()
{

}

#include "inserttempldialog.moc"

/* END */

