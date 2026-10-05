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
#include <QMap>
#include <QVariantHash>

#include <KContacts/Addressee>

#include "kraftcontact.h"
#include "ui_identity.h"

/**
 * @brief The MyIdentity class
 *
 * The own identity is the KraftContact of the own company, see ownContact(). This
 * class is what loads and stores it.
 *
 * The address of the identity can come from two different places:
 * 1. There is just a UUID in the settings file stored under userUid(),
 *    which contains the id under which the own identify can be found in
 *    the addressbook through the backend.
 * 2. If the id is non existent or empty, the identity is read from a file
 *    stored in a specific path. It is written by the prefsdialog.
 *
 * The business data of the company, which no address book can hold, is stored
 * with the KraftContact either way.
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

    /* The own company as a KraftContact: the address, wherever it comes from, plus
     * the business data that no address book can hold.
     *
     * There is only one own identity, so this is the one instance of it, shared by
     * all MyIdentity objects. It is created and read from disk on first use.
     */
    static KraftContact *ownContact();

    void load();

    // One of the parameters need to be empty when calling this method
    void save(const QString& uuid, const KContacts::Addressee& contact = KContacts::Addressee());

    QString identityFile();

    /* The business data of the own company: tax registration, commercial register
     * and bank account. It does not fit into a vCard and is therefore stored with
     * the KraftContact of the own identity rather than in the contact itself.
     *
     * ownBusinessData() returns the plain values, trimmed and normalized. Use it
     * for every consumer that is not a template, ie. the EPC QR code, which is
     * read by banking apps and must carry the account holder name verbatim.
     */
    static QMap<QString, QString> ownBusinessData();

    /* The same data HTML escaped for the templates, merged into the `me` namespace.
     * Never feed this to anything but a template.
     */
    static QVariantHash ownBusinessVariantHash();

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
    // Takes over the business data of a Kraft that kept it in the settings file.
    static void migrateSettingsToContact(KraftContact *contact);

    static KraftContact *_ownContact;
};

#endif
