/***************************************************************************
        addressselection  - widget to select address entries
                             -------------------
    begin                : 2006-09-03
    copyright            : (C) 2006 by Klaas Freitag
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
#include "addressselection.h"

#include "filterheader.h"

#include <klocale.h>
#include <kdebug.h>
#include <kdialog.h>

#include <kabc/addressee.h>

#include <akonadi/contact/contactsearchjob.h>
#include <akonadi/control.h>
#include <akonadi/contact/contacteditor.h>
#include <akonadi/contact/contacteditordialog.h>

#include <QSizePolicy>
#include <QComboBox>
#include <QLabel>
#include <QTreeWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QPushButton>

using namespace KABC;
using namespace Akonadi;

AddressSelection::AddressSelection( QWidget *parent )
  : QWidget( parent )
{
  QVBoxLayout *vbox = new QVBoxLayout;
  setLayout( vbox );

  QLabel *l = new QLabel;
  l->setText(i18n("Please select a contact from the list below: "));
  vbox->addWidget( l );

  mTreeWidget = new QTreeWidget;
  vbox->addWidget( mTreeWidget );

  mTreeWidget->setRootIsDecorated( true );
  mTreeWidget->setColumnCount( 2 );
  mTreeWidget->header()->setResizeMode( QHeaderView::ResizeToContents );
  QStringList li;
  li << i18n( "Real Name" );
  li << i18n( "Locality" );
  mTreeWidget->setHeaderLabels( li );

  mTreeWidget->setSelectionMode( QAbstractItemView::SingleSelection );

  QHBoxLayout *hbox = new QHBoxLayout;
  vbox->addLayout( hbox );

  QPushButton *openAdrBook = new QPushButton(i18n("New Contact..."));
  hbox->addWidget( openAdrBook );
  connect( openAdrBook, SIGNAL(clicked() ), SLOT( slotOpenAddressBook() ) );
  hbox->addStretch(4);

  connect(  mTreeWidget, SIGNAL( currentItemChanged ( QTreeWidgetItem*, QTreeWidgetItem*  )),
           SLOT( slotSelectionChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ) );
}

void AddressSelection::slotOpenAddressBook()
{
  ContactEditorDialog *dlg = new ContactEditorDialog( Akonadi::ContactEditorDialog::CreateMode, this );
  connect( dlg, SIGNAL( contactStored( const Akonadi::Item& ) ),
           this, SLOT( slotRefreshAddressList() ) );
  dlg->show();
}

void AddressSelection::slotRefreshAddressList()
{
  setupAddressList();
}

void AddressSelection::setupAddressList()
{
  mTreeWidget->clear();
  mAddressIds.clear();
  // Start an Akonadi Search job which returns all Addresses found in Akonadis
  // contact storage, which can be multiple address books.
  Akonadi::ContactSearchJob *job = new Akonadi::ContactSearchJob;
  connect( job, SIGNAL( result( KJob* ) ), SLOT( readContacts( KJob* ) ) );
}

// Result Slot of the Contact Search above
void AddressSelection::readContacts( KJob* job )
{
  kDebug() << "Reading Akonadi Search Job!";
  if ( job->error() ) {
    qDebug() << "Akonadi Contact Job Read Error: " << job->errorString();
    return;
  }

  Akonadi::ContactSearchJob *searchJob = qobject_cast<Akonadi::ContactSearchJob*>( job );
  KABC::Addressee::List contacts = searchJob->contacts();
  kDebug() << "Amount of address entries: " << contacts.size();

  // iterate over all found contacts and write build up the treeview
  foreach ( const KABC::Addressee &contact, contacts ) {
    QTreeWidgetItem *item = new QTreeWidgetItem( mTreeWidget );
    item->setText( 0, contact.name() );

    // remember the name as a search key for the slot selectionChanged
    mAddressIds[item] = contact.name();

    Address::List adr = contact.addresses();
    Address::List::iterator adrIt;
    for ( adrIt = adr.begin(); adrIt != adr.end(); ++adrIt ) {
      item->setText( 1, ( *adrIt ).locality () );
    }
  }
}

// slot called if the user clicks on the treeview
void AddressSelection::slotSelectionChanged( QTreeWidgetItem *item, QTreeWidgetItem* )
{
  Addressee adr;
  QString adrName;

  QTreeWidgetItem *it = item;
  if ( ! it ) {
    it = mTreeWidget->currentItem();
  }

  if ( it ) {
    adrName = mAddressIds[it];

    if ( ! adrName.isEmpty() ) {
      // fire again an akonadi search job with the name as key
      // FIXME: Use the contacts Unique Object ID to search for, currently
      // not supported by Akonadi.
      Akonadi::ContactSearchJob *job = new Akonadi::ContactSearchJob;
      job->setQuery( Akonadi::ContactSearchJob::Name, adrName );
      connect( job, SIGNAL( result( KJob* ) ), SLOT( addressSelectedResult( KJob* ) ) );
    }
  }
}

// Slot to handle the selection Query Job
void AddressSelection::addressSelectedResult( KJob *job )
{
  kDebug() << "Address Selection Job!";
  if ( job->error() ) {
    qDebug() << "Akonadi Contact Job Read Error: " << job->errorString();
    return;
  }

  Akonadi::ContactSearchJob *searchJob = qobject_cast<Akonadi::ContactSearchJob*>( job );
  KABC::Addressee::List contacts = searchJob->contacts();
  kDebug() << "Amount of address entries: " << contacts.size();

  // emit the selected Signal for every Contact. Usually its only one.
  foreach ( const KABC::Addressee &contact, contacts ) {
    kDebug() << "Found the addressee: " << contact.name();
    emit addressSelected( contact );
  }
}




