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

#include "ktreeviewsearchline.h"

#include <klocale.h>
#include <kdebug.h>
#include <kdialog.h>

#include <kabc/addresseelist.h>
#include <kabc/addressee.h>
#include <kabc/addressbook.h>
#include <kabc/stdaddressbook.h>
#include <kabc/contactgroup.h>

#include <akonadi/contact/contactsearchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/entitydisplayattribute.h>
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

AddressSelection::AddressSelection( QWidget *parent, bool showText )
  : QWidget( parent )
{
  QVBoxLayout *vbox = new QVBoxLayout;
  setLayout( vbox );

  if( showText ) {
    QLabel *l = new QLabel;
    l->setText(i18n("Please select a contact from the list below: "));
    vbox->addWidget( l );
    // vbox->addWidget( contactsView() );
  }

  KTreeViewSearchLine *treeViewFilter = new KTreeViewSearchLine( this );
  treeViewFilter->setMinimumWidth( 200 );
  treeViewFilter->setKeepParentsVisible( true );

  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->insertStretch( 0, 2 );
  QLabel *lab = new QLabel;
  lab->setText( i18n("&Search: ") );
  lab->setBuddy( treeViewFilter );
  hbox->addWidget( lab );
  hbox->addWidget( treeViewFilter );

  vbox->addLayout(hbox);

  // use a separated session for this model
  mAkonadiSession = new Akonadi::Session( "KraftSession" );

  Akonadi::ItemFetchScope scope;
  // fetch all content of the contacts, including images
  scope.fetchFullPayload( true );
  // fetch the EntityDisplayAttribute, which contains custom names and icons
  scope.fetchAttribute<Akonadi::EntityDisplayAttribute>();

  mAkonadiChangeRecorder = new Akonadi::ChangeRecorder;
  mAkonadiChangeRecorder->setSession( mAkonadiSession );
  // include fetching the collection tree
  mAkonadiChangeRecorder->fetchCollection( true );
  // set the fetch scope that shall be used
  mAkonadiChangeRecorder->setItemFetchScope( scope );
  // monitor all collections below the root collection for changes
  mAkonadiChangeRecorder->setCollectionMonitored( Akonadi::Collection::root() );
  // list only contacts and contact groups
  mAkonadiChangeRecorder->setMimeTypeMonitored( KABC::Addressee::mimeType(), true );
  mAkonadiChangeRecorder->setMimeTypeMonitored( KABC::ContactGroup::mimeType(), true );

  mModel = new Akonadi::ContactsTreeModel( mAkonadiChangeRecorder );
  mFilterModel = new Akonadi::ContactsFilterProxyModel;
  mFilterModel->setSourceModel( mModel );

  Akonadi::ContactsTreeModel::Columns columns;
  columns << Akonadi::ContactsTreeModel::FullName;
  columns << Akonadi::ContactsTreeModel::HomeAddress;
  mModel->setColumns( columns );

  mTreeView = new Akonadi::EntityTreeView;
  mTreeView->setModel( mFilterModel );
  vbox->addWidget( mTreeView );
  connect( mTreeView, SIGNAL( clicked( const Akonadi::Item& ) ),
          this, SLOT( slotViewClicked( const Akonadi::Item& ) ));

  mTreeView->setExpanded( mTreeView->rootIndex(), true );
  mTreeView->setSortingEnabled( true );
  mTreeView->sortByColumn( 0 );

  mTreeView->setSelectionMode( QAbstractItemView::SingleSelection );

  treeViewFilter->addTreeView( mTreeView );

  QHBoxLayout *hbox1 = new QHBoxLayout;
  vbox->addLayout( hbox1 );

  QPushButton *openAdrBook = new QPushButton(i18n("New Contact..."));
  hbox1->addWidget( openAdrBook );
  connect( openAdrBook, SIGNAL(clicked() ), SLOT( slotOpenAddressBook() ) );
  hbox1->addStretch(4);

  connect(  mTreeView, SIGNAL( currentItemChanged ( QTreeWidgetItem*, QTreeWidgetItem*  )),
           SLOT( slotSelectionChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ) );
}

AddressSelection::~AddressSelection()
{
  delete mTreeView;
  delete mModel;
  delete mFilterModel;
  delete mAkonadiSession;
  delete mAkonadiChangeRecorder;
}

void AddressSelection::slotViewClicked( const Akonadi::Item &item )
{
   if ( item.isValid() && item.hasPayload<KABC::Addressee>() ) {
     Addressee addressee = item.payload<KABC::Addressee>();
     emit addressSelected( addressee );
   }
}

void AddressSelection::slotOpenAddressBook()
{
  ContactEditorDialog *dlg = new ContactEditorDialog( Akonadi::ContactEditorDialog::CreateMode, this );
  connect( dlg, SIGNAL( contactStored( const Akonadi::Item& ) ),
           this, SLOT( slotUpdateAddressList( const Akonadi::Item& ) ) );
  dlg->show();
}

