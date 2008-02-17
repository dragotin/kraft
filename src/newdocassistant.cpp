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

#include <qwidget.h>
#include <qlabel.h>
#include <qvbox.h>
#include <krun.h>
#include <ktextedit.h>

#include <klocale.h>
#include <kdialog.h>
#include <kcombobox.h>
#include <kdatewidget.h>
#include <qpushbutton.h>
#include <qsizepolicy.h>

#include "newdocassistant.h"
#include "addressselection.h"
#include "doctype.h"
#include <qgrid.h>

CustomerSelectPage::CustomerSelectPage( QWidget *parent )
  :QWidget( parent )
{

  QVBox *vbox = new QVBox( parent );
  vbox->setSpacing( KDialog::marginHint() );

  QLabel *help = new QLabel( vbox );
  help->setText( i18n( "Please select a customer as addressee for the document. "
                   "If there is no entry for the customer in the addressbook yet, it can be opened "
                       "by clicking on the button below." ) );
  help->setTextFormat( Qt::RichText );
  mAddresses = new AddressSelection( vbox );
  connect( mAddresses,  SIGNAL( addressSelected( const Addressee& ) ),
           SIGNAL( addresseeSelected( const Addressee& ) ) );

  mAddresses->setupAddressList();

  QHBox *hbox = new QHBox( vbox );
  QPushButton *but = new QPushButton( i18n( "Create new Customer Entry..." ), hbox );
  connect( but, SIGNAL( clicked() ), this,  SIGNAL( startAddressbook() ) );

  QWidget *spaceEater = new QWidget( hbox );
  spaceEater->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}

CustomerSelectPage:: ~CustomerSelectPage()
{

}

// ###########################################################################

DocDetailsPage::DocDetailsPage( QWidget *parent )
  :mCustomerLabel( 0 )
{

  QVBox *vbox = new QVBox( parent );
  vbox->setSpacing( KDialog::marginHint() );

  QLabel *help = new QLabel( vbox );
  help->setTextFormat( Qt::RichText );
  help->setText( i18n( "Select a document type and a date. A comment on the whiteboard "
                       "helps to classify the document." ) );

  mCustomerLabel = new QLabel( vbox );
  mCustomerLabel->setFrameStyle( QFrame::Box + QFrame::Sunken );
  mCustomerLabel->setTextFormat( Qt::RichText );
  mCustomerLabel->setMargin( KDialog::marginHint() );
  mCustomerLabel->setText( i18n( "Customer: Not yet selected!" ) );

  QGrid *grid = new QGrid( 2,  vbox );

  grid->setSpacing( KDialog::marginHint() );

//   QLabel *l = new QLabel( i18n( "Some Document Details: " ), vbox );
//  l->setMargin( KDialog::marginHint() );

  QLabel *l = new QLabel( i18n( "Document Type: " ), grid );
  mTypeCombo = new KComboBox( grid );
  l->setBuddy( mTypeCombo );

  mTypeCombo->insertStringList( DocType::allLocalised() );

  QLabel *l2 = new QLabel( i18n( "Document Date: " ), grid );
  ( void ) l2;
  mDateEdit = new KDateWidget( grid );
  mDateEdit->setDate( QDate::currentDate() );

  l = new QLabel( vbox );
  l->setText( i18n( "Whiteboard Content:" ) );
  mWhiteboardEdit = new KTextEdit(  vbox );
  l->setBuddy( mWhiteboardEdit );

  QWidget *spaceEater = new QWidget( vbox );  // space Eater

  spaceEater->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}

DocDetailsPage::~DocDetailsPage()
{

}

// ###########################################################################

KraftWizard::KraftWizard(QWidget *parent, const char* name, bool modal, WFlags f )
  :KWizard( parent,  name,  modal,  f ),
   mCustomerBox( 0 )
{
  setMinimumWidth( 400 );
}

KraftWizard::~KraftWizard()
{

}

void KraftWizard::init()
{
  mCustomerBox = new QHBox( this );
  setCaption( i18n( "Document Creation Wizard" ) );

  mCustomerPage = new CustomerSelectPage( mCustomerBox );
  connect( mCustomerPage, SIGNAL( addresseeSelected( const Addressee& ) ),
           this,  SLOT( slotAddressee( const Addressee& ) ) );
  connect( mCustomerPage, SIGNAL( startAddressbook() ),
           this, SLOT( slotStartAddressbook() ) );


  QHBox *hb2 = new QHBox( this );
  mDetailsPage = new DocDetailsPage( hb2 );

  addPage( hb2, i18n( "Document Details" ) );
  addPage( mCustomerBox, i18n( "Select an Addressee" ) );

  setFinishEnabled ( mCustomerBox, true );
  setFinishEnabled ( hb2, true );
}

void KraftWizard::slotAddressee( const Addressee& )
{
  kdDebug() << "Addressee Changed!"  << endl;
  // setNextEnabled ( mCustomerPage, true );
}

void KraftWizard::slotStartAddressbook()
{
  KRun::runCommand( QString::fromLatin1( "kaddressbook --new-contact" ),
                    QString::fromLatin1("kaddressbook" ), "address" );
}

QDate KraftWizard::date() const
{
  return mDetailsPage->mDateEdit->date();
}

QString KraftWizard::addressUid() const
{
  return mCustomerPage->mAddresses->currentAddressee().uid();
}

QString KraftWizard::docType() const
{
  return mDetailsPage->mTypeCombo->currentText();
}

QString KraftWizard::whiteboard() const
{
  return mDetailsPage->mWhiteboardEdit->text();
}

void KraftWizard::setDocIdentifier( const QString& ident )
{
  // we already know the customer, disable the customer select page.
  setAppropriate( page( 1 ), false );
  if ( mDetailsPage->mCustomerLabel ) {
    mDetailsPage->mCustomerLabel->setText( ident );
  }
}

void KraftWizard::setAvailDocTypes( const QStringList& list )
{
  mDetailsPage->mTypeCombo->clear();
  mDetailsPage->mTypeCombo->insertStringList( list );
}

#include "newdocassistant.moc"
