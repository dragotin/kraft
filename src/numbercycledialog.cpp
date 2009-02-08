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
#include <qtooltip.h>

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
  mBaseWidget->mCounterEdit->setMaxValue( 1000000 );
  mBaseWidget->mCounterEdit->setLineStep( 1 );

  const QString tip = i18n( "The template may contain the following tags:"
                            "<ul><li>%y - the year of the documents date.</li>"
                            "<li>%w - the week number of the documents date</li>"
                            "<li>%d - the day number of the documents date</li>"
                            "<li>%m - the month number of the documents date</li>"
                            "<li>%c - the customer id from kaddressbook</li>"
                            "<li>%i - the unique counter</li>"
                            "<li>%type - the localised doc type (offer, invoice etc.)</li>"
                            "</ul>%i needs to be part of the template." );
  QToolTip::add( mBaseWidget->mIdTemplEdit, tip );

  connect( mBaseWidget->mPbAdd, SIGNAL( clicked() ),
           SLOT( slotAddCycle() ) );
  connect( mBaseWidget->mPbRemove, SIGNAL( clicked() ),
           SLOT( slotRemoveCycle() ) );

  loadCycles();

  connect( mBaseWidget->mCycleListBox, SIGNAL( highlighted( int ) ),
           SLOT( slotNumberCycleSelected( int ) ) );

  mBaseWidget->mCycleListBox->setCurrentItem( 0 );
  slotUpdateExample();

  connect( mBaseWidget->mIdTemplEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( slotTemplTextChanged( const QString& ) ) );
  connect( mBaseWidget->mCounterEdit, SIGNAL( valueChanged( int ) ),
           SLOT( slotUpdateExample() ) );
}

void NumberCycleDialog::loadCycles()
{
  QSqlQuery q( "SELECT * FROM numberCycles ORDER BY name" );

  mBaseWidget->mCycleListBox->clear();

  while ( q.next() ) {
    dbID id( q.value( 0 ).toInt() );
    NumberCycle nc( id );
    nc.setName( q.value( 1 ).toString() );
    nc.setCounter( q.value( 2 ).toInt() );
    nc.setTemplate( q.value( 3 ).toString() );

    mNumberCycles[nc.name()] = nc;
    mBaseWidget->mCycleListBox->insertItem( nc.name() );
  }
}

void NumberCycleDialog::slotUpdateExample()
{
  DocType dt;
  dt.setName( i18n( "Doc-Type" ) );
  int id = mBaseWidget->mCounterEdit->value();
  dt.setIdentTemplate( mBaseWidget->mIdTemplEdit->text() );

  QString idText = dt.generateDocumentIdent( 0, id );
  mBaseWidget->mExampleId->setText( idText );
}

void NumberCycleDialog::slotTemplTextChanged( const QString& str )
{
  bool state = false;

  if ( !str.isEmpty() && str.contains( "%i" ) ) {
    state = true;
  }
  actionButton( Ok )->setEnabled( state );
  slotUpdateExample();
}

void NumberCycleDialog::updateCycleDataFromGUI()
{
  // Store the updated values
  if ( !mSelectedCycle.isEmpty() ) {
    kdDebug() << "Updating the cycle: " << mSelectedCycle << endl;

    if ( mNumberCycles.contains( mSelectedCycle ) ) {
      QString h = mBaseWidget->mIdTemplEdit->text();
      mNumberCycles[mSelectedCycle].setTemplate( h );
      kdDebug() << "Number Cycle Template: " << h << endl;

      int num = mBaseWidget->mCounterEdit->value();
      kdDebug() << "Number Edit: " << num << endl;
      mNumberCycles[mSelectedCycle].setCounter( num );
    } else {
      kdDebug() << "WRN: NumberCycle " << mSelectedCycle << " is not known" << endl;
    }
  } else {
    kdDebug() << "The selected cycle name is Empty!" << endl;
  }

}

void NumberCycleDialog::slotNumberCycleSelected( int num )
{
  updateCycleDataFromGUI();

  // set the new data of the selected cycle
  QString name = mBaseWidget->mCycleListBox->text( num );
  if ( ! mNumberCycles.contains( name ) ) {
    kdDebug() << "No numbercycle found at pos " << num << endl;
  }
  NumberCycle nc = mNumberCycles[name];
  kdDebug() << "Selected number cycle number " << num << endl;

  mBaseWidget->mIdTemplEdit->setText( nc.getTemplate() );
  mBaseWidget->mCounterEdit->setMinValue( nc.counter() );
  mBaseWidget->mCounterEdit->setValue( nc.counter() );
  mBaseWidget->mNameEdit->setText( nc.name() );
  mBaseWidget->mNameEdit->setReadOnly( true );

  // remember the cycle name
  mSelectedCycle = name;

  bool state = true;
  if ( name == NumberCycle::defaultName() ) {
    state = false;
  }
  mBaseWidget->mPbRemove->setEnabled( state );
}

void NumberCycleDialog::slotAddCycle()
{
  QString newName = KInputDialog::getText( i18n( "Add Number Cycle" ),
                                           i18n( "Enter the name of a new number cycle." ) );
  if ( newName.isEmpty() ) return;

  bool uniq = true;
  if ( mNumberCycles.contains( newName ) ) {
    uniq = false;
  }

  if ( uniq ) {
    NumberCycle numCycle;
    numCycle.setName( newName );
    numCycle.setTemplate( QString::fromLatin1( "%y%w-%i" ) );

    QSqlQuery q( "SELECT 1+MAX(lastIdentNumber) FROM numberCycles" );

    if ( q.next() ) {
      numCycle.setCounter( q.value( 0 ).toInt() );
    }

    mNumberCycles[newName] = numCycle;
    mBaseWidget->mCycleListBox->insertItem( numCycle.name() );
  } else {
    kdDebug() << "The name is not unique!" << endl;
  }
  QListBoxItem *item = mBaseWidget->mCycleListBox->findItem( newName );
  if ( item ) {
    mBaseWidget->mCycleListBox->setCurrentItem( item );
  }
}

void NumberCycleDialog::slotRemoveCycle()
{
  QString entry = mBaseWidget->mCycleListBox->currentText();
  QListBoxItem *item = mBaseWidget->mCycleListBox->selectedItem();
  if ( entry.isEmpty() || !item ) return;

  mRemovedCycles << entry;

  if ( item ) {
    mNumberCycles.remove( entry );
    delete item;
  }
}

bool NumberCycleDialog::dropOfNumberCycleOk( const QString& name )
{
  QSqlQuery q;
  q.prepare( "SELECT count(att.id) FROM attributes att, attributeValues attVal WHERE att.id=attVal.attributeId AND att.hostObject=:dtype AND att.name=:attName AND attVal.value=:val" );
  q.bindValue( ":dtype", "DocType" );
  q.bindValue( ":attName", "identNumberCycle" );
  q.bindValue( ":val", name );
  q.exec();

  if ( q.next() ) {
    int cnt = q.value( 0 ).toInt();

    if ( cnt > 0 ) {
      KMessageBox::information( this, i18n( "The numbercycle %1 is still assigned to a document type."
                                  "The number cycle can not be deleted as long as it "
                                            "is assigned to a document type." ).arg( name ),
                                i18n( "Numbercycle Deletion" ) );
    }
    return cnt == 0;
  }
  return true;
}


void NumberCycleDialog::slotOk()
{
  kdDebug() << "Slot Ok hit" << endl;

  // get the changed stuff from the gui elements
  updateCycleDataFromGUI();

  // First remove the dropped cycles
  QSqlQuery qDel;
  qDel.prepare( "DELETE FROM numberCycles WHERE name=:name" );
  for ( QStringList::Iterator it = mRemovedCycles.begin();
        it != mRemovedCycles.end(); ++it ) {
    kdDebug() << "about to drop the number cycle " << *it << endl;
    if ( dropOfNumberCycleOk( *it ) ) {
      qDel.bindValue( ":name", *it );
      qDel.exec();
    }
  }

  // update existing entries and insert new ones
  QSqlQuery q;
  q.prepare( "SELECT * FROM numberCycles WHERE name=:name" );
  QMap<QString, NumberCycle>::Iterator it;
  for ( it = mNumberCycles.begin(); it != mNumberCycles.end(); ++it ) {
    QString cycleName = it.key();
    NumberCycle cycle = it.data();

    q.bindValue( ":name", cycleName );
    // name changes can not happen by design
    q.exec();
    if ( q.next() ) {
      kdDebug() << "Checking existing number cycle " << cycleName << " for update" << endl;
      // there is an entry
      if ( q.value( 2 ).toInt() != cycle.counter() ) {
        updateField( q.value( 0 ).toInt(),
                     "lastIdentNumber", QString::number( cycle.counter() ) );
      }
      if ( q.value( 3 ).toString() != cycle.getTemplate() ) {
        updateField( q.value( 0 ).toInt(), "identTemplate", cycle.getTemplate() );
      }
    } else {
      kdDebug() << "This number cycle is new: " << cycleName << endl;
      QSqlQuery qIns;
      qIns.prepare( "INSERT INTO numberCycles (name, lastIdentNumber, identTemplate) "
                    "VALUES (:name, :number, :templ)" );

      qIns.bindValue( ":name", cycleName );
      qIns.bindValue( ":number", cycle.counter() );
      qIns.bindValue( ":templ", cycle.getTemplate() );

      qIns.exec();
    }
  }
  KDialogBase::slotOk();
}

void NumberCycleDialog::updateField( int id, const QString& field, const QString& value )
{
  QSqlQuery qUpdate;
  QString sql = "UPDATE numberCycles SET " + field + "=:value WHERE id=:id";
  qUpdate.prepare( sql );
  // qUpdate.bindValue( ":field", field );
  qUpdate.bindValue( ":value", value );
  qUpdate.bindValue( ":id", id );

  qUpdate.exec();
}

#include "numbercycledialog.moc"

