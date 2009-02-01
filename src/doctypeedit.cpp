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

#include "prefsdialog.h"
#include "katalogsettings.h"
#include "kraftsettings.h"
#include "kraftdb.h"
#include "kraftdoc.h"
#include "defaultprovider.h"
#include "doctype.h"
#include "doctypeedit.h"
#include <kinputdialog.h>
#include "numbercycledialog.h"

// --------------------------------------------------------------------------------

DocTypeEdit::DocTypeEdit( QWidget *parent )
  : DocTypeEditBase( parent )
{
  connect( mTypeListBox, SIGNAL( highlighted( const QString& ) ),
           this,  SLOT( slotDocTypeSelected( const QString& ) ) );

  QStringList types = DocType::allLocalised();;
  mTypeListBox->clear();
  mTypeListBox->insertStringList( types );

  for ( QStringList::Iterator it = types.begin(); it != types.end(); ++it ) {
    DocType dt( *it );
    mNumberCycleDict[*it] = dt.numberCycleName();
    mOrigDocTypes[*it] = dt;
  }

  mTypeListBox->setSelected( 0, true );
  QString dtype = mTypeListBox->currentText();

  mPbAdd->setPixmap( BarIcon( "filenew" ) );
  mPbEdit->setPixmap( BarIcon( "edit" ) );
  mPbRemove->setPixmap( BarIcon( "editdelete" ) );

  connect( mPbAdd, SIGNAL( clicked() ),
           SLOT( slotAddDocType() ) );
  connect( mPbEdit, SIGNAL( clicked() ),
           SLOT( slotEditDocType() ) );
  connect( mPbRemove, SIGNAL( clicked() ),
           SLOT( slotRemoveDocType() ) );

  connect( mNumberCycleCombo, SIGNAL( activated( const QString& ) ),
           SLOT( slotNumberCycleChanged( const QString& ) ) );

  connect( mPbEditCycles, SIGNAL( clicked() ),
           SLOT( slotEditNumberCycles() ) );

  fillNumberCycleCombo();
  DocType dt( dtype );
  mNumberCycleCombo->setCurrentText( dt.numberCycleName() );
}

void DocTypeEdit::fillNumberCycleCombo()
{
  QSqlQuery q;
  q.prepare( "SELECT name FROM numberCycles ORDER BY name" );
  q.exec();
  QStringList cycles;
  while ( q.next() ) {
    cycles << q.value( 0 ).toString();
  }
  mNumberCycleCombo->clear();
  mNumberCycleCombo->insertStringList( cycles );
}

void DocTypeEdit::slotAddDocType()
{
  kdDebug() << "Adding a doctype!" << endl;

  QString newName = KInputDialog::getText( i18n( "Add Document Type" ),
                                           i18n( "Enter the name of a new document type" ) );
  if ( newName.isEmpty() ) return;
  kdDebug() << "New Name to add: " << newName << endl;

  if ( mTypeListBox->findItem( newName ) ) {
    kdDebug() << "New Name already exists" << endl;
  } else {
    mTypeListBox->insertItem( newName );
    DocType newDt( newName );
    mNumberCycleDict[newName] = NumberCycle::defaultName();
    mOrigDocTypes[newName] = newDt;
    mAddedTypes.append( newName );
  }
}

void DocTypeEdit::slotEditDocType()
{
  kdDebug() << "Editing a doctype!" << endl;

  QString currName = mTypeListBox->currentText();

  if ( currName.isEmpty() ) return;

  QString newName = KInputDialog::getText( i18n( "Add Document Type" ),
                                           i18n( "Enter the name of a new document type" ),
                                           currName );
  if ( newName.isEmpty() ) return;
  kdDebug() << "edit: " << currName << " became " << newName << endl;
  if ( newName != currName ) {
    mTypeListBox->changeItem( newName, mTypeListBox->currentItem() );

    /* check if the word that was changed now was already changed before. */
    bool prechanged = false;
    bool skipEntry = false;
    QMap<QString, QString>::Iterator it;
    for ( it = mTypeNameChanges.begin(); !prechanged && it != mTypeNameChanges.end(); ++it ) {

      if (it.key() == currName ) { // it was changed back to an original name.
        mTypeNameChanges.remove( it );
        skipEntry = true;
      }

      if ( !skipEntry && it.data() == currName ) {
        kdDebug() << "Was changed before, key is " << it.key() << endl;
        currName = it.key();
        prechanged = true;
      }
    }
    if ( ! skipEntry ) {
      mTypeNameChanges[currName] = newName;
      DocType dt( currName );
      mNumberCycleDict[newName] = dt.numberCycleName();
    }
  }
}

void DocTypeEdit::slotRemoveDocType()
{
  kdDebug() << "Removing a doctype!" << endl;

  QString currName = mTypeListBox->currentText();

  if ( currName.isEmpty() ) {
    kdDebug() << "No current Item, return" << endl;
    return;
  }

  if ( mAddedTypes.find( currName ) != mAddedTypes.end() ) {
    // remove item from recently added list.
    mAddedTypes.remove( currName );
    mOrigDocTypes.remove( currName );
  } else {
    QString toRemove = currName;
    QMap<QString, QString>::Iterator it;
    for ( it = mTypeNameChanges.begin(); it != mTypeNameChanges.end(); ++it ) {
      if ( currName == it.data() ) {
        // remove the original name
        toRemove = it.key(); // the original name
      }
    }
    mRemovedTypes.append( toRemove );
  }
  mTypeListBox->removeItem( mTypeListBox->currentItem() );
}

void DocTypeEdit::slotDocTypeSelected( const QString& newValue )
{
  QString value = mTypeListBox->currentText();
  if ( ! newValue.isEmpty() ) {
    value = newValue;
  }
  DocType dt( value );
  if ( mNumberCycleDict.contains( value ) ) {
    dt.setNumberCycleName( mNumberCycleDict[value] );
  }
  kdDebug() << "Selected doc type " << value << endl;
  mIdent->setText( dt.identTemplate() );
  int nextNum = dt.nextIdentId( false );
  mCounter->setText( QString::number( nextNum ) );
  mNumberCycleCombo->setCurrentText( dt.numberCycleName() );
  // mHeader->setText( i18n( "Details for %1:" ).arg( dt.name() ) );
  mExampleId->setText( dt.generateDocumentIdent( 0, nextNum ) );

}

void DocTypeEdit::slotEditNumberCycles()
{
  NumberCycleDialog dia( this );

  if ( dia.exec() == QDialog::Accepted ) {
    fillNumberCycleCombo();
  }
}

void DocTypeEdit::slotNumberCycleChanged( const QString& newCycle )
{
  QString docType = mTypeListBox->currentText();
  mNumberCycleDict[docType] = newCycle;
  kdDebug() << "Changing the cycle name of " << docType << " to " << newCycle << endl;

  DocType dt( docType );
  dt.setNumberCycleName( newCycle );

  mIdent->setText( dt.identTemplate() );
  int nextNum = dt.nextIdentId( false );
  mCounter->setText( QString::number( nextNum ) );
  mExampleId->setText( dt.generateDocumentIdent( 0, nextNum ) );
}

QStringList DocTypeEdit::allNumberCycles()
{
  QStringList re;
  re << NumberCycle::defaultName();
  QSqlQuery q( "SELECT av.value FROM attributes a, attributeValues av "
               "WHERE a.id=av.attributeId AND a.hostObject='DocType' "
               "AND a.name='identNumberCycle'" );

  while ( q.next() ) {
    QString cycleName = q.value(0).toString();
    re << cycleName;
  }
  return re;
}

void DocTypeEdit::saveDocTypes()
{
  // removed doctypes
  // FIXME: Remove unreferenced number cycles
  for ( QStringList::Iterator it = mRemovedTypes.begin(); it != mRemovedTypes.end(); ++it ) {
    if ( mOrigDocTypes.contains( *it ) ) {
      DocType dt = mOrigDocTypes[*it];
      removeTypeFromDb( *it );
      mOrigDocTypes.remove( *it );
      mNumberCycleDict.remove( *it );
    }
  }

  // added doctypes
  for ( QStringList::Iterator it = mAddedTypes.begin(); it != mAddedTypes.end(); ++it ) {
    QString name = *it;
    if ( mOrigDocTypes.contains( name ) ) { // just to check
      DocType dt( name );
      QString numCycleName = mNumberCycleDict[name];
      kdDebug() << "Number cycle name for to add doctype " << name << ": " << numCycleName << endl;
      dt.setNumberCycleName( numCycleName );
      dt.save();
    }
  }

  // edited doctypes
  QMap<QString, QString>::Iterator it;
  for ( it = mTypeNameChanges.begin(); it != mTypeNameChanges.end(); ++it ) {
    QString oldName( it.key() );
    if ( mOrigDocTypes.contains( oldName ) ) {
      QString newName = it.data();
      kdDebug() << "Renaming " << oldName << " to " << newName << endl;
      DocType dt = mOrigDocTypes[oldName];
      dt.setName( newName );
      mOrigDocTypes.remove( oldName );
      mOrigDocTypes[newName] = dt;
      QString numCycleName = mNumberCycleDict[oldName];
      mNumberCycleDict[newName] = numCycleName;
      mNumberCycleDict.remove( oldName );
      dt.setNumberCycleName( numCycleName );
      // renameTypeInDb( oldName, newName );
      dt.save();
    } else {
      kdError() << "Can not find doctype to change named " << oldName << endl;
    }
  }

  // check if numberCycles have changed.
  QMap<QString, QString>::Iterator mapit;
  for ( mapit = mNumberCycleDict.begin(); mapit != mNumberCycleDict.end(); ++mapit ) {
    DocType dt( mapit.key() );
    if ( dt.numberCycleName() != mNumberCycleDict[mapit.key()] ) {
      // the numberCycleName has changed.
      dt.setNumberCycleName( mapit.data() );
      dt.save();
    }
  }

  // now the list of document types should be up to date and reflected into
  // the database.
  DocType::clearMap();
}

void DocTypeEdit::removeTypeFromDb( const QString& name )
{
  QSqlQuery delQuery;

  dbID id = DocType::docTypeId( name );
  if ( !id.isOk() ) {
    kdDebug() << "Can not find doctype " << name << " to remove!" << endl;
    return;
  }

  // delete in DocTypeRelations
  delQuery.prepare( "DELETE FROM DocTypeRelations WHERE followerId=:id or typeId=:id" );
  delQuery.bindValue( ":id", id.toString() );
  delQuery.exec();
  kdDebug() << "1-XXXXXXXXXXX " << delQuery.lastError().text() << endl;

  // delete in DocTexts
  delQuery.prepare( "DELETE FROM DocTexts WHERE DocTypeId=:id" );
  delQuery.bindValue( ":id", id.toString() );
  delQuery.exec();
  kdDebug() << "2-XXXXXXXXXXX " << delQuery.lastError().text() << endl;

  // delete in the DocTypes table
  delQuery.prepare( "DELETE FROM DocTypes WHERE docTypeId=:id" );
  delQuery.bindValue( ":id", id.toString() );
  delQuery.exec();
  kdDebug() << "3-XXXXXXXXXXX " << delQuery.lastError().text() << endl;

  AttributeMap attMap( "DocType" );
  attMap.dbDeleteAll( id );
}

void DocTypeEdit::renameTypeInDb( const QString& oldName,  const QString& newName )
{
  QSqlQuery q;
  q.prepare( "UPDATE DocTypes SET name=:newName WHERE docTypeID=:oldId" );
  dbID id = DocType::docTypeId( oldName );
  if ( id.isOk() ) {
    q.bindValue( ":newName", newName.utf8() );
    q.bindValue( ":oldId", id.toInt() );
    q.exec();
    if ( q.numRowsAffected() == 0 ) {
      kdError() << "Database update failed for renaming " << oldName << " to " << newName << endl;
    } else {
      kdDebug() << "Renamed doctype " << oldName << " to " << newName << endl;
    }
  } else {
    kdError() << "Could not find the id for doctype named " << oldName << endl;
  }
}

#include "doctypeedit.moc"

