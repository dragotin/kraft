/***************************************************************************
    AddressSelectorWidget - Address Selection Widget based on Akonadi
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

#include "kraftsettings.h"
#include "addressselectorwidget.h"
#include "addressprovider.h"

#include <QtGui>
#include <QHeaderView>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QDebug>

#include <klocalizedstring.h>

#include <kcontacts/addressee.h>
#include <kcontacts/contactgroup.h>

/* ==================================================================== */
AddressSortProxyModel::AddressSortProxyModel(AddressProvider *provider, QObject *parent)
    : QSortFilterProxyModel(parent),
      _provider(provider)
{
    // setFilterRole(ActivityItemDelegate::ActionTextRole);
    setFilterKeyColumn(0);
}

#if 0
bool AddressSortProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    QVariant leftData = sourceModel()->data(left);
    QVariant rightData = sourceModel()->data(right);

    if (leftData.type() == QVariant::DateTime) {
        return leftData.toDateTime() < rightData.toDateTime();
    } else {
        qDebug() << "OOOOO " << endl;
    }
    return true;
}
#endif


static bool addressMatchesFilter(const KContacts::Address &address, const QString &filterString)
{
    if (address.street().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (address.locality().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (address.region().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (address.postalCode().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (address.country().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (address.label().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (address.postOfficeBox().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    return false;
}

static bool contactMatchesFilter(const KContacts::Addressee &contact, const QString &filterString)
{
    if (contact.assembledName().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (contact.formattedName().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (contact.nickName().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (contact.birthday().toString().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    const KContacts::Address::List addresses = contact.addresses();
    int count = addresses.count();
    for (int i = 0; i < count; ++i) {
        if (addressMatchesFilter(addresses.at(i), filterString)) {
            return true;
        }
    }

    const KContacts::PhoneNumber::List phoneNumbers = contact.phoneNumbers();
    count = phoneNumbers.count();
    for (int i = 0; i < count; ++i) {
        if (phoneNumbers.at(i).number().contains(filterString, Qt::CaseInsensitive)) {
            return true;
        }
    }

    const QStringList emails = contact.emails();
    count = emails.count();
    for (int i = 0; i < count; ++i) {
        if (emails.at(i).contains(filterString, Qt::CaseInsensitive)) {
            return true;
        }
    }

    const QStringList categories = contact.categories();
    count = categories.count();
    for (int i = 0; i < count; ++i) {
        if (categories.at(i).contains(filterString, Qt::CaseInsensitive)) {
            return true;
        }
    }

    if (contact.mailer().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (contact.title().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (contact.role().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (contact.organization().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (contact.department().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (contact.note().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    if (contact.url().url().url().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    const QStringList customs = contact.customs();
    count = customs.count();
    for (int i = 0; i < count; ++i) {
        if (customs.at(i).contains(filterString, Qt::CaseInsensitive)) {
            return true;
        }
    }

    return false;
}

bool contactGroupMatchesFilter(const KContacts::ContactGroup &group, const QString &filterString)
{
    if (group.name().contains(filterString, Qt::CaseInsensitive)) {
        return true;
    }

    const uint count = group.dataCount();
    for (uint i = 0; i < count; ++i) {
        if (group.data(i).name().contains(filterString, Qt::CaseInsensitive)) {
            return true;
        }
        if (group.data(i).email().contains(filterString, Qt::CaseInsensitive)) {
            return true;
        }
    }

    return false;
}

bool AddressSortProxyModel::filterAcceptsRow(int row, const QModelIndex &parent) const
{
    KContacts::Addressee contact = _provider->getAddressee(row, parent);

    if( contact.isEmpty() ) {
        return true;
    } else {
        const QString p = filterRegExp().pattern();
        return contactMatchesFilter(contact, p );
    }

    return true;
}

/* ------------------------------------------------------------------------------ */

AddressSelectorWidget::AddressSelectorWidget(QWidget *parent, bool /* showText */)
    : QWidget(parent)
{
    setupGui();
}


AddressSelectorWidget::~AddressSelectorWidget()
{
}

void AddressSelectorWidget::setupGui()
{
  QVBoxLayout *vbox = new QVBoxLayout;
  setLayout( vbox );

  _provider = new AddressProvider(this);

  mAddressSelectorUi = new Ui::AddressSelectorWidget;
  QWidget *w = new QWidget;
  mAddressSelectorUi->setupUi( w );
  vbox->addWidget( w );

  QHBoxLayout *hboxBot = new QHBoxLayout;
  hboxBot->addStretch(4);
  vbox->addLayout( hboxBot );
  mButEditContact = new QPushButton(i18n("Edit Contact..."));
  mButEditContact->setToolTip( i18n("Edit the currently selected contact" ));
  mButEditContact->setEnabled( false );
  hboxBot->addWidget( mButEditContact );
  QPushButton *butCreateContact = new QPushButton(i18n("New Contact..."));
  butCreateContact->setToolTip( i18n("Create a new Contact" ) );
  hboxBot->addWidget( butCreateContact );

  mProxyModel = new AddressSortProxyModel(_provider, this);
  mProxyModel->setSourceModel(_provider->model());
  mAddressSelectorUi->mAddressList->setModel(mProxyModel);
  mProxyModel->sort(0);

  connect(butCreateContact,SIGNAL(clicked()),SLOT( slotCreateNewContact()));
  connect(mButEditContact,SIGNAL(clicked()),SLOT(slotEditContact()));

  mAddressSelectorUi->mSearchEdit->setClearButtonEnabled(true);
  connect(mAddressSelectorUi->mSearchEdit, SIGNAL(textChanged(QString)),
          SLOT(slotFilterTextChanged(QString)));


}

void AddressSelectorWidget::slotFilterTextChanged( const QString& filter)
{
    qDebug() << "Filter: " << filter;
    mProxyModel->setFilterRegExp(QRegExp(filter, Qt::CaseInsensitive, QRegExp::RegExp));
    // mProxyModel.setFilterFixedString(filter);
}

void AddressSelectorWidget::restoreState()
{
}

void AddressSelectorWidget::saveState()
{
}


void AddressSelectorWidget::slotCreateNewContact()
{
    // if( mContactsEditor ) delete( mContactsEditor );

    // FIXME
    //  mContactsEditor = new Akonadi::ContactEditorDialog( Akonadi::ContactEditorDialog::CreateMode, this );
    //   mContactsEditor->show();
}

void AddressSelectorWidget::slotEditContact()
{
  if( mAddressSelectorUi->mAddressList->selectionModel()->hasSelection() ) {
#if 0
      QModelIndex index = mItemView->selectionModel()->currentIndex();
    if ( index.isValid() ) {
      const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
      if ( item.isValid() && item.hasPayload<KContacts::Addressee>() ) {
        if( mContactsEditor ) delete( mContactsEditor );
        mContactsEditor = new Akonadi::ContactEditorDialog( Akonadi::ContactEditorDialog::EditMode, this );
        mContactsEditor->setContact( item );
        mContactsEditor->show();
      }
    }
#endif
  }
}

void AddressSelectorWidget::slotItemActivated( const QModelIndex& index )
{
    if ( index.isValid() ) {
      mButEditContact->setEnabled( true );
    } else {
      // qDebug () << "No address was selected!";
      mButEditContact->setEnabled( false );
    }
}

