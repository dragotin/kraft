/***************************************************************************
       addressselectordialog.h  - select addressee from address book.
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

#ifndef ADDRESSSELECTORDIALOG_H
#define ADDRESSSELECTORDIALOG_H

#include <QDialog>
#include <kcontacts/addressee.h>

class AddressSelectorWidget;

using namespace KContacts;

class AddressSelectorDialog : public QDialog
{
    Q_OBJECT
public:
    AddressSelectorDialog( QWidget *parent = 0 );
    KContacts::Addressee addressee();

    void done(int r);

private slots:
    void slotAddresseeSelected(const KContacts::Addressee&);

private:
    AddressSelectorWidget *_addressSelectorWidget;
    Addressee _addressee;
};

#endif // ADDRESSSELECTORDIALOG_H
