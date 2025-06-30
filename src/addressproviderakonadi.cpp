/***************************************************************************
                          addressprovider.cpp  -
                             -------------------
    begin                : Fri Mar 4 2011
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

#include "addressproviderakonadi.h"
#include <kcontacts/contactgroup.h>

#include <QDebug>

#ifdef HAVE_AKONADI
#include <Akonadi/ContactSearchJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/Control>
#include <Akonadi/ServerManager>

using namespace Akonadi;
#endif

AddressProviderAkonadi::AddressProviderAkonadi( QObject *parent )
  :AddressProviderPrivate(parent),
    _akonadiUp(false),
    mSession(0),
    mMonitor(0)
{
    init();
}

bool AddressProviderAkonadi::init()
{
    _akonadiUp = false;
#ifdef HAVE_AKONADI
    if ( Akonadi::ServerManager::state() == Akonadi::ServerManager::Broken ) {
        // should be handled in Akonadi::Control::start().
        // See https://invent.kde.org/pim/akonadi/-/merge_requests/189
        qDebug() << "Akonadi broken: " << Akonadi::ServerManager::brokenReason();
    } else if (Akonadi::ServerManager::state() == Akonadi::ServerManager::Running) {
        qDebug() << "** Akonadi is already running";
        mSession = Session::defaultSession();
        _akonadiUp = true;
    } else if (Akonadi::Control::start()) {
        qDebug() << "Akonadi Started!";
        mSession = new Akonadi::Session("KraftSession", this);
        _akonadiUp = true;
    } else {
        qDebug() << "Akonadi Start failed";
    }
    if (_akonadiUp)
        qDebug() << "** Akonadi Session available.";

#endif
    return _akonadiUp;
}

QString AddressProviderAkonadi::backendName() const
{
#ifdef HAVE_AKONADI
    return QStringLiteral("Akonadi");
#else
    return QStringLiteral("Akonadi disabled by compile option!");
#endif
}


bool AddressProviderAkonadi::backendUp()
{
    return _akonadiUp;
}

bool AddressProviderAkonadi::isSearchOngoing(const QString& uid)
{
    return mUidSearches.contains(uid);
}

bool AddressProviderAkonadi::lookupAddressee( const QString& uid )
{
    if( uid.isEmpty() ) {
        qDebug() << "Invalid: UID to lookup is empty.";
        return false;
    }

    if( mUidSearches.contains( uid ) ) {
        // search is already running
        // qDebug () << "Search already underways!";^
        return false;
    }
#ifdef HAVE_AKONADI
    qDebug() << "Looking up uid" << uid << "in the Akonadi backend";
    Akonadi::ContactSearchJob *csjob = new Akonadi::ContactSearchJob( this );
    csjob->setLimit( 1 );
    csjob->fetchScope().fetchFullPayload();
    csjob->setQuery(ContactSearchJob::ContactUid , uid, ContactSearchJob::ExactMatch);

    // Store the lookup uid in an extra property
    csjob->setProperty("UID", uid);
    connect(csjob, &Akonadi::ItemFetchJob::result, this, &AddressProviderAkonadi::searchResult);

    mUidSearches.insert( uid );

    model();
    return true;
#endif
    return false;
}

#ifdef HAVE_AKONADI
void AddressProviderAkonadi::searchResult( KJob* job )
{
    if( !job ) return;

    const QString uid {job->property("UID").toString()};
    KContacts::Addressee contact;

    if( job->error() ) {
        // both uid and err message can be empty
        const QString errMsg = job->errorString();
        Q_EMIT lookupError(uid, errMsg );
        // qDebug () << "Address Search job failed: " << job->errorString();
    } else {
        Akonadi::ContactSearchJob *searchJob = qobject_cast<Akonadi::ContactSearchJob*>( job );
        if (searchJob) {
            const KContacts::Addressee::List contacts = searchJob->contacts();
            // qDebug () << "Found list of " << contacts.size() << " addresses as search result";

            if (contacts.size() > 0) {
                auto contact = contacts.at(0);

                // qDebug() << "Found uid search job for UID " << uid << " = " << contact.realName();
                Q_EMIT addresseeFound(uid, contact);
            } else {
                // qDebug() << "No search result for UID" << uid;
                Q_EMIT addresseeNotFound(uid);
            }
        }
    }

    // cleanup
    if(!uid.isEmpty()) {
        mUidSearches.remove( uid );
    }

    job->deleteLater();
}
#endif

QAbstractItemModel* AddressProviderAkonadi::model()
{
    if( !_akonadiUp ) {
        return 0;
    }
#ifdef HAVE_AKONADI
    Akonadi::ItemFetchScope scope;
    // fetch all content of the contacts, including images
    scope.fetchFullPayload( true );
    // fetch the EntityDisplayAttribute, which contains custom names and icons
    scope.fetchAttribute<Akonadi::EntityDisplayAttribute>();
    if( ! mMonitor ) {
        mMonitor = new Akonadi::ChangeRecorder;
        mMonitor->setSession( mSession );
        // include fetching the collection tree
        mMonitor->fetchCollection( true );
        // set the fetch scope that shall be used
        mMonitor->setItemFetchScope( scope );
        // monitor all collections below the root collection for changes
        mMonitor->setCollectionMonitored( Akonadi::Collection::root() );
        // list only contacts and contact groups
        mMonitor->setMimeTypeMonitored( KContacts::Addressee::mimeType(), true );
        mMonitor->setMimeTypeMonitored( KContacts::ContactGroup::mimeType(), true );
    }
    if( !_model ) {
        auto model = new Akonadi::ContactsTreeModel(mMonitor);
        Akonadi::ContactsTreeModel::Columns columns;
        columns << Akonadi::ContactsTreeModel::FullName;
        columns << Akonadi::ContactsTreeModel::HomeAddress;
        // columns << Akonadi::ContactsTreeModel::FamilyName;
        // columns << Akonadi::ContactsTreeModel::GivenName;
        model->setColumns( columns );

        _model.reset(model);
    }
    return _model.get();
#endif
    return 0;
}

KContacts::Addressee AddressProviderAkonadi::getAddressee(const QModelIndex& indx)
{
    KContacts::Addressee contact;
#ifdef HAVE_AKONADI
    if( indx.isValid() ) {
        const Akonadi::Item item = indx.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();

        if (item.hasPayload<KContacts::Addressee>()) {
            contact = item.payload<KContacts::Addressee>();
        }
    }
#endif
    return contact;

}

KContacts::Addressee AddressProviderAkonadi::getAddressee(int row, const QModelIndex &parent)
{
#ifdef HAVE_AKONADI
    const QModelIndex index = _model->index(row, 0, parent);

    return getAddressee(index);
#endif
    return KContacts::Addressee();
}
