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
#include <kurlrequester.h>

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

  connect( mWatermarkCombo, SIGNAL( activated( int ) ),
           SLOT( slotWatermarkModeChanged( int ) ) );

  connect( mWatermarkUrl, SIGNAL( textChanged( const QString& ) ),
           SLOT( slotWatermarkUrlChanged( const QString& ) ) );

  connect( mTemplateUrl, SIGNAL( textChanged( const QString& ) ),
           SLOT( slotTemplateUrlChanged( const QString& ) ) );

  fillNumberCycleCombo();
  DocType dt( dtype );
  mNumberCycleCombo->setCurrentText( dt.numberCycleName() );

  mTemplateUrl->setFilter( "*.trml" );
  mWatermarkUrl->setFilter( "*.pdf" );

  mTemplateUrl->setURL( dt.templateFile() );
  mWatermarkUrl->setURL( dt.watermarkFile() );

  int newMode = dt.mergeIdent().toInt();
  mWatermarkCombo->setCurrentItem( newMode );
  bool state = true;
  if ( newMode == 0 )
    state = false;
  mWatermarkUrl->setEnabled( state );
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
                                           i18n( "Edit the name of a document type" ),
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
      if ( mChangedDocTypes.contains( currName ) ) {
        dt = mChangedDocTypes[currName];
      }
      dt.setName( newName );
      mChangedDocTypes[newName] = dt;
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
  kdDebug() << "docTypeSelected: " << newValue << " and previous: " << mPreviousType << endl;

  DocType dt( newValue );
  if ( mChangedDocTypes.contains( newValue ) ) {
    dt = mChangedDocTypes[newValue];
    kdDebug() << "new docType taken from ChangedDocTypes: " << endl;
  }

  // store the previous type
  DocType prevType = mOrigDocTypes[mPreviousType];
  if ( mChangedDocTypes.contains( mPreviousType ) ) {
    prevType = mChangedDocTypes[mPreviousType];
    kdDebug() << "previous docType taken from ChangedDocTypes: " << endl;
  }
  prevType.setNumberCycleName( mNumberCycleCombo->currentText() );
  prevType.setTemplateFile( mTemplateUrl->url() );
  prevType.setWatermarkFile( mWatermarkUrl->url() );
  prevType.setMergeIdent( QString::number( mWatermarkCombo->currentItem() ) );
  mChangedDocTypes[mPreviousType] = prevType;

  // dt.setNumberCycleName( dt.numberCycleName() );
  kdDebug() << "Selected doc type " << newValue << endl;
  mIdent->setText( dt.identTemplate() );
  int nextNum = dt.nextIdentId( false );
  mCounter->setText( QString::number( nextNum ) );
  mNumberCycleCombo->setCurrentText( dt.numberCycleName() );
  // mHeader->setText( i18n( "Details for %1:" ).arg( dt.name() ) );
  mExampleId->setText( dt.generateDocumentIdent( 0, nextNum ) );
  mTemplateUrl->setURL( dt.templateFile() );

  mWatermarkUrl->setURL( dt.watermarkFile() );
  mWatermarkCombo->setCurrentItem( dt.mergeIdent().toInt() );

  mPreviousType = newValue;

}

void DocTypeEdit::slotEditNumberCycles()
{
  saveDocTypes();
  QString currNumbercycle = mNumberCycleCombo->currentText();
  NumberCycleDialog dia( this, currNumbercycle );

  if ( dia.exec() == QDialog::Accepted ) {
    fillNumberCycleCombo();
    mNumberCycleCombo->setCurrentText( currNumbercycle );

    DocType dt = currentDocType();
    dt.readIdentTemplate();
    // only the numbercycle has changed - refresh the display
    mIdent->setText( dt.identTemplate() );
    int nextNum = dt.nextIdentId( false );
    mCounter->setText( QString::number( nextNum ) );
    mExampleId->setText( dt.generateDocumentIdent( 0, nextNum ) );
  }
}

DocType DocTypeEdit::currentDocType()
{
  QString docType = mTypeListBox->currentText();
  DocType dt = mOrigDocTypes[docType];
  if ( mChangedDocTypes.contains( docType ) ) {
    dt = mChangedDocTypes[docType];
  }
  return dt;
}

void DocTypeEdit::slotWatermarkModeChanged( int newMode )
{
  DocType dt = currentDocType();

  QString newMergeIdent = QString::number( newMode );
  if ( newMergeIdent != dt.mergeIdent() ) {
    dt.setMergeIdent( newMergeIdent );
    if ( !mTypeListBox->currentText().isEmpty() ) {
      mChangedDocTypes[ mTypeListBox->currentText() ] = dt;
    }
  }

  bool state = true;
  if ( newMode == 0 )
    state = false;
  mWatermarkUrl->setEnabled( state );
}

void DocTypeEdit::slotTemplateUrlChanged( const QString& newUrl )
{
  QString docType = mTypeListBox->currentText();
  DocType dt = mOrigDocTypes[docType];
  if ( mChangedDocTypes.contains( docType ) ) {
    dt = mChangedDocTypes[docType];
  }

  if ( newUrl != dt.templateFile() ) {
    dt.setTemplateFile( newUrl );
    mChangedDocTypes[docType] = dt;
  }
}

void DocTypeEdit::slotWatermarkUrlChanged( const QString& newUrl )
{
  QString docType = mTypeListBox->currentText();
  DocType dt = mOrigDocTypes[docType];
  if ( mChangedDocTypes.contains( docType ) ) {
    dt = mChangedDocTypes[docType];
  }

  if ( newUrl != dt.watermarkFile() ) {
    dt.setWatermarkFile( newUrl );
    mChangedDocTypes[docType] = dt;
  }

}

void DocTypeEdit::slotNumberCycleChanged( const QString& newCycle )
{
  QString docTypeName = mTypeListBox->currentText();
  DocType dt = currentDocType();
  dt.setNumberCycleName( newCycle );
  mChangedDocTypes[docTypeName] = dt;
  kdDebug() << "Changing the cycle name of " << docTypeName << " to " << newCycle << endl;

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
      mChangedDocTypes.remove( *it );
    }
  }

  // added doctypes
  for ( QStringList::Iterator it = mAddedTypes.begin(); it != mAddedTypes.end(); ++it ) {
    QString name = *it;
    if ( mOrigDocTypes.contains( name ) ) { // just to check
      DocType dt = mChangedDocTypes[name];
      QString numCycleName = dt.numberCycleName();
      kdDebug() << "Number cycle name for to add doctype " << name << ": " << numCycleName << endl;
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
      if ( mChangedDocTypes.contains( newName ) ) {
        dt = mChangedDocTypes[newName];
      } else {
        dt.setName( newName );
      }
      mOrigDocTypes.remove( oldName );
      mOrigDocTypes[newName] = dt;
      dt.save();
    } else {
      kdError() << "Can not find doctype to change named " << oldName << endl;
    }
  }

  // check if numberCycles have changed.
  QMap<QString, DocType>::Iterator mapit;
  for ( mapit = mChangedDocTypes.begin(); mapit != mChangedDocTypes.end(); ++mapit ) {
    DocType dt = mapit.data();
    dt.save();
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

  // delete in DocTexts
  delQuery.prepare( "DELETE FROM DocTexts WHERE DocTypeId=:id" );
  delQuery.bindValue( ":id", id.toString() );
  delQuery.exec();

  // delete in the DocTypes table
  delQuery.prepare( "DELETE FROM DocTypes WHERE docTypeId=:id" );
  delQuery.bindValue( ":id", id.toString() );
  delQuery.exec();

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

