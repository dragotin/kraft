/***************************************************************************
    addressproviderprivate.cpp  - Addressprovider abstract private class
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

#ifndef ADDRESSPROVIDERPRIVATE_H
#define ADDRESSPROVIDERPRIVATE_H

#include <KContacts/Addressee>
#include <QObject>
#include <QAbstractItemModel>

class AddressProviderPrivate : public QObject
{
    Q_OBJECT

public:
    AddressProviderPrivate( QObject* parent = 0 );

    // returns the result of the init process later on
    virtual bool backendUp() = 0;
    virtual QString backendName() const = 0;

    virtual bool lookupAddressee( const QString& uid ) = 0;

    virtual QAbstractItemModel *model() = 0;

    virtual KContacts::Addressee getAddressee(int row, const QModelIndex &parent) = 0;
    virtual KContacts::Addressee getAddressee(const QModelIndex& indx) = 0;

    virtual bool isSearchOngoing(const QString& uid) = 0;

Q_SIGNALS:
    //
    void addresseeFound( const QString&, const KContacts::Addressee& );
    void addresseeNotFound( const QString& );

    // error message when looking up the address for a UID
    void lookupError( const QString&, const QString&);

    // emitted when the search is finished, even if there was no result.
    void finished( int );

protected:
    QSet<QString>        mUidSearches;
    std::unique_ptr<QAbstractItemModel>  _model;
};

#endif // ADDRESSPROVIDERPRIVATE_H
