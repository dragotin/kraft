/***************************************************************************
    akonadiaddressselector - Address Selection Widget based on Akonadi
                             -------------------
    begin                : Jul 2011
    copyright            : (C) 2011 by Klaas Freitag
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

#ifndef AKONADIADDRESSSELECTOR_H
#define AKONADIADDRESSSELECTOR_H

#include <QWidget>

#include <kabc/addressee.h>


namespace Akonadi {
class Collection;
class ContactGroupViewer;
class ContactViewer;
class ContactsFilterProxyModel;
class EntityMimeTypeFilterModel;
class EntityTreeView;
class Item;
class ItemView;
class StandardContactActionManager;
}

using namespace KABC;

class QTreeView;
class QuickSearchWidget;
class QItemSelectionModel;

class AkonadiAddressSelector : public QWidget
{
    Q_OBJECT
public:
  explicit AkonadiAddressSelector(QWidget *parent = 0, bool showText = true );

  ~AkonadiAddressSelector();

  // QTreeView *treeView() { return mItemView; }
signals:
  void addressSelected( const Addressee& );

protected slots:
 // void slotViewClicked( const Akonadi::Item & );
  void slotOpenAddressBook();

private:
  QWidget*         contactsView();
  void             setupGui();

  Akonadi::EntityMimeTypeFilterModel *mCollectionTree;
  Akonadi::EntityMimeTypeFilterModel *mItemTree;
  Akonadi::EntityMimeTypeFilterModel *mAllContactsModel;
  Akonadi::ContactsFilterProxyModel *mContactsFilterModel;

  QuickSearchWidget *mQuickSearchWidget;
  Akonadi::EntityTreeView *mCollectionView;
  Akonadi::EntityTreeView *mItemView;

  QItemSelectionModel *mCollectionSelectionModel;

};

#endif // AKONADIADDRESSSELECTOR_H
