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
#include <QInputDialog>
#include <QMessageBox>

#include <QDialog>
#include <QDebug>
#include <QDialogButtonBox>
#include <QVBoxLayout>

#include "prefsdialog.h"
#include "defaultprovider.h"
#include "doctypeedit.h"
#include "numbercycledialog.h"


NumberCycleDialog::NumberCycleDialog( QWidget *parent, const QString& initType )
    :QDialog( parent ) //  "NUMBER_CYCLES_EDIT", true, i18n( "Edit Number Cycles" ), Ok|Cancel )
{
    setObjectName( "NUMBER_CYCLES_EDIT" );
    setModal( true );
    setWindowTitle( i18n( "Edit Number Cycles" ) );

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    QWidget *w = new QWidget;
    layout->addWidget(w);
    mBaseWidget = new Ui::NumberCycleEditBase( );
    mBaseWidget->setupUi( w );

    mBaseWidget->mPbAdd->setIcon( DefaultProvider::self()->icon( "list-add" ) );
    mBaseWidget->mPbRemove->setIcon( DefaultProvider::self()->icon( "list-remove" ) );
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
                              "<li>%ii .. %iiiiii - the counter padded with leading 0, ie. 012</li>"
                              "<li>%n - a day based counter, resets every day. Combined with date, it makes the number unique.</li>"
                              "<li>%nn .. %nnnnnn - the day based counter padded with leading 0.</li>"
                              "<li>%type - the localised doc type (offer, invoice etc.)</li>"
                              "<li>%uid - the contact id of the client.</li>"
                              "</ul>%i or %n need to be part of the template." );
    mBaseWidget->mIdTemplEdit->setToolTip( tip );

    connect( mBaseWidget->mPbAdd, SIGNAL( clicked() ),
             SLOT( slotAddCycle() ) );
    connect( mBaseWidget->mPbRemove, SIGNAL( clicked() ),
             SLOT( slotRemoveCycle() ) );

    loadCycles();

    connect(mBaseWidget->mCycleListBox, &QListWidget::currentRowChanged,
            this, &NumberCycleDialog::slotNumberCycleSelected);

    QListWidgetItem *initItem = mBaseWidget->mCycleListBox->findItems( initType, Qt::MatchExactly ).first();
    if ( initItem ) {
        mBaseWidget->mCycleListBox->setCurrentItem( initItem,  QItemSelectionModel::Select );
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    _okButton = buttonBox->button(QDialogButtonBox::Ok);
    _okButton->setDefault(true);
    _okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addWidget(buttonBox);
    slotUpdateExample();

    connect( mBaseWidget->mIdTemplEdit, &QLineEdit::textChanged,
             this, &NumberCycleDialog::slotTemplTextChanged);
    connect( mBaseWidget->mCounterEdit, &QSpinBox::valueChanged,
             this, &NumberCycleDialog::slotUpdateExample);
}

void NumberCycleDialog::loadCycles()
{
    QMap<QString, NumberCycle> ncs = NumberCycles::load();

    mBaseWidget->mCycleListBox->clear();

    for(const auto& nc : ncs) {
        mNumberCycles[nc.name()] = nc;
        mBaseWidget->mCycleListBox->addItem( nc.name() );
    }
}

void NumberCycleDialog::slotUpdateExample()
{
    NumberCycle nc;

    nc.setName(mBaseWidget->mNameEdit->text());
    int id = mBaseWidget->mCounterEdit->value();
    nc.setCounter(id);

    const QString tmpl = mBaseWidget->mIdTemplEdit->text();
    nc.setTemplate(tmpl);

    QString idText = nc.exampleIdent( QStringLiteral("Doc-Type"),
                                      QDate::currentDate(),
                                      QLatin1String("<addressUid>"));

    // generateDocumentIdent automatically adds a %i to the pattern, if it has neither
    // %i nor %n. A note is added here to the dialog text
    if ( !(tmpl.contains("%i") || tmpl.contains("%n"))) {
        idText.append(" ");
        idText.append(i18nc("do not translate %i, it is a template variable.", "(%i added)"));
    }
    mBaseWidget->mExampleId->setText( idText );
}

void NumberCycleDialog::slotTemplTextChanged( const QString& str )
{
    bool state = false;

    if ( !str.isEmpty() &&
         (str.contains( "%i" ) || str.contains("%n") )) {
        state = true;
    }

    if( _okButton ) {
        _okButton->setEnabled( state );
    }
    slotUpdateExample();
}

void NumberCycleDialog::updateCycleDataFromGUI()
{
    // Store the updated values
    if ( !mSelectedCycle.isEmpty() ) {
        // qDebug () << "Updating the cycle: " << mSelectedCycle;

        if ( mNumberCycles.contains( mSelectedCycle ) ) {
            const QString h = mBaseWidget->mIdTemplEdit->text();
            mNumberCycles[mSelectedCycle].setTemplate(h);
            // qDebug () << "Number Cycle Template: " << h;

            int num = mBaseWidget->mCounterEdit->value();
            // qDebug () << "Number Edit: " << num;
            mNumberCycles[mSelectedCycle].setCounter( num );
        } else {
            // qDebug () << "WRN: NumberCycle " << mSelectedCycle << " is not known";
        }
    } else {
        // qDebug () << "The selected cycle name is Empty!";
    }

}

void NumberCycleDialog::slotNumberCycleSelected( int num )
{
    updateCycleDataFromGUI();

    // set the new data of the selected cycle
    QString name = mBaseWidget->mCycleListBox->item( num )->text();
    if ( ! mNumberCycles.contains( name ) ) {
        qDebug () << "No numbercycle found at pos " << num;
    }
    NumberCycle nc = mNumberCycles[name];
    // qDebug () << "Selected number cycle number " << num;

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
    QString newName = QInputDialog::getText( this, i18n( "Add Number Cycle" ),
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
        numCycle.setCounter(1);

        mNumberCycles[newName] = numCycle;
        mBaseWidget->mCycleListBox->addItem( numCycle.name() );
    } else {
        // qDebug () << "The name is not unique!";
    }
    const auto items = mBaseWidget->mCycleListBox->findItems( newName, Qt::MatchExactly );
    QListWidgetItem *item = items.first();
    if ( item ) {
        mBaseWidget->mCycleListBox->setCurrentItem( item );
    }
}

void NumberCycleDialog::slotRemoveCycle()
{
    const QString entry = mBaseWidget->mCycleListBox->currentItem()->text();
    QListWidgetItem *item = mBaseWidget->mCycleListBox->currentItem();
    if ( entry.isEmpty() || !item ) return;

    mRemovedCycles << entry;

    if ( item ) {
        mNumberCycles.remove( entry );
        delete item;
    }
}

void NumberCycleDialog::accept()
{
    // qDebug () << "Slot Ok hit";

    // get the changed stuff from the gui elements
    updateCycleDataFromGUI();

    auto res = NumberCycles::saveAll(mNumberCycles);

    if (res != NumberCycles::SaveResult::SaveOk) {
        qDebug() << "Saving numbercycles failed!";
    }
    QDialog::accept();
}


