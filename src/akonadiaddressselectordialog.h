/***************************************************************************
    akonadiaddressselectordialog.h  - select addressee from address book.
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

#ifndef AKONADIADDRESSSELECTORDIALOG_H
#define AKONADIADDRESSSELECTORDIALOG_H

#include <kdialog.h>
#include <kabc/addressee.h>

class AkonadiAddressSelector;

using namespace KABC;

class AkonadiAddressSelectorDialog : public KDialog
{
    Q_OBJECT
public:
    AkonadiAddressSelectorDialog( QWidget *parent = 0 );
    KABC::Addressee addressee();

private slots:
    void slotAddresseeSelected(const Addressee &);

private:
    AkonadiAddressSelector *m_addressSelector;
    Addressee m_addressee;
};

#endif // AKONADIADDRESSSELECTORDIALOG_H
