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

#ifdef HAVE_AKONADI
#include <akonadi_version.h>

#if AKONADI_VERSION >= QT_VERSION_CHECK(5,20,0)
#include <AkonadiContact/akonadi-contact_version.h>
#else
#define AKONADICONTACT_VERSION AKONADI_VERSION
#endif
#if AKONADICONTACT_VERSION >= QT_VERSION_CHECK(5, 24, 0)
#include <AkonadiContactEditor/Akonadi/ContactViewer>
#include <AkonadiContactEditor/Akonadi/ContactEditorDialog>
#elif AKONADICONTACT_VERSION >= QT_VERSION_CHECK(5, 20, 0)
#include <AkonadiContact/Akonadi/ContactViewer>
#include <AkonadiContact/Akonadi/ContactEditorDialog>
#else
#include <Akonadi/Contact/ContactViewer>
#include <Akonadi/Contact/ContactEditorDialog>
#endif
#endif

#include <kcontacts/addressee.h>

#include "addressprovider.h"

using namespace KContacts;

class QLabel;
class QTreeView;
class QuickSearchWidget;
class QItemSelectionModel;
class QuickSearchWidget;
class QPushButton;

/* =============================================================== */


class KraftContactViewer : public QWidget
{
    Q_OBJECT
    public:

    explicit KraftContactViewer(QWidget *parent = 0);

    void setContact( const KContacts::Addressee& contact);

private:
#ifdef HAVE_AKONADI
#if AKONADICONTACT_VERSION >= QT_VERSION_CHECK(5, 24, 0)
    ContactEditor::ContactViewer *_contactViewer;
#else
    Akonadi::ContactViewer *_contactViewer;
#endif
#endif
};

class AddressSortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    AddressSortProxyModel(AddressProvider *provider, QObject *parent = 0);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

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

    bool backendUp() const;

Q_SIGNALS:
  void addressSelected(KContacts::Addressee);

public Q_SLOTS:
  void saveState();

protected Q_SLOTS:
  void slotCreateNewContact();
  void slotEditContact();
  void restoreState();
  void slotFilterTextChanged( const QString& filter);

private Q_SLOTS:
  void slotAddresseeSelected(QModelIndex index);

private:
  void setupUi();

  QPushButton       *mButEditContact;

  AddressProvider *_provider;
  AddressSortProxyModel *mProxyModel;
  QTreeView *_addressTreeView;
  KraftContactViewer *_contactViewer;
#ifdef HAVE_AKONADI
#if AKONADICONTACT_VERSION >= QT_VERSION_CHECK(5, 24, 0)
    ContactEditor::ContactEditorDialog *_addressEditor;
#else
    Akonadi::ContactEditorDialog *_addressEditor;
#endif
#endif
};

#endif // AKONADIADDRESSSELECTOR_H
