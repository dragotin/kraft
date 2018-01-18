/***************************************************************************
   addressselectordialog.cpp  - select addressee from address book.
                             -------------------
    begin                : Sept. 2016
    copyright            : (C) 2016 by Klaas Freitag
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

#include "kraftsettings.h"
#include "addressselectorwidget.h"
#include "addressselectordialog.h"

AddressSelectorDialog::AddressSelectorDialog( QWidget *parent )
    :QDialog(parent)
{
    setModal( true );
    const QByteArray geo = QByteArray::fromBase64( KraftSettings::self()->addressSelectDialogSize().toAscii() );
    restoreGeometry(geo);

    _addressSelectorWidget = new AddressSelectorWidget(this, false);
    connect(_addressSelectorWidget, SIGNAL(addressSelected(KContacts::Addressee)),
            SLOT(slotAddresseeSelected(KContacts::Addressee)));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(_addressSelectorWidget);

    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    mainLayout->addWidget(buttonBox);
}

void AddressSelectorDialog::slotAddresseeSelected(  const KContacts::Addressee& addressee )
{
    _addressee = addressee;
    _addressee.insertCustom(CUSTOM_ADDRESS_MARKER, "addressbook");
}

KContacts::Addressee AddressSelectorDialog::addressee()
{
    return _addressee;
}

void AddressSelectorDialog::done(int r)
{
    const QByteArray geo = saveGeometry().toBase64();
    KraftSettings::self()->setAddressSelectDialogSize(geo);
    _addressSelectorWidget->saveState();

    QDialog::done(r);
}
