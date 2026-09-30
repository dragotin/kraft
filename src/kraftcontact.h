/***************************************************************************
                  kraftcontact.h - A contact as Kraft needs it
                             -------------------
    begin                : Sept. 2026
    copyright            : (C) 2026 by Klaas Freitag
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

#ifndef KRAFTCONTACT_H
#define KRAFTCONTACT_H

#include <QMap>
#include <QObject>
#include <QString>

#include <KContacts/Addressee>

#include "kraftobj.h"

class AddressProvider;

/**
 * @brief The KraftContact class - a contact the way Kraft needs it.
 *
 * A contact consists of two parts: the address itself, which lives in an address
 * book backend and is handed around as a KContacts::Addressee, and the data that
 * Kraft needs but a vCard has no room for, ie. the tax registration of the own
 * company.
 *
 * The address book part is never written by this class. The vCards stay where
 * their backend keeps them, and which backend that is - Akonadi or a directory of
 * vCard files - is hidden behind AddressProvider.
 *
 * The Kraft part is stored as one small XML file per contact in the `contacts`
 * directory of the Kraft v2 dir. The file references the address by the uid it
 * has in the backend and carries the Kraft data in the attribute map that comes
 * with KraftObj.
 *
 * The file is named after the backend uid of the address, so the contact for a
 * document can be loaded straight from its KraftDoc::addressUid() without an
 * index in between. The own identity sets a storage key of its own instead,
 * because it may be an address typed by hand that has no uid in any backend.
 */
class KraftContact : public QObject, public KraftObj
{
    Q_OBJECT
public:
    /* Where the address behind this contact came from. */
    enum class Source {
        Unknown,
        Manual,  // typed by the user, not part of any address book
        Backend  // looked up in the address book backend
    };

    /* The names of the Kraft specific attributes. They are also the names of the
     * template variables of the contact, so that there is only one name per value
     * for the user to learn.
     */
    static const QString VatId;           // BT-31
    static const QString TaxNumber;       // BT-32
    static const QString RegistrationId;  // BT-30
    static const QString LegalForm;       // BT-33
    static const QString AccountName;
    static const QString Iban;            // BT-84
    static const QString Bic;

    explicit KraftContact(QObject *parent = nullptr);
    // The contact of the address with this backend uid, ie. KraftDoc::addressUid().
    explicit KraftContact(const QString& backendUid, QObject *parent = nullptr);

    /* The uid the address has in the address book backend, ie. the one a document
     * keeps in KraftDoc::addressUid(). Empty if the address was typed by hand.
     */
    QString backendUid() const { return _backendUid; }
    void setBackendUid(const QString& uid);

    /* The name of the file this contact is stored in, without the extension.
     * Defaults to the backend uid, which is what every contact but the own
     * identity uses. Setting it explicitly decouples the storage from the address,
     * so that the contact stays findable when its address is swapped or missing.
     */
    QString storageKey() const;
    void setStorageKey(const QString& key);

    KContacts::Addressee addressee() const { return _addressee; }
    void setAddressee(const KContacts::Addressee& contact);

    Source source() const { return _source; }
    void setSource(Source s) { _source = s; }

    // Is there an address book backend that can be asked at all?
    bool hasBackend();

    /* Look up the address of backendUid() in the address book backend.
     *
     * Asynchronous: connect to addresseeLoaded(), which is emitted in any case,
     * with an empty addressee if the lookup failed. errorMsg() tells why.
     */
    void lookupAddressee();
    QString errorMsg(const QString& uid);

    /* The Kraft side of the contact, see the class comment.
     *
     * Both do nothing without a storage key, ie. for a contact that has neither a
     * backend uid nor a key of its own.
     */
    bool load();
    bool save();

    // The file this contact is stored in, empty if it has no storage key.
    QString storageFile() const;

Q_SIGNALS:
    /* The result of lookupAddressee(). The uid is the one that was looked up, the
     * contact is empty if it was not found.
     */
    void addresseeLoaded(const QString& uid, const KContacts::Addressee& contact);

private Q_SLOTS:
    void slotAddresseeFound(const QString& uid, const KContacts::Addressee& contact);

private:
    AddressProvider *addressProvider();

    // Backend uids are not necessarily valid file names, ie. the ones of Akonadi.
    static QString keyToFileName(const QString& key);

    AddressProvider     *_addressProvider;
    KContacts::Addressee _addressee;
    QString              _backendUid;
    QString              _storageKey;
    Source               _source;
};

#endif // KRAFTCONTACT_H
