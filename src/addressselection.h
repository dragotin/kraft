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

#include <kabc/addressee.h>
#include <akonadi/item.h>
#include <akonadi/session.h>
#include <akonadi/changerecorder.h>
#include <akonadi/entitytreeview.h>
#include <akonadi/contact/contactstreemodel.h>
#include <akonadi/contact/contactsfilterproxymodel.h>


class QComboBox;
class QPushButton;
class KJob;
class AddressProvider;

using namespace KABC;

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
