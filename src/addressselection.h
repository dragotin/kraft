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

#include <qmap.h>
#include <qstring.h>
#include <QTreeWidget>

#include <kcontacts/addressee.h>
#include <AkonadiCore/Session>
#include <AkonadiCore/ChangeRecorder>
#include <AkonadiCore/EntityTreeModel>
#include <AkonadiWidgets/entitytreeview.h>
#include <Akonadi/Contact/ContactsFilterProxyModel>
#include <akonadi/contact/contactstreemodel.h>


class QComboBox;
class QPushButton;
class KJob;
class AddressProvider;

using namespace KContacts;

class AddressSelection : public QWidget
{
  Q_OBJECT

public:
  AddressSelection( QWidget *parent = 0, bool showText = true );
  ~AddressSelection();

  QTreeView *treeView() { return mTreeView; }
signals:
  void addressSelected( const Addressee& );

protected slots:
  void slotViewClicked( const Akonadi::Item & );
  void slotOpenAddressBook();

private:
  QWidget*         contactsView();

  QMap<QTreeWidgetItem*, QString> mAddressIds;
  QPushButton         *mRefreshList;
  AddressProvider     *mAddressProvider;

  Akonadi::Session           *mAkonadiSession;
  Akonadi::ChangeRecorder    *mAkonadiChangeRecorder;
  Akonadi::ContactsTreeModel *mModel;
  Akonadi::ContactsFilterProxyModel *mFilterModel;
  Akonadi::EntityTreeView    *mTreeView;

};

#endif
