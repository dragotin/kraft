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

#include <kcontacts/addressee.h>

#include <kjob.h>

#include <AkonadiCore/session.h>
#include <AkonadiCore/changerecorder.h>
#include <akonadi/contact/contactstreemodel.h>


class QAbstractItemModel;
class AddressItemModel;

// An akonadi based provider.
class AddressProviderPrivate : public QObject
{
    Q_OBJECT
public:
    AddressProviderPrivate( QObject* parent = 0 );

    void lookupAddressee( const QString& uid );
    QString formattedAddress( const KContacts::Addressee& ) const;

    QAbstractItemModel *model();

    KContacts::Addressee getAddressee(int row, const QModelIndex &parent);
    KContacts::Addressee getAddressee(const QModelIndex& indx);

public slots:
    void searchResult( KJob* );

signals:
    //
    void addresseeFound( const QString&, const KContacts::Addressee& );

    // error message when looking up the address for a UID
    void lookupError( const QString&, const QString&);

    // emitted when the search is finished, even if there was no result.
    void finished( int );

private:
    QSet<QString>        mUidSearches;

    Akonadi::Session *mSession;
    Akonadi::ChangeRecorder* mMonitor; // FIXME: Must static somehow
    Akonadi::ContactsTreeModel *_model;
};

#endif // ADDRESSPROVIDER_H