/***************************************************************************
    addressselectorwidget - Address Selection Widget
                             -------------------
    begin                : Jul 2011
    copyright            : (C) 2011- by Klaas Freitag
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

#ifndef ADDRESSSELECTORWIDGET_H
#define ADDRESSSELECTORWIDGET_H

#include <QWidget>
#include <QSortFilterProxyModel>
#include <QSplitter>

#define HAVE_AKONADI 1

#ifdef HAVE_AKONADI
#include <Akonadi/Contact/ContactViewer>
#endif

#include <kcontacts/addressee.h>
#include "ui_addressselectorwidget.h"

#include "addressprovider.h"

using namespace KContacts;

class QLabel;
class QPushButton;
class QTreeView;
class QuickSearchWidget;
class QItemSelectionModel;
class QuickSearchWidget;

/* =============================================================== */


class KraftContactViewer : public QWidget
{
    Q_OBJECT
    public:

    explicit KraftContactViewer(QWidget *parent = 0);

    void setContact( const KContacts::Addressee& contact);

private:
#ifdef HAVE_AKONADI
    Akonadi::ContactViewer *_contactViewer;
#endif
};

class AddressSortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    AddressSortProxyModel(AddressProvider *provider, QObject *parent = 0);

protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const;
    // bool lessThan(const QModelIndex &left, const QModelIndex &right) const Q_DECL_OVERRIDE;

public:
    QString mFilter;
    AddressProvider *_provider;
};

/* =============================================================== */

class AddressSelectorWidget : public QSplitter
{
    Q_OBJECT
public:
  explicit AddressSelectorWidget(QWidget *parent = 0, bool showText = true );

  ~AddressSelectorWidget();

signals:
  void addressSelected( const Addressee& );

public slots:
  void saveState();

protected slots:
  void slotCreateNewContact();
  void slotEditContact();
  void restoreState();
  void slotItemActivated( const QModelIndex& index );
  void slotFilterTextChanged( const QString& filter);

private slots:
  void slotAddresseeSelected(QModelIndex index);

private:
  void setupUi();

  QPushButton       *mButEditContact;

  Ui::AddressSelectorWidget *mAddressSelectorUi;
  AddressProvider *_provider;
  AddressSortProxyModel *mProxyModel;
  QTreeView *_addressTreeView;
  KraftContactViewer *_contactViewer;

};

#endif // AKONADIADDRESSSELECTOR_H
