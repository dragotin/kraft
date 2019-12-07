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

#ifdef HAVE_AKONADI
#include <kjob.h>
#include <AkonadiCore/session.h>
#include <AkonadiCore/changerecorder.h>
#include <akonadi/contact/contactstreemodel.h>
#endif

class QAbstractItemModel;
class AddressItemModel;

// An akonadi based provider.
class AddressProviderPrivate : public QObject
{
    Q_OBJECT
public:
    AddressProviderPrivate( QObject* parent = 0 );

    // initialize the backend and return true if that worked.
    bool init();
    // returns the result of the init process later on
    bool backendUp();
    QString backendName() const;

    bool lookupAddressee( const QString& uid );
    QString formattedAddress( const KContacts::Addressee& ) const;

    QAbstractItemModel *model();

    KContacts::Addressee getAddressee(int row, const QModelIndex &parent);
    KContacts::Addressee getAddressee(const QModelIndex& indx);

    bool isSearchOngoing(const QString& uid);
#ifdef HAVE_AKONADI
public slots:
    void searchResult( KJob* );
#endif
signals:
    //
    void addresseeFound( const QString&, const KContacts::Addressee& );
    void addresseeNotFound( const QString& );

    // error message when looking up the address for a UID
    void lookupError( const QString&, const QString&);

    // emitted when the search is finished, even if there was no result.
    void finished( int );

private:
    QSet<QString>        mUidSearches;
    bool                 _akonadiUp;

#ifdef HAVE_AKONADI
    Akonadi::Session *mSession;
    Akonadi::ChangeRecorder* mMonitor; // FIXME: Must static somehow
    Akonadi::ContactsTreeModel *_model;
#else
    void *mSession;
    void *mMonitor;
    void *_model;
#endif
};

#endif // ADDRESSPROVIDER_AKONADI_H
