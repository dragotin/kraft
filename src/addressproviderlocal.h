/***************************************************************************
    addressprovider_local.cpp  - Addressprovider reading a directory
                             -------------------
    begin                : Dec. 2024
    copyright            : (C) 2024 by Klaas Freitag
    email                : kraft@freisturz.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADDRESSPROVIDER_LOCAL_H
#define ADDRESSPROVIDER_LOCAL_H

#include <QSet>

#include <kcontacts/addressee.h>

#include "addressproviderprivate.h"

class QAbstractItemModel;
class AddressItemModel;

// An akonadi based provider.
class AddressProviderLocal : public AddressProviderPrivate
{
    Q_OBJECT
public:
    AddressProviderLocal(const QString& baseDir, QObject* parent = 0);

    bool init();

    // initialize the backend and return true if that worked.
    // returns the result of the init process later on
    bool backendUp() override;
    QString backendName() const override;

    bool lookupAddressee( const QString& uid ) override;

    QAbstractItemModel* model() override;

    KContacts::Addressee getAddressee(int row, const QModelIndex &parent) override;
    KContacts::Addressee getAddressee(const QModelIndex& indx) override;

    bool isSearchOngoing(const QString& uid) override;

private Q_SLOTS:
    void searchResult(const QString& uid);

private:
    QString              _baseDir;
};

#endif // ADDRESSPROVIDER_AKONADI_H
