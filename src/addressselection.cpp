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
#include <klistview.h>

#include <kabc/addressbook.h>
#include <kabc/stdaddressbook.h>

#include <qsizepolicy.h>
#include <qcombobox.h>
#include <qwidgetstack.h>
#include <qlabel.h>
#include <qvbox.h>

using namespace KABC;

AddressSelection::AddressSelection( QWidget *parent )
  : KListView( parent )
{
  setRootIsDecorated( true );
  addColumn( i18n( "Real Name" ) );
  addColumn( i18n( "Locality" ) );
  setSelectionMode( QListView::Single );
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

  kdDebug() << "Filling address List" << endl;

  // FIXME: handle deletes and updates correctly.

  QValueList<QString> uidList;

  uidList = mAddressIds.values();
  KListViewItem *newItem = 0;
  int newItemCnt = 0;

  AddressBook::Iterator it;
  for ( it = ab->begin(); it != ab->end(); ++it ) {

    // check if we already know the uid and add it if not.
    if ( uidList.find( ( *it ).uid() ) == uidList.end() ) {
      KListViewItem *item = new KListViewItem( this, ( *it ).realName() );
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
    setSelected( newItem, true );
  }
}

Addressee AddressSelection::currentAddressee( QListViewItem *item )
{
  Addressee adr;
  QString adrUid;

  QListViewItem *it = item;
  if ( ! it ) it = currentItem();

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
