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

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

#include "akonadiaddressselector.h"
#include "akonadiaddressselectordialog.h"

AkonadiAddressSelectorDialog::AkonadiAddressSelectorDialog( QWidget *parent )
    :QDialog(parent)
{
    m_addressSelector = new AkonadiAddressSelector(this, false);
    connect(m_addressSelector, SIGNAL(addressSelected(Addressee)),SLOT(slotAddresseeSelected(Addressee)));

//PORTING: Verify that widget was added to mainLayout: //PORTING: Verify that widget was added to mainLayout:     setMainWidget( m_addressSelector );
// Add mainLayout->addWidget(m_addressSelector); if necessary
// Add mainLayout->addWidget(m_addressSelector); if necessary

    setModal( true );
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    //PORTING SCRIPT: WARNING mainLayout->addWidget(buttonBox) must be last item in layout. Please move it.
    mainLayout->addWidget(buttonBox);
    buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);

}

void AkonadiAddressSelectorDialog::slotAddresseeSelected(  const Addressee& addressee )
{
    m_addressee = addressee;
}

KContacts::Addressee AkonadiAddressSelectorDialog::addressee()
{
    return m_addressee;
}
