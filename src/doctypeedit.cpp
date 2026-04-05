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
#include "defaultprovider.h"
#include "doctype.h"
#include "doctypeedit.h"
#include "numbercycledialog.h"
#include "xmldirlister.h"

// --------------------------------------------------------------------------------

DocTypeEdit::DocTypeEdit( QWidget *parent )
    : QWidget(parent), Ui::DocTypeEditBase( ),
      mExampleDocType(i18n("<document type>")),
      mExampleAddressUid(i18n("<address Id>"))
{
    setupUi( this );

    connect(mTypeListBox, &QListWidget::currentTextChanged,
            this,  &DocTypeEdit::slotDocTypeSelected);


    DocTypes dts;
    dts.loadAll();
    _dts = dts.map();
    QStringList types = _dts.keys();
    mTypeListBox->clear();
    mTypeListBox->addItems( types );

    fillNumberCycleCombo();

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

    mTypeListBox->setCurrentRow(0);
}

void DocTypeEdit::fillNumberCycleCombo()
{
    NumberCycles ncs;
    ncs.loadAll();

    const QStringList cycles = ncs.map().keys();
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

    if ( _dts.keys().contains(newName)) {
        // qDebug () << "New Name already exists";
    } else {
        mTypeListBox->addItem( newName );
        DocType newDt;
        newDt.setName(newName);
        _dts.insert(newName, newDt);
    }
}

void DocTypeEdit::slotXRechnungToggled(bool newState)
{
    qDebug() << "set XREchnung state:" << newState;

    DocType dt = currentDocType();

    if ( newState != dt.isXRechnungEnabled() ) {
        dt.setXRechnungEnabled(newState);
        _dts.insert(dt.name(), dt);
    }
}

void DocTypeEdit::slotEditDocType()
{
    // qDebug () << "Editing a doctype!";

    QString currName = mTypeListBox->currentItem()->text();
    DocType oldDt = currentDocType();

    if ( currName.isEmpty() ) return;

    QString newName = QInputDialog::getText( this,
                                             i18n( "Edit Document Type" ),
                                             i18n( "Edit the name of a document type" ),
                                             QLineEdit::Normal,
                                             currName );
    if ( newName.isEmpty() || newName == currName) {
        return;
    }
    // qDebug () << "edit: " << currName << " became " << newName;

    mTypeListBox->currentItem()->setText(newName);

    Q_ASSERT(_dts.keys().contains(currName));

    DocType dt = _dts[currName];
    dt.setName(newName);
    _dts.insert(newName, dt);
    _dts.remove(currName);
    mRemovedTypes.append(currName);

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

    Q_ASSERT(_dts.keys().contains(currName));
    const DocType dt = _dts[currName];
    _dts.remove(currName);
    mRemovedTypes.append(currName);
    delete currItem;

}

void DocTypeEdit::slotDocTypeSelected( const QString& newValue )
{
    // qDebug () << "docTypeSelected: " << newValue << " and previous: " << mPreviousType;

    NumberCycles ncs;
    ncs.loadAll();

    Q_ASSERT(_dts.keys().contains(newValue));
    const DocType dt = _dts[newValue];

    qDebug () << "Selected doc type " << newValue;
    const QString& ncn = dt.numberCycleName();
    NumberCycle nc = ncs.get(ncn);

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
    int mergeIdent = dt.mergeIdent();
    mWatermarkCombo->setCurrentIndex( mergeIdent );
    mWatermarkUrl->setEnabled( mergeIdent > 0 );
    mAppendUrl->setText(dt.appendPDF());
    bool xrechnungEnabled = dt.isXRechnungEnabled();
    mCbXRechnung->setCheckState(xrechnungEnabled ? Qt::Checked : Qt::Unchecked);
}

void DocTypeEdit::slotEditNumberCycles()
{
    saveDocTypes();
    NumberCycles ncs;
    ncs.loadAll();

    QString currNumbercycle = mNumberCycleCombo->currentText();
    NumberCycleDialog dia( this, currNumbercycle );

    if ( dia.exec() == QDialog::Accepted ) {
        fillNumberCycleCombo();
        mNumberCycleCombo->setCurrentIndex(mNumberCycleCombo->findText( currNumbercycle ));
        NumberCycle nc = ncs.get(currNumbercycle);

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
    Q_ASSERT(!docType.isEmpty() && _dts.keys().contains(docType));
    const DocType dt = _dts[docType];

    return dt;
}

void DocTypeEdit::slotWatermarkModeChanged( int newMode )
{
    DocType dt = currentDocType();

    if ( newMode != dt.mergeIdent() ) {
        dt.setMergeIdent( newMode );
        _dts.insert(dt.name(), dt);
    }

    bool state = newMode != 0;
    mWatermarkUrl->setEnabled( state );
}

void DocTypeEdit::slotAppendPDFUrlChanged(const QString& newUrl)
{
    DocType dt = currentDocType();

    if (newUrl != dt.appendPDF()) {
        dt.setAppendPDFFile(newUrl);
        _dts.insert(dt.name(), dt);
    }
}

void DocTypeEdit::slotTemplateUrlChanged( const QString& newUrl )
{
    DocType dt = currentDocType();

    if (newUrl != dt.templateFile()) {
        dt.setTemplateFile(newUrl);
        _dts.insert(dt.name(), dt);
    }
}

void DocTypeEdit::slotWatermarkUrlChanged( const QString& newUrl )
{
    DocType dt = currentDocType();

    if (newUrl != dt.watermarkFile()) {
        dt.setWatermarkFile(newUrl);
        _dts.insert(dt.name(), dt);
    }
}

void DocTypeEdit::slotNumberCycleChanged( const QString& newCycle )
{
    DocType dt = currentDocType();
    if (dt.numberCycleName() == newCycle) {
        return;
    }
    dt.setNumberCycleName( newCycle );

    NumberCycles ncs;
    ncs.loadAll();
    NumberCycle nc = ncs.get(newCycle);
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
    DocTypes dts;
    auto res = dts.saveAll(_dts);
    for (const auto &rf : mRemovedTypes) {
        dts.remove(rf);
    }
    Q_UNUSED(res) // FIXME
}

#if 0
void DocTypeEdit::removeTypeXml( const QString& name )
{
    DocTypes dts;
    dts.loadAll();
    const DocType dt = dts.get(name);

    dts.remove(dt);
}

void DocTypeEdit::renameTypeXml( const QString& oldName,  const QString& newName )
{
    DocTypes dts;
    dts.loadAll();
    const DocType dtOld = dts.get(oldName);

    DocType dt = dtOld;
    dt.setName(newName);
    dts.save(dt);

    dts.remove(dtOld);
}
#endif

