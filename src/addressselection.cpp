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
#include <k3listview.h>

#include <kabc/addressbook.h>
#include <kabc/stdaddressbook.h>

#include <qsizepolicy.h>
#include <qcombobox.h>
#include <q3widgetstack.h>
#include <qlabel.h>
#include <q3vbox.h>
//Added by qt3to4:
#include <Q3ValueList>

using namespace KABC;

AddressSelection::AddressSelection( QWidget *parent )
  : QTreeWidget( parent )
{
  setRootIsDecorated( true );
  setColumnCount( 2 );
  QStringList li;
  li << i18n( "Real Name" );
  li << i18n( "Locality" );
  setHeaderLabels( li );

  setSelectionMode( QAbstractItemView::SingleSelection );

  connect( this, SIGNAL( selectionChanged() ),
           SLOT( slotSelectionChanged() ) );
}

void AddressSelection::setupAddressList()
{
  // open the address book asynchroniouly. If it is local, it returns
  // immediately loaded however.
  mStdAddressbook = StdAddressBook::self( true );
  if ( mStdAddressbook->loadingHasFinished() ) {
    slotAddressBookChanged( mStdAddressbook );
  }

  // be prepared to changes
  connect( mStdAddressbook, SIGNAL( addressBookChanged( AddressBook* ) ),
           this, SLOT( slotAddressBookChanged( AddressBook* ) ) );
}

void AddressSelection::slotAddressBookChanged( AddressBook *ab )
{
  if ( ! ab ) return;

  kDebug() << "Filling address List" << endl;

  // FIXME: handle deletes and updates correctly.

  Q3ValueList<QString> uidList;

  uidList = mAddressIds.values();
  QTreeWidgetItem *newItem = 0;
  int newItemCnt = 0;

  AddressBook::Iterator it;
  for ( it = ab->begin(); it != ab->end(); ++it ) {

    // check if we already know the uid and add it if not.
    if ( uidList.find( ( *it ).uid() ) == uidList.end() ) {
      QTreeWidgetItem *item = new QTreeWidgetItem( this );
      item->setText( 0, ( *it ).realName() );
      newItem = item;
      newItemCnt++;

      mAddressIds[item] = ( *it ).uid();

      Address::List adr = ( *it ).addresses();
      Address::List::iterator adrIt;
      for ( adrIt = adr.begin(); adrIt != adr.end(); ++adrIt ) {
        item->setText( 1, ( *adrIt ).locality () );
      }
    }
  }

  // if there is exactly one new item, we select it
  if ( newItemCnt == 1 && newItem ) {
    clearSelection();
    setCurrentItem( newItem );
  }
}

Addressee AddressSelection::currentAddressee( QTreeWidgetItem *item )
{
  Addressee adr;
  QString adrUid;

  QTreeWidgetItem *it = item;
  if ( ! it ) {
    it = currentItem();
  }

  if ( it ) {
    adrUid = mAddressIds[it];

    if ( ! adrUid.isEmpty() ) {
      if ( mStdAddressbook && mStdAddressbook->loadingHasFinished() )
        adr = mStdAddressbook->findByUid( adrUid );
    }
  }
  return adr;
}

void AddressSelection::slotSelectionChanged()
{
  emit addressSelected( currentAddressee() );
}

#include "addressselection.moc"
