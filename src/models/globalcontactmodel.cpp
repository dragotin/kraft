/*
    This file is part of KAddressBook.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "globalcontactmodel.h"

#include <akonadi/changerecorder.h>
#include <akonadi/contact/contactstreemodel.h>
#include <akonadi/entitydisplayattribute.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/session.h>
#include <kabc/addressee.h>
#include <kabc/contactgroup.h>
#include <klocale.h>

GlobalContactModel* GlobalContactModel::mInstance = 0;

GlobalContactModel::GlobalContactModel()
{
  mSession = new Akonadi::Session( "KAddressBook::GlobalContactSession" );

  Akonadi::ItemFetchScope scope;
  scope.fetchFullPayload( true );
  scope.fetchAttribute<Akonadi::EntityDisplayAttribute>();

  mMonitor = new Akonadi::ChangeRecorder;
  mMonitor->setSession(mSession);
  mMonitor->fetchCollection( true );
  mMonitor->setItemFetchScope( scope );
  mMonitor->setCollectionMonitored( Akonadi::Collection::root() );
  mMonitor->setMimeTypeMonitored( KABC::Addressee::mimeType(), true );
  mMonitor->setMimeTypeMonitored( KABC::ContactGroup::mimeType(), true );

  mModel = new Akonadi::ContactsTreeModel( mMonitor );
}

GlobalContactModel::~GlobalContactModel()
{
  delete mModel;
  delete mMonitor;
  delete mSession;
}

GlobalContactModel* GlobalContactModel::instance()
{
  if ( !mInstance )
    mInstance = new GlobalContactModel();

  return mInstance;
}

Akonadi::ContactsTreeModel* GlobalContactModel::model() const
{
  return mModel;
}
