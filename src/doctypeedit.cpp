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
#include <QSpinBox>
#include <QListWidget>
#include <QLocale>
#include <QIcon>
#include <QMessageBox>
#include <QInputDialog>
#include <QSqlQuery>

#include <QDialog>
#include <QDebug>
#include <QFileDialog>

#include "prefsdialog.h"
#include "kraftsettings.h"
#include "kraftdb.h"
#include "kraftdoc.h"
#include "defaultprovider.h"
#include "doctype.h"
#include "doctypeedit.h"
#include "numbercycledialog.h"

// --------------------------------------------------------------------------------

DocTypeEdit::DocTypeEdit( QWidget *parent )
  : QWidget(parent), Ui::DocTypeEditBase( ),
    mExampleDocType(i18n("<document type>")),
    mExampleAddressUid(i18n("<address Id>"))
{
  setupUi( this );

  connect(mTypeListBox, &QListWidget::currentTextChanged,
           this,  &DocTypeEdit::slotDocTypeSelected);

  QStringList types = DocType::allLocalised();
  mTypeListBox->clear();
  mTypeListBox->addItems( types );

  for ( QStringList::Iterator it = types.begin(); it != types.end(); ++it ) {
    DocType dt( *it );
    mOrigDocTypes[*it] = dt;
  }

  mTypeListBox->setCurrentRow( 0, QItemSelectionModel::Select );

  QString dtype;
  if(mTypeListBox->currentRow() != -1)
    dtype = mTypeListBox->currentItem()->text();

  mPbAdd->setIcon( DefaultProvider::self()->icon( "plus" ) );
  mPbEdit->setIcon( DefaultProvider::self()->icon( "pencil" ) );
  mPbRemove->setIcon( DefaultProvider::self()->icon( "minus" ) );

  const QIcon& icon = DefaultProvider::self()->icon("device-floppy");
  if (!icon.isNull() ) {
      tmplFileSelectButton->setIcon(icon);
      tmplFileSelectButton->setText("");
      watermarkSelectButton->setIcon(icon);
      watermarkSelectButton->setText("");
      appendSelectButton->setIcon(icon);
      appendSelectButton->setText("");
  }
  tmplFileSelectButton->setToolTip(i18n("Select template file from harddisk"));
  watermarkSelectButton->setToolTip(i18n("Select watermark file from harddisk"));
  appendSelectButton->setToolTip(i18n("Select PDF file to append to documents from harddisk"));

  connect(tmplFileSelectButton, &QPushButton::clicked, this, [this]() {
      QString file = QFileDialog::getOpenFileName(this,
                                                  i18n("Find Template File"), QDir::homePath(),
                                                  i18n("Kraft Templates (*.trml *.gtmpl)"));

      if (!file.isEmpty()) {
          mTemplateUrl->setText(file);
      }
  });
  connect(watermarkSelectButton, &QPushButton::clicked, this, [this]() {
      QString file = QFileDialog::getOpenFileName(this,
                                                  i18n("Find Watermark File"), QDir::homePath(),
                                                  i18n("PDF file (*.pdf)"));

      if (!file.isEmpty()) {
          mWatermarkUrl->setText(file);
      }
  });

  connect(appendSelectButton, &QPushButton::clicked, this, [this]() {
      QString file = QFileDialog::getOpenFileName(this,
                                                  i18n("Find Append PDF File"), QDir::homePath(),
                                                  i18n("PDF file (*.pdf)"));

      if (!file.isEmpty()) {
          mAppendUrl->setText(file);
      }
  });

  connect( mCbXRechnung, &QCheckBox::toggled, this, &DocTypeEdit::slotXRechnungToggled);

  connect( mPbAdd, &QPushButton::clicked, this, &DocTypeEdit::slotAddDocType);
  connect( mPbEdit, &QPushButton::clicked, this, &DocTypeEdit::slotEditDocType);
  connect( mPbRemove, &QPushButton::clicked, this, &DocTypeEdit::slotRemoveDocType);

  connect(mNumberCycleCombo, &QComboBox::currentTextChanged, this, &DocTypeEdit::slotNumberCycleChanged);

  connect( mPbEditCycles, &QPushButton::clicked, this, &DocTypeEdit::slotEditNumberCycles);

  connect( mWatermarkCombo, &QComboBox::activated, this, &DocTypeEdit::slotWatermarkModeChanged);

  connect( mWatermarkUrl, &QLineEdit::textChanged, this, &DocTypeEdit::slotWatermarkUrlChanged);
  connect( mTemplateUrl, &QLineEdit::textChanged, this, &DocTypeEdit::slotTemplateUrlChanged);

  connect( mAppendUrl, &QLineEdit::textChanged, this, &DocTypeEdit::slotAppendPDFUrlChanged );

  fillNumberCycleCombo();
  DocType dt( dtype );
  mNumberCycleCombo->setCurrentIndex(mNumberCycleCombo->findText( dt.numberCycleName() ));

  int newMode = dt.mergeIdent().toInt();
  mWatermarkCombo->setCurrentIndex( newMode );
  bool state = true;
  if ( newMode == 0 )
    state = false;
  mWatermarkUrl->setEnabled( state );

    bool xrechnungEnabled = dt.isXRechnungEnabled();
    mCbXRechnung->setCheckState(xrechnungEnabled ? Qt::Checked : Qt::Unchecked);
}

void DocTypeEdit::fillNumberCycleCombo()
{
  QMap<QString, NumberCycle> ncs = NumberCycles::load();

  const QStringList cycles = ncs.keys();
  mNumberCycleCombo->clear();
  mNumberCycleCombo->addItems(cycles);
}

void DocTypeEdit::slotAddDocType()
{
  // qDebug () << "Adding a doctype!";

  QString newName = QInputDialog::getText( this, i18n( "Add Document Type" ),
                                           i18n( "Enter the name of a new document type" ) );
  if ( newName.isEmpty() ) return;
  // qDebug () << "New Name to add: " << newName;

  if ( mTypeListBox->findItems(newName, Qt::MatchExactly).count() > 0 ) {
    // qDebug () << "New Name already exists";
  } else {
    mTypeListBox->addItem( newName );
    DocType newDt( newName, true );

    mOrigDocTypes[newName] = newDt;
    mChangedDocTypes[newName] = newDt; // Check again!
    mAddedTypes.append( newName );
  }
}

void DocTypeEdit::slotXRechnungToggled(bool newState)
{
    qDebug() << "set XREchnung state:" << newState;

    DocType dt = currentDocType();

    if ( newState != dt.isXRechnungEnabled() ) {
        dt.setXRechnungEnabled(newState);
        mChangedDocTypes[dt.name()] = dt;
    }
}

void DocTypeEdit::slotEditDocType()
{
  // qDebug () << "Editing a doctype!";

  QString currName = mTypeListBox->currentItem()->text();

  if ( currName.isEmpty() ) return;

  QString newName = QInputDialog::getText( this,
                                           i18n( "Add Document Type" ),
                                           i18n( "Edit the name of a document type" ),
                                           QLineEdit::Normal,
                                           currName );
  if ( newName.isEmpty() ) return;
  // qDebug () << "edit: " << currName << " became " << newName;
  if ( newName != currName ) {
    mTypeListBox->currentItem()->setText(newName);

    /* check if the word that was changed now was already changed before. */
    bool prechanged = false;
    bool skipEntry = false;
    QMap<QString, QString>::Iterator it;
    for ( it = mTypeNameChanges.begin(); !prechanged && it != mTypeNameChanges.end(); ++it ) {

      if (it.key() == currName ) { // it was changed back to an original name.
        mTypeNameChanges.erase( it );
        skipEntry = true;
      }

      if ( !skipEntry && it.value() == currName ) {
        // qDebug () << "Was changed before, key is " << it.key();
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
  // qDebug () << "Removing a doctype!";

  QListWidgetItem *currItem = mTypeListBox->currentItem();

  if ( !currItem || currItem->text().isEmpty() ) {
    // qDebug () << "No current Item, return";
    return;
  }
  QString currName = currItem->text();

  if ( mAddedTypes.indexOf( currName ) != -1 ) {
    // remove item from recently added list.
    mChangedDocTypes.remove( currName );
    mAddedTypes.removeAll( currName );
    mOrigDocTypes.remove( currName );
  } else {
    QString toRemove = currName;
    QMap<QString, QString>::Iterator it;
    for ( it = mTypeNameChanges.begin(); it != mTypeNameChanges.end(); ++it ) {
      if ( currName == it.value() ) {
        // remove the original name
        toRemove = it.key(); // the original name
      }
    }
    mRemovedTypes.append( toRemove );
  }

  delete currItem;
  // qDebug () << "removed type: " << mRemovedTypes;
  Q_EMIT removedType( currName );
}

void DocTypeEdit::slotDocTypeSelected( const QString& newValue )
{
  // qDebug () << "docTypeSelected: " << newValue << " and previous: " << mPreviousType;

  DocType dt( newValue );
  if ( mChangedDocTypes.contains( newValue ) ) {
    dt = mChangedDocTypes[newValue];
    // qDebug () << "new docType taken from ChangedDocTypes: ";
  }

  // store the previous type
  DocType prevType = mOrigDocTypes[mPreviousType];
  if ( mChangedDocTypes.contains( mPreviousType ) ) {
    prevType = mChangedDocTypes[mPreviousType];
    // qDebug () << "previous docType taken from ChangedDocTypes: ";
  }
  const QString oldNcName = prevType.numberCycleName();
  const NumberCycle oldNc = NumberCycles::get(oldNcName);

  prevType.setNumberCycleName(oldNcName);
  prevType.setTemplateFile( mTemplateUrl->text() );
  prevType.setWatermarkFile( mWatermarkUrl->text() );
  prevType.setAppendPDFFile(mAppendUrl->text());
  prevType.setMergeIdent( QString::number( mWatermarkCombo->currentIndex() ) );
  prevType.setXRechnungEnabled(mCbXRechnung->checkState() == Qt::Checked);
  mChangedDocTypes[mPreviousType] = prevType;

  qDebug () << "Selected doc type " << newValue;
  const QString& ncn = dt.numberCycleName();
  NumberCycle nc = NumberCycles::get(ncn);

  mIdent->setText(nc.getTemplate());
  const QString nextNum = nc.exampleIdent(dt.name(), QDate::currentDate(), QStringLiteral("<addressId>"));
  mCounter->setText(nextNum);
  mNumberCycleCombo->setCurrentIndex(mNumberCycleCombo->findText( dt.numberCycleName() ));
  // mHeader->setText( i18n( "Details for %1:", dt.name() ) );

  mExampleId->setText( nc.exampleIdent(newValue,
                                       QDate::currentDate(),
                                       mExampleAddressUid) );

  mTemplateUrl->setText( dt.templateFile() );

  mWatermarkUrl->setText( dt.watermarkFile() );
  int mergeIdent = dt.mergeIdent().toInt();
  mWatermarkCombo->setCurrentIndex( mergeIdent );
  mWatermarkUrl->setEnabled( mergeIdent > 0 );
  mAppendUrl->setText(dt.appendPDF());
  bool xrechnungEnabled = dt.isXRechnungEnabled();
  mCbXRechnung->setCheckState(xrechnungEnabled ? Qt::Checked : Qt::Unchecked);

  mPreviousType = newValue;

}

void DocTypeEdit::slotEditNumberCycles()
{
  saveDocTypes();
  QString currNumbercycle = mNumberCycleCombo->currentText();
  NumberCycleDialog dia( this, currNumbercycle );

  if ( dia.exec() == QDialog::Accepted ) {
    fillNumberCycleCombo();
    mNumberCycleCombo->setCurrentIndex(mNumberCycleCombo->findText( currNumbercycle ));
    NumberCycle nc = NumberCycles::get(currNumbercycle);

    // only the numbercycle has changed - refresh the display
    mIdent->setText(nc.getTemplate());
    int nextNum = nc.counter();
    mCounter->setText( QString::number( nextNum ) );
    const QString docType = mTypeListBox->currentItem()->text();
    mExampleId->setText( nc.exampleIdent( docType,
                                          QDate::currentDate(),
                                          mExampleAddressUid) );
  }
}

DocType DocTypeEdit::currentDocType()
{
  QString docType = mTypeListBox->currentItem()->text();
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
    if ( !mTypeListBox->currentItem()->text().isEmpty() ) {
      mChangedDocTypes[ mTypeListBox->currentItem()->text() ] = dt;
    }
  }

  bool state = true;
  if ( newMode == 0 )
    state = false;
  mWatermarkUrl->setEnabled( state );
}

void DocTypeEdit::slotAppendPDFUrlChanged(const QString& newUrl)
{
    QString docType;
    if(mTypeListBox->currentRow() != -1)
      docType = mTypeListBox->currentItem()->text();

    if( docType.isEmpty() || ! mOrigDocTypes.contains(docType) ) return;
    DocType dt = mOrigDocTypes[docType];
    if ( mChangedDocTypes.contains( docType ) ) {
      dt = mChangedDocTypes[docType];
    }

    if ( newUrl != dt.appendPDF() ) {
      dt.setAppendPDFFile(newUrl);
      mChangedDocTypes[docType] = dt;
    }
}

void DocTypeEdit::slotTemplateUrlChanged( const QString& newUrl )
{
  QString docType;
  if(mTypeListBox->currentRow() != -1)
    docType = mTypeListBox->currentItem()->text();

  if( docType.isEmpty() || ! mOrigDocTypes.contains(docType) ) return;
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
  QString docType = mTypeListBox->currentItem()->text();
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
  DocType dt = currentDocType();
  dt.setNumberCycleName( newCycle );
  mChangedDocTypes[newCycle] = dt;

  NumberCycle nc = NumberCycles::get(newCycle);
  // qDebug () << "Changing the cycle name of " << docTypeName << " to " << newCycle;

  mIdent->setText( nc.getTemplate() );
  int nextNum = nc.counter();
  mCounter->setText( QString::number( nextNum ) );
  mExampleId->setText( nc.exampleIdent(newCycle,
                                       QDate::currentDate(),
                                       mExampleAddressUid) );
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
      Q_EMIT removedType( *it );
    }
  }

  // added doctypes
  for ( QStringList::Iterator it = mAddedTypes.begin(); it != mAddedTypes.end(); ++it ) {
    QString name = *it;
    if ( mOrigDocTypes.contains( name ) ) { // just to check
      DocType dt = mChangedDocTypes[name];
      dt.save();
    }
  }

  // edited doctypes
  QMap<QString, QString>::Iterator it;
  for ( it = mTypeNameChanges.begin(); it != mTypeNameChanges.end(); ++it ) {
    QString oldName( it.key() );
    if ( mOrigDocTypes.contains( oldName ) ) {
      QString newName = it.value();
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
      qCritical() << "Can not find doctype to change named " << oldName;
    }
  }

  // check if numberCycles have changed.
  QMap<QString, DocType>::Iterator mapit;
  for ( mapit = mChangedDocTypes.begin(); mapit != mChangedDocTypes.end(); ++mapit ) {
    DocType dt = mapit.value();
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
    // qDebug () << "Can not find doctype " << name << " to remove!";
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
    q.bindValue( ":newName", newName );
    q.bindValue( ":oldId", id.toInt() );
    q.exec();
    if ( q.numRowsAffected() == 0 ) {
      qCritical() << "Database update failed for renaming " << oldName << " to " << newName;
    } else {
      // qDebug () << "Renamed doctype " << oldName << " to " << newName;
    }
  } else {
    qCritical() << "Could not find the id for doctype named " << oldName;
  }
}


