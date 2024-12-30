/***************************************************************************
                          addressprovider.h
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

#ifndef ADDRESSPROVIDER_AKONADI_H
#define ADDRESSPROVIDER_AKONADI_H

#include <QSet>
#include <QLatin1String>

#include <kcontacts/addressee.h>

#include "addressproviderprivate.h"

#ifdef HAVE_AKONADI
#include <kjob.h>

#include <Akonadi/ContactsTreeModel>
#include <Akonadi/Session>
#include <Akonadi/ChangeRecorder>
#endif // HAVE_AKONADI


class QAbstractItemModel;
class AddressItemModel;

// An akonadi based provider.
class AddressProviderAkonadi : public AddressProviderPrivate
{
public:
    AddressProviderAkonadi( QObject* parent = 0 );

    // initialize the backend and return true if that worked.
    bool init();
    // returns the result of the init process later on
    bool backendUp() override;
    QString backendName() const override;

    bool lookupAddressee( const QString& uid ) override;

    QAbstractItemModel *model() override;

    KContacts::Addressee getAddressee(int row, const QModelIndex &parent) override;
    KContacts::Addressee getAddressee(const QModelIndex& indx) override;

    bool isSearchOngoing(const QString& uid) override;

public Q_SLOTS:
    void searchResult( KJob* );

private:
    bool                 _akonadiUp;

#ifdef HAVE_AKONADI
    Akonadi::Session *mSession;
    Akonadi::ChangeRecorder* mMonitor; // FIXME: Must static somehow
#else
    void *mSession;
    void *mMonitor;
#endif
};

#endif // ADDRESSPROVIDER_AKONADI_H
