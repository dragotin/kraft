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
#include "addressprovider.h"

#include <klocale.h>
#include <kdebug.h>
#include <kdialog.h>

#include <kabc/addresseelist.h>
#include <kabc/addressee.h>
#include <kabc/addressbook.h>
#include <kabc/stdaddressbook.h>
#include <kabc/contactgroup.h>

#include <akonadi/contact/contactsearchjob.h>
#include <akonadi/contact/contactstreemodel.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/entitydisplayattribute.h>
#include <akonadi/changerecorder.h>
#include <akonadi/control.h>
#include <akonadi/contact/contacteditor.h>
#include <akonadi/contact/contacteditordialog.h>
#include <akonadi/session.h>
#include <akonadi/entitytreeview.h>


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

  mAddressProvider = new AddressProvider( this );
  connect( mAddressProvider, SIGNAL( addressListFound( const KABC::Addressee::List& ) ),
           this, SLOT( slotNewAddressList( const KABC::Addressee::List& ) ) );
  // connect( mAddressProvider, SIGNAL(addresseeFound( const QString&, const KABC::Addressee& ) ),
  //         this, SLOT( slotAddresseeFound( const QString&, const KABC::Addressee& ) ) );

  QLabel *l = new QLabel;
  l->setText(i18n("Please select a contact from the list below: "));
  vbox->addWidget( l );
  // vbox->addWidget( contactsView() );

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

#if 0
QWidget* AddressSelection::contactsView()
{
  // use a separated session for this model
  Akonadi::Session *session = new Akonadi::Session( "MySession" );

  Akonadi::ItemFetchScope scope;
  // fetch all content of the contacts, including images
  scope.fetchFullPayload( true );
  // fetch the EntityDisplayAttribute, which contains custom names and icons
  scope.fetchAttribute<Akonadi::EntityDisplayAttribute>();

  Akonadi::ChangeRecorder *changeRecorder = new Akonadi::ChangeRecorder;
  changeRecorder->setSession( session );
  // include fetching the collection tree
  changeRecorder->fetchCollection( true );
  // set the fetch scope that shall be used
  changeRecorder->setItemFetchScope( scope );
  // monitor all collections below the root collection for changes
  changeRecorder->setCollectionMonitored( Akonadi::Collection::root() );
  // list only contacts and contact groups
  changeRecorder->setMimeTypeMonitored( KABC::Addressee::mimeType(), true );
  changeRecorder->setMimeTypeMonitored( KABC::ContactGroup::mimeType(), true );

  Akonadi::ContactsTreeModel *model = new Akonadi::ContactsTreeModel( changeRecorder );

  Akonadi::ContactsTreeModel::Columns columns;
  columns << Akonadi::ContactsTreeModel::FullName;
  columns << Akonadi::ContactsTreeModel::HomeAddress;
  model->setColumns( columns );

  Akonadi::EntityTreeView *view = new Akonadi::EntityTreeView;
  view->setModel( model );

  return view;
}
#endif


void AddressSelection::slotOpenAddressBook()
{
  ContactEditorDialog *dlg = new ContactEditorDialog( Akonadi::ContactEditorDialog::CreateMode, this );
  connect( dlg, SIGNAL( contactStored( const Akonadi::Item& ) ),
           this, SLOT( slotUpdateAddressList( const Akonadi::Item& ) ) );
  dlg->show();
}

void AddressSelection::slotUpdateAddressList( const Akonadi::Item& )
{
  kDebug() << "Update slot called!";
}

void AddressSelection::setupAddressList()
{
  // query all addresses and get the result in slotNewAddressList
  mAddressProvider->allAddresses();
}

void AddressSelection::slotNewAddressList( const KABC::Addressee::List& addresses )
{
  mTreeWidget->clear();
  mAddressIds.clear();

  KABC::Addressee::List sorted = addresses;
  sorted.sort();
  kDebug() << "Amount of address entries: " << sorted.size();
  if( sorted.size() ) {
    // iterate over all found contacts and write build up the treeview
    foreach ( const KABC::Addressee &contact, sorted ) {
      contactToWidgetEntry( contact );
    }
  }
}

QTreeWidgetItem* AddressSelection::contactToWidgetEntry( const KABC::Addressee& contact )
{
  QTreeWidgetItem *item = 0;

  if( ! contact.isEmpty() ) {
    item = new QTreeWidgetItem( mTreeWidget );
    item->setText( 0, contact.realName() );

    // remember the name as a search key for the slot selectionChanged
    mAddressIds[item] = contact.uid();

    Address::List adr = contact.addresses();
    Address::List::iterator adrIt;
    // FIXME
    for ( adrIt = adr.begin(); adrIt != adr.end(); ++adrIt ) {
      const QString loc = (*adrIt).locality();
      if( !loc.isEmpty() )
        item->setText( 1, loc );
    }
  }
  return item;
}

// slot called if the user clicks on the treeview
void AddressSelection::slotSelectionChanged( QTreeWidgetItem *item, QTreeWidgetItem* )
{
  QString uid;

  QTreeWidgetItem *it = item;
  if ( ! it ) {
    it = mTreeWidget->currentItem();
  }

  if ( it && mAddressIds.contains( it )) {
    uid = mAddressIds[it];

    if ( ! uid.isEmpty() ) {
      // search for the selected uid
      mAddressProvider->getAddressee( uid );
    }
  }
}

void AddressSelection::addresseeFound( const QString&, const KABC::Addressee& contact )
{
  emit addressSelected( contact );
}

