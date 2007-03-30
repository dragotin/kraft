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



AddressSelection::AddressSelection()
{

}

void AddressSelection::setupAddressList( KListView* listView )
{
   KABC::AddressBook *ab = KABC::StdAddressBook::self();

   if ( ab ) {
     KABC::AddressBook::Iterator it;
     for ( it = ab->begin(); it != ab->end(); ++it ) {
       KListViewItem *item = new KListViewItem( listView, ( *it ).realName() );

       KABC::Address::List adr = ( *it ).addresses();
       KABC::Address::List::iterator adrIt;
       for ( adrIt = adr.begin(); adrIt != adr.end(); ++adrIt ) {
         item->setText( 1, ( *adrIt ).locality () );
       }

     }
   }
}

