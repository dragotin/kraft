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

#include <QLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QSqlQuery>
#include <QSpinBox>
#include <QToolTip>

#include <kdialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <kvbox.h>

#include "prefsdialog.h"
#include "kraftsettings.h"
#include "kraftdb.h"
#include "kraftdoc.h"
#include "defaultprovider.h"
#include "doctype.h"
#include "doctypeedit.h"
#include "numbercycledialog.h"


NumberCycleDialog::NumberCycleDialog( QWidget *parent, const QString& initType )
 :KDialog( parent ) //  "NUMBER_CYCLES_EDIT", true, i18n( "Edit Number Cycles" ), Ok|Cancel )
{
  setObjectName( "NUMBER_CYCLES_EDIT" );
  setModal( true );
  setCaption( i18n( "Edit Number Cycles" ) );
  setButtons( Ok|Cancel );

  showButtonSeparator( true );

  QWidget *w = new QWidget(this);
  setMainWidget( w );

  mBaseWidget = new Ui::NumberCycleEditBase( );
  mBaseWidget->setupUi( w );

  mBaseWidget->mPbAdd->setIcon( KIcon( "list-add" ) );
  mBaseWidget->mPbRemove->setIcon( KIcon( "list-remove" ) );
  mBaseWidget->mCounterEdit->setMaximum( 1000000 );
  mBaseWidget->mCounterEdit->setSingleStep( 1 );

  const QString tip = i18n( "The template may contain the following tags:"
                            "<ul><li>%y or %yyyy - the year of the documents date.</li>"
                            "<li>%yy - the year of the document (two digits).</li>"

                            "<li>%w - the week number of the documents date.</li>"
                            "<li>%ww - the week number of the documents date with leading zero.</li>"

                            "<li>%d - the day number of the documents date.</li>"
                            "<li>%dd - the day number of the documents date with leading zero.</li>"

                            "<li>%m or %M - the month number of the documents date.</li>"
                            "<li>%MM - the month number with leading zero.</li>"

                            "<li>%c - the customer id from kaddressbook</li>"
                            "<li>%i - the unique counter</li>"
                            "<li>%type - the localised doc type (offer, invoice etc.)</li>"
                            "<li>%uid - the contact id of the client.</li>"
                            "</ul>%i needs to be part of the template." );
  mBaseWidget->mIdTemplEdit->setToolTip( tip );

  connect( mBaseWidget->mPbAdd, SIGNAL( clicked() ),
           SLOT( slotAddCycle() ) );
  connect( mBaseWidget->mPbRemove, SIGNAL( clicked() ),
           SLOT( slotRemoveCycle() ) );

  loadCycles();

  connect( mBaseWidget->mCycleListBox, SIGNAL( currentRowChanged( int ) ),
           SLOT( slotNumberCycleSelected( int ) ) );

  QListWidgetItem *initItem = mBaseWidget->mCycleListBox->findItems( initType, Qt::MatchExactly ).first();
  if ( initItem ) {
    mBaseWidget->mCycleListBox->setCurrentItem( initItem,  QItemSelectionModel::Select );
  }
  slotUpdateExample();

  connect( mBaseWidget->mIdTemplEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( slotTemplTextChanged( const QString& ) ) );
  connect( mBaseWidget->mCounterEdit, SIGNAL( valueChanged( int ) ),
           SLOT( slotUpdateExample() ) );
}

void NumberCycleDialog::loadCycles()
{
  QSqlQuery q( "SELECT id, name, lastIdentNumber, identTemplate FROM numberCycles ORDER BY name" );

  mBaseWidget->mCycleListBox->clear();

  while ( q.next() ) {
    dbID id( q.value( 0 ).toInt() );
    NumberCycle nc( id );
    nc.setName( q.value( 1 ).toString() );
    nc.setCounter( q.value( 2 ).toInt() );
    nc.setTemplate( q.value( 3 ).toString() );

    mNumberCycles[nc.name()] = nc;
    mBaseWidget->mCycleListBox->addItem( nc.name() );
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
  button( Ok )->setEnabled( state );
  slotUpdateExample();
}

void NumberCycleDialog::updateCycleDataFromGUI()
{
  // Store the updated values
  if ( !mSelectedCycle.isEmpty() ) {
    kDebug() << "Updating the cycle: " << mSelectedCycle;

    if ( mNumberCycles.contains( mSelectedCycle ) ) {
      QString h = mBaseWidget->mIdTemplEdit->text();
      mNumberCycles[mSelectedCycle].setTemplate( h );
      kDebug() << "Number Cycle Template: " << h;

      int num = mBaseWidget->mCounterEdit->value();
      kDebug() << "Number Edit: " << num;
      mNumberCycles[mSelectedCycle].setCounter( num );
    } else {
      kDebug() << "WRN: NumberCycle " << mSelectedCycle << " is not known";
    }
  } else {
    kDebug() << "The selected cycle name is Empty!";
  }

}

void NumberCycleDialog::slotNumberCycleSelected( int num )
{
  updateCycleDataFromGUI();

  // set the new data of the selected cycle
  QString name = mBaseWidget->mCycleListBox->item( num )->text();
  if ( ! mNumberCycles.contains( name ) ) {
    kDebug() << "No numbercycle found at pos " << num;
  }
  NumberCycle nc = mNumberCycles[name];
  kDebug() << "Selected number cycle number " << num;

  mBaseWidget->mIdTemplEdit->setText( nc.getTemplate() );
  mBaseWidget->mCounterEdit->setMinimum( 0 ); // nc.counter() );
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
    mBaseWidget->mCycleListBox->addItem( numCycle.name() );
  } else {
    kDebug() << "The name is not unique!";
  }
  QListWidgetItem *item = mBaseWidget->mCycleListBox->findItems( newName, Qt::MatchExactly ).first();
  if ( item ) {
    mBaseWidget->mCycleListBox->setCurrentItem( item );
  }
}

void NumberCycleDialog::slotRemoveCycle()
{
  QString entry = mBaseWidget->mCycleListBox->currentItem()->text();
  QListWidgetItem *item = mBaseWidget->mCycleListBox->currentItem();
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


void NumberCycleDialog::accept()
{
  kDebug() << "Slot Ok hit";

  // get the changed stuff from the gui elements
  updateCycleDataFromGUI();

  // First remove the dropped cycles
  if ( mRemovedCycles.count() > 0 ) {
    QSqlQuery qDel;
    qDel.prepare( "DELETE FROM numberCycles WHERE name=:name" );
    for ( QStringList::Iterator it = mRemovedCycles.begin();
          it != mRemovedCycles.end(); ++it ) {
      kDebug() << "about to drop the number cycle " << *it;
      if ( dropOfNumberCycleOk( *it ) ) {
        qDel.bindValue( ":name", *it );
        qDel.exec();
      }
    }
  }

  // update existing entries and insert new ones
//  CREATE TABLE numberCycles (
//    id INTEGER PRIMARY KEY ASC autoincrement,
//    name VARCHAR(64) NOT NULL,
//    lastIdentNumber  INT NOT NULL,
//    identTemplate VARCHAR(64) NOT NULL
//  );

  QSqlQuery q;
  q.prepare( "SELECT id, name, lastIdentNumber, identTemplate FROM numberCycles WHERE name=:name" );
  QMap<QString, NumberCycle>::Iterator it;
  for ( it = mNumberCycles.begin(); it != mNumberCycles.end(); ++it ) {
    QString cycleName = it.key();
    NumberCycle cycle = it.value();

    q.bindValue( ":name", cycleName );
    // name changes can not happen by design
    q.exec();
    if ( q.next() ) {
      kDebug() << "Checking existing number cycle " << cycleName << " for update";
      // there is an entry
      if ( q.value( 2 ).toInt() != cycle.counter() ) {
        bool doUpdate = true;
        if ( q.value( 2 ).toInt() > cycle.counter() ) {
          if ( q.value( 3 ).toString() == cycle.getTemplate() ) {
            // The number has become smaller but the template remains the same.
            // That has high potential to end up with duplicate doc numbers.
            if( KMessageBox::questionYesNo( this,
                                            i18n( "The new counter is lower than the old one. "
                                                  "That has potential to create duplicate document numbers. Do you really want to decrease it?" ),
                                            i18n("Dangerous Counter Change"),
                                            KStandardGuiItem::yes(), KStandardGuiItem::no() )
                  != KMessageBox::Yes )
            {
              doUpdate = false;
            }
          }
        }
        if ( doUpdate ) {
          updateField( q.value( 0 ).toInt(),
                       "lastIdentNumber", QString::number( cycle.counter() ) );
        }
      }
      if ( q.value( 3 ).toString() != cycle.getTemplate() ) {
        updateField( q.value( 0 ).toInt(), "identTemplate", cycle.getTemplate() );
      }
    } else {
      kDebug() << "This number cycle is new: " << cycleName;
      QSqlQuery qIns;
      qIns.prepare( "INSERT INTO numberCycles (name, lastIdentNumber, identTemplate) "
                    "VALUES (:name, :number, :templ)" );

      qIns.bindValue( ":name", cycleName );
      qIns.bindValue( ":number", cycle.counter() );
      qIns.bindValue( ":templ", cycle.getTemplate() );

      qIns.exec();
    }
  }
  KDialog::accept();
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

