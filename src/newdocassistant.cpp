/***************************************************************************
        newdocassistant  - widget to select header data for the doc
                             -------------------
    begin                : 2008-02-12
    copyright            : (C) 2008 by Klaas Freitag
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

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QWizard>

#include <kcontacts/addressee.h>
#include <QComboBox>
#include <QDialog>
#include <QDateEdit>
#include <QDebug>
#include <QTextEdit>
#include <QPointer>

#include <klocalizedstring.h>

#include "newdocassistant.h"
#include "defaultprovider.h"
#include "filterheader.h"
#include "doctype.h"
#include "kraftsettings.h"
#include "addressselectorwidget.h"
#include "documentman.h"


CustomerSelectPage::CustomerSelectPage( QWidget *parent )
  :QWizardPage ( parent )
{
  QVBoxLayout *vbox = new QVBoxLayout;
  setLayout( vbox );

  setTitle(i18n( "New Document Settings" ));

  QLabel *help = new QLabel;
  help->setText( i18n( "Please select a customer as addressee for the document. "
                   "If there is no entry for the customer in the addressbook yet, it can be opened "
                       "by clicking on the button below." ) );
  // help->setTextFormat( Qt::RichText );
  help->setWordWrap( true );
  help->setSizePolicy( QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed ));

  vbox->addWidget( help );

  mAddresses = new AddressSelectorWidget(this);

  connect( mAddresses,  SIGNAL( addressSelected( const KContacts::Addressee& ) ),
           SIGNAL( addresseeSelected( const KContacts::Addressee& ) ) );

  vbox->addWidget( mAddresses );
}

void CustomerSelectPage::saveState()
{
  mAddresses->saveState();
}

void CustomerSelectPage::setupAddresses()
{

}

CustomerSelectPage:: ~CustomerSelectPage()
{

}

// ###########################################################################

DocDetailsPage::DocDetailsPage( QWidget *parent )
  : QWizardPage(parent),
    _haveAddressSelect(true),
    mCustomerLabel( nullptr )
{
  QVBoxLayout *vbox = new QVBoxLayout;
  setLayout( vbox );
  setTitle(i18n( "New Document Settings" ));

  QLabel *help = new QLabel;
  help->setTextFormat( Qt::RichText );
  help->setText( i18n( "Select a document type and a date. A comment on the whiteboard "
                       "helps to classify the document." ) );
  vbox->addWidget( help );

  mCustomerLabel = new QLabel;
  mCustomerLabel->setFrameStyle( QFrame::Box + QFrame::Sunken );
  mCustomerLabel->setTextFormat( Qt::RichText );
  mCustomerLabel->setText( i18n( "Customer: Not yet selected!" ) );
  vbox->addWidget( mCustomerLabel );

  QFormLayout *grid = new QFormLayout;
  vbox->addLayout( grid );

  mTypeCombo = new QComboBox;
  mTypeCombo->insertItems( 0, DocType::allLocalised() );
  mTypeCombo->setCurrentIndex( mTypeCombo->findText( DefaultProvider::self()->docType() ));
  grid->addRow( i18n("Document &Type:"), mTypeCombo );

  mDateEdit = new QDateEdit;
  mDateEdit->setDate( QDate::currentDate() );
  grid->addRow( i18n( "Document Date: " ), mDateEdit );

  mWhiteboardEdit = new QTextEdit;
  grid->addRow( i18n( "Whiteboard Content:" ), mWhiteboardEdit );

  QHBoxLayout *hbox = new QHBoxLayout;
  vbox->addLayout(hbox);
  mKeepItemsCB = new QCheckBox( i18n("Copy document items from predecessor document"));
  hbox->addWidget( mKeepItemsCB );
  mSourceDocIdentsCombo = new QComboBox;
  hbox->addWidget(mSourceDocIdentsCombo);
  mSourceDocIdentsCombo->setVisible(false);

  mKeepItemsCB->setChecked(true);
  mKeepItemsCB->setVisible(false);

  vbox->addStretch( 1 );
}

DocDetailsPage::~DocDetailsPage()
{

}

void DocDetailsPage::setNoAddresses()
{
    _haveAddressSelect = false;
}

// ###########################################################################

KraftWizard::KraftWizard(QWidget *parent, const char* name, bool modal )
  :QWizard( parent ),
   mCustomerPage( nullptr ),
   mCustomerBox( nullptr ),
   mParent( parent )
{
  setObjectName( name );
  setModal( modal );

  const QByteArray geo = QByteArray::fromBase64( KraftSettings::self()->newDocWizardGeometry().toAscii() );
  restoreGeometry(geo);
}

KraftWizard::~KraftWizard()
{

}

void KraftWizard::init( bool haveAddressSelect, const QString& followUpDoc )
{
    QScopedPointer<AddressProvider> addressProvider;
    addressProvider.reset(new AddressProvider);

    if( followUpDoc.isEmpty() ) {
        setWindowTitle( i18n( "Create a new Kraft Document" ) );
    } else {
        setWindowTitle(followUpDoc);
    }

    mDetailsPage = new DocDetailsPage();
    addPage(mDetailsPage);
    // w1, QLatin1Literal("<h2>") +  + QLatin1Literal("</h2>") );

    // only pick an addressee if the document is really new
    if( addressProvider->backendUp() && haveAddressSelect ) {
        mCustomerPage = new CustomerSelectPage( );
        addPage( mCustomerPage); // w, QLatin1Literal("<h2>") + i18n( "Select an Addressee" ) + QLatin1Literal("</h2>") );

        mCustomerPage->setupAddresses();
        connect( mCustomerPage, SIGNAL( addresseeSelected(KContacts::Addressee)),
                 this,  SLOT( slotAddressee(KContacts::Addressee)));
    }
}

void KraftWizard::done( int r )
{
    if( mCustomerPage ) {
        mCustomerPage->saveState();
    }
    const QByteArray geo = saveGeometry().toBase64();
    KraftSettings::self()->setNewDocWizardGeometry(geo);

    QWizard::done(r);
}

void KraftWizard::slotAddressee(const KContacts::Addressee& addressee)
{
  // qDebug () << "Addressee Changed!";
  mAddressee = addressee;
}

QDate KraftWizard::date() const
{
  return mDetailsPage->mDateEdit->date();
}

QString KraftWizard::addressUid() const
{
  return mAddressee.uid();
}

QString KraftWizard::docType() const
{
  return mDetailsPage->mTypeCombo->currentText();
}

QString KraftWizard::whiteboard() const
{
  return mDetailsPage->mWhiteboardEdit->toPlainText();
}

void KraftWizard::setDocToFollow( DocGuardedPtr sourceDoc)
{
    if( !sourceDoc ) {
        return;
    }
    DocGuardedPtr dPtr = sourceDoc;

    QString id = sourceDoc->docID().toString();
    while( ! id.isEmpty() ) {
        // store the id of the follower and clear id
        const QString idT = dPtr->docIdentifier();
        mDetailsPage->mSourceDocIdentsCombo->addItem(idT, id);
        id = QString();

        // remember the current dptr to be able to delete it soon
        DocGuardedPtr oldDptr = dPtr;
        dPtr =  DocumentMan::self()->openDocumentbyIdent( dPtr->predecessor() );
        if( dPtr ) {
            id = dPtr->docID().toString();
        }
        if( oldDptr != sourceDoc ) {
            delete oldDptr;
        }
    }
    if( mDetailsPage->mSourceDocIdentsCombo->count() > 0  ) {
        mDetailsPage->mKeepItemsCB->setVisible(true);
        mDetailsPage->mSourceDocIdentsCombo->setVisible(true);
    }

    // we already know the customer, disable the customer select page.
     mDetailsPage->setNoAddresses();

    if ( mDetailsPage->mCustomerLabel ) {
        const QString followText = i18n("Followup Document for %1", sourceDoc->docIdentifier() );
        mDetailsPage->mCustomerLabel->setText( followText );
    }

}

QString KraftWizard::copyItemsFromPredecessor()
{
    QString re;
    if( mDetailsPage->mKeepItemsCB->checkState() == Qt::Checked ) {
        re = mDetailsPage->mSourceDocIdentsCombo->currentData().toString();
    }
    return re;
}

void KraftWizard::setAvailDocTypes( const QStringList& list )
{
  mDetailsPage->mTypeCombo->clear();
  mDetailsPage->mTypeCombo->insertItems( -1, list );
}
