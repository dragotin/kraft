/***************************************************************************
                 addressselection  - widget to select Addresses
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

#ifndef ADDRESSSELECTION_H
#define ADDRESSSELECTION_H

#include <qvbox.h>
#include <qmap.h>
#include <qstring.h>

#include <klistview.h>
#include <kabc/addressee.h>
#include <kabc/addressbook.h>

class QComboBox;

using namespace KABC;

class AddressSelection : public KListView
{
  Q_OBJECT

public:
  AddressSelection( QWidget* );

  ~AddressSelection() { };
  void setupAddressList( );
  Addressee currentAddressee( QListViewItem* item = 0 );

signals:
  void addressSelected( const Addressee& );

protected slots:
  void slotAddressBookChanged( AddressBook* );
  void slotSelectionChanged();

private:
  QMap<QListViewItem*, QString> mAddressIds;
  AddressBook *mStdAddressbook;
};

#endif
