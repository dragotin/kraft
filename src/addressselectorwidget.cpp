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

class AddressModel : public QAbstractItemModel
{
public:
    AddressModel(QObject *parent) :
        QAbstractItemModel(parent), _dataRetrieved(false) {

    }

    int columnCount(const QModelIndex &parent) const {
        return 2;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const
    {
        if( role == Qt::DisplayRole && orientation == Qt::Horizontal ) {
            if( section == 0 )
                return i18n("Customer");
            else if( section == 1 ) {
                return i18n("City");
            }
        }
        return QVariant();
    }

    QVariant data(const QModelIndex &index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        if (role != Qt::DisplayRole)
            return QVariant();

        // FIXME get the customerdata out here
        if( _dataRetrieved ) {

        }
    }

private:
    int _dataRetrieved;
};

/* ------------------------------------------------------------------------------ */

AddressSelectorWidget::AddressSelectorWidget(QWidget *parent, bool /* showText */) :
  QWidget(parent)
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

  connect(butCreateContact,SIGNAL(clicked()),SLOT( slotCreateNewContact()));
  connect(mButEditContact,SIGNAL(clicked()),SLOT(slotEditContact()));
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

