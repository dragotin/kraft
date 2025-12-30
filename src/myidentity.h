/***************************************************************************
                             myidentity.h
                             -------------------
    begin                : Oct. 2024
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

#ifndef MYIDENTITY_H
#define MYIDENTITY_H

#include <QObject>

#include <KContacts/Addressee>

#include "ui_identity.h"

class AddressProvider;

/**
 * @brief The MyIdentity class
 *
 * The identity can be stored in two different ways:
 * 1. There is just a UUID in the settings file stored under userUid(),
 *    which contains the id under which the own identify can be found in
 *    the addressbook through the backend.
 * 2. If the id is non existent or empty, the identity is read from a file
 *    stored in a specific path. It is written by the prefsdialog.
 */

class MyIdentity : public QObject
{
    Q_OBJECT
public:
    explicit MyIdentity(QObject *parent = nullptr);

    enum class Source {
        Unknown,
        Manual,
        Backend
    };

    static KContacts::Addressee UIToAddressee(Ui::manualOwnIdentity ui);

    void load();

    // One of the parameters need to be empty when calling this method
    void save(const QString& uuid, const KContacts::Addressee& contact = KContacts::Addressee());

    QString identityFile();

    // returns the addressee that was found on the last attempt to look up the own identity.
    // If there was no call to load before, the returned addressee is obviously empty.
    KContacts::Addressee contact() const;

    MyIdentity::Source source() const;

    QString errorMsg(const QString& uid);

    bool hasBackend();
Q_SIGNALS:

    // final signal after the contact could be loaded
    void myIdentityLoaded(const QString& uuid, const KContacts::Addressee& contact);

private Q_SLOTS:
    void slotAddresseeFound(const QString& uid, const KContacts::Addressee &contact);

private:
    AddressProvider *_addressProvider;
    static KContacts::Addressee _myContact;
    Source _source;
};

#endif
