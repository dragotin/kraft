/***************************************************************************
   akonadiaddressselectordialog.cpp  - select addressee from address book.
                             -------------------
    begin                : Sept. 2012
    copyright            : (C) 2012 by Klaas Freitag
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

#include "akonadiaddressselector.h"
#include "akonadiaddressselectordialog.h"

AkonadiAddressSelectorDialog::AkonadiAddressSelectorDialog( QWidget *parent )
    :KDialog(parent)
{
    m_addressSelector = new AkonadiAddressSelector(this, false);
    connect(m_addressSelector, SIGNAL(addressSelected(Addressee)),SLOT(slotAddresseeSelected(Addressee)));

    setMainWidget( m_addressSelector );

    setModal( true );
    setButtons(KDialog::Ok | KDialog::Cancel);
    setDefaultButton(KDialog::Ok);
    showButtonSeparator( true);

}

void AkonadiAddressSelectorDialog::slotAddresseeSelected(  const Addressee& addressee )
{
    m_addressee = addressee;
}

KABC::Addressee AkonadiAddressSelectorDialog::addressee()
{
    return m_addressee;
}
