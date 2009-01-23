/***************************************************************************
                   doctypeedit.h  - the document type editor
                             -------------------
    begin                : Fri Jan 2 2009
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

#include<qlayout.h>
#include<qlineedit.h>
#include <qlineedit.h>
#include<qlabel.h>
#include<qframe.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qtextedit.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qlistbox.h>
#include <qsqlquery.h>
#include <qspinbox.h>

#include<kdialog.h>
#include<klocale.h>
#include<kiconloader.h>
#include<kmessagebox.h>
#include <kinputdialog.h>

#include "prefsdialog.h"
#include "katalogsettings.h"
#include "kraftsettings.h"
#include "kraftdb.h"
#include "kraftdoc.h"
#include "defaultprovider.h"
#include "doctype.h"
#include "doctypeedit.h"
#include "numbercycledialog.h"


NumberCycleDialog::NumberCycleDialog( QWidget *parent )
 :KDialogBase( parent, "NUMBER_CYCLES_EDIT", true, i18n( "Edit Number Cycles" ), Ok|Cancel )
{
  enableButtonSeparator( true );
  QWidget *w = makeVBoxMainWidget();

  mBaseWidget = new NumberCycleEditBase( w );

  mBaseWidget->mPbAdd->setPixmap( BarIcon( "filenew" ) );
  mBaseWidget->mPbEdit->setPixmap( BarIcon( "edit" ) );
  mBaseWidget->mPbEdit->setHidden( true );
  mBaseWidget->mPbRemove->setPixmap( BarIcon( "editdelete" ) );

  connect( mBaseWidget->mPbAdd, SIGNAL( clicked() ),
           SLOT( slotAddCycle() ) );
  connect( mBaseWidget->mPbRemove, SIGNAL( clicked() ),
           SLOT( slotRemoveCycle() ) );

  loadCycles();

  connect( mBaseWidget->mCycleListBox, SIGNAL( highlighted( int ) ),
           SLOT( slotNumberCycleSelected( int ) ) );

  mBaseWidget->mCycleListBox->setCurrentItem( 0 );

  connect( mBaseWidget->mIdTemplEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( slotTemplTextChanged( const QString& ) ) );
}

void NumberCycleDialog::loadCycles()
{
  QSqlQuery q( "SELECT * FROM numberCycles ORDER BY name" );

  int cnt = 0;
  mBaseWidget->mCycleListBox->clear();

  while ( q.next() ) {
    dbID id( q.value( 0 ).toInt() );
    NumberCycle nc( id );
    nc.setName( q.value( 1 ).toString() );
    nc.setCounter( q.value( 2 ).toInt() );
    nc.setTemplate( q.value( 3 ).toString() );
    mNumberCycles.resize( 1+cnt );
    mNumberCycles[cnt] = nc;
    cnt++;
    mBaseWidget->mCycleListBox->insertItem( nc.name(), cnt );
  }
}

void NumberCycleDialog::slotTemplTextChanged( const QString& str )
{
  bool state = false;

  if ( !str.isEmpty() && str.contains( "%i" ) ) {
    state = true;
  }
  actionButton( Ok )->setEnabled( state );
}

void NumberCycleDialog::slotNumberCycleSelected( int num )
{
  NumberCycle nc = mNumberCycles[num];
  kdDebug() << "Selected number cycle number " << num << endl;

  mBaseWidget->mIdTemplEdit->setText( nc.getTemplate() );
  mBaseWidget->mCounterEdit->setValue( nc.counter() );
  mBaseWidget->mCounterEdit->setMinValue( nc.counter() );
  mBaseWidget->mNameEdit->setText( nc.name() );
  mBaseWidget->mNameEdit->setReadOnly( true );
}

void NumberCycleDialog::slotAddCycle()
{

}

void NumberCycleDialog::slotRemoveCycle()
{

}

#include "numbercycledialog.moc"

