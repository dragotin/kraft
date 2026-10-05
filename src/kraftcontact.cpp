/***************************************************************************
                 kraftcontact.cpp - A contact as Kraft needs it
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

#include "kraftcontact.h"
#include "addressprovider.h"
#include "defaultprovider.h"
#include "stringutil.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QRegularExpression>
#include <QSaveFile>

using namespace Qt::StringLiterals;

const QString KraftContact::VatId          = u"VATID"_s;
const QString KraftContact::TaxNumber      = u"TAXNUMBER"_s;
const QString KraftContact::RegistrationId = u"REGISTRATIONID"_s;
const QString KraftContact::LegalForm      = u"LEGALFORM"_s;
const QString KraftContact::AccountName    = u"ACCOUNTNAME"_s;
const QString KraftContact::Iban           = u"IBAN"_s;
const QString KraftContact::Bic            = u"BIC"_s;

KraftContact::KraftContact(QObject *parent)
    : QObject{parent},
      KraftObj(),
      _addressProvider{nullptr},
      _source{Source::Unknown}
{

}

KraftContact::KraftContact(const QString& backendUid, QObject *parent)
    : KraftContact(parent)
{
    _backendUid = backendUid;
}

/* The backend uid is the key of an ordinary contact: a document points to its
 * customer with it, so the contact can be loaded without looking anything up.
 */
QString KraftContact::storageKey() const
{
    if (_storageKey.isEmpty())
        return _backendUid;

    return _storageKey;
}

void KraftContact::setStorageKey(const QString& key)
{
    _storageKey = key;
}

void KraftContact::setBackendUid(const QString& uid)
{
    if (_backendUid != uid) {
        _backendUid = uid;
        setModified();
    }
}

void KraftContact::setAddressee(const KContacts::Addressee& contact)
{
    // The addressee itself belongs to the backend and is not stored here, so this
    // does not make the contact modified.
    _addressee = contact;
}

/* Business data of the contact that a vCard has no room for. The VAT id is
 * squeezed on the way out because BR-CO-9 wants it prefixed with the ISO 3166-1
 * alpha-2 country code and without spaces, not as "DE 123 456 789" as a user may
 * well type it.
 */
QMap<QString, QString> KraftContact::businessData() const
{
    QMap<QString, QString> re;

    QString vatId = stringAttribute(VatId);
    vatId.remove(QRegularExpression(u"\\s"_s));
    re.insert(VatId, vatId.toUpper());

    const QStringList plain{TaxNumber, RegistrationId, LegalForm, AccountName, Iban, Bic};
    for (const QString& name : plain) {
        re.insert(name, stringAttribute(name));
    }

    return re;
}

/* Escaped here, once, so that no template has to remember to do it. */
QVariantHash KraftContact::businessVariantHash() const
{
    QVariantHash re;

    const auto data = businessData();
    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        re.insert(it.key(), it.value().toHtmlEscaped());
    }
    return re;
}

/* The address book backend, created on first use. Going through here rather than
 * touching _addressProvider directly makes sure the provider is never in use
 * without its result signal connected.
 */
AddressProvider *KraftContact::addressProvider()
{
    if (!_addressProvider) {
        _addressProvider = new AddressProvider(this);
        connect(_addressProvider, &AddressProvider::lookupResult,
                this, &KraftContact::slotAddresseeFound);
    }
    return _addressProvider;
}

bool KraftContact::hasBackend()
{
    return addressProvider()->backendUp();
}

void KraftContact::lookupAddressee()
{
    if (_backendUid.isEmpty()) {
        qDebug() << "Can not look up a contact without a backend uid";
        Q_EMIT addresseeLoaded(_backendUid, KContacts::Addressee());
        return;
    }

    _source = Source::Backend;

    AddressProvider *provider = addressProvider();
    const AddressProvider::LookupState state = provider->lookupAddressee(_backendUid);
    switch (state) {
    case AddressProvider::LookupFromCache:
        // The address is known already, no signal will come in for it.
        slotAddresseeFound(_backendUid, provider->getAddresseeFromCache(_backendUid));
        break;
    case AddressProvider::LookupNotFound:
    case AddressProvider::ItemError:
    case AddressProvider::BackendError:
        // The caller has to know that the lookup is over, otherwise it waits for a
        // signal that never arrives.
        slotAddresseeFound(_backendUid, KContacts::Addressee());
        break;
    case AddressProvider::LookupOngoing:
    case AddressProvider::LookupStarted:
        // Nothing to do but wait for the signal.
        break;
    }
}

void KraftContact::slotAddresseeFound(const QString& uid, const KContacts::Addressee& contact)
{
    setAddressee(contact);
    Q_EMIT addresseeLoaded(uid, contact);
}

QString KraftContact::errorMsg(const QString& uid)
{
    if (!_addressProvider)
        return QString();

    return _addressProvider->errorMsg(uid);
}

/* Backend uids are opaque strings. The ones of a vCard directory are plain uuids,
 * but Akonadi hands out uids that are no good as a file name. Those are reduced to
 * the harmless characters plus a hash of the original, which keeps the name unique.
 */
QString KraftContact::keyToFileName(const QString& key)
{
    QString re;
    bool replaced{false};

    for (const QChar c : key) {
        if ((c >= u'a' && c <= u'z') || (c >= u'A' && c <= u'Z') || (c >= u'0' && c <= u'9') ||
            c == u'-' || c == u'_') {
            re.append(c);
        } else {
            re.append(u'_');
            replaced = true;
        }
    }

    if (replaced) {
        const QByteArray hash = QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Md5);
        re.append(u'-');
        re.append(QString::fromLatin1(hash.toHex().left(8)));
    }
    return re;
}

QString KraftContact::storageFile() const
{
    const QString contactsDir = DefaultProvider::self()->kraftV2Dir(DefaultProvider::KraftV2Dir::Contacts);
    const QString key = storageKey();

    if (key.isEmpty() || contactsDir.isEmpty())
        return QString();

    return QStringLiteral("%1/%2.xml").arg(contactsDir, keyToFileName(key));
}

bool KraftContact::load()
{
    const QString file = storageFile();
    if (file.isEmpty())
        return false;

    QFile f(file);
    if (!f.exists()) // A contact Kraft has no data of yet, which is not an error.
        return false;

    if (!f.open(QIODevice::ReadOnly)) {
        qDebug() << "Can not open the contact file" << file;
        return false;
    }

    QDomDocument xmldoc;
    const auto res = xmldoc.setContent(&f);
    f.close();

    if (!res) {
        qDebug() << "Can not parse the contact file" << file << res.errorMessage;
        return false;
    }

    QDomElement root = xmldoc.documentElement();
    QDomElement kobj = root.firstChildElement(QStringLiteral("kobj"));
    if (kobj.isNull()) {
        qDebug() << "The contact file has no kobj element:" << file;
        return false;
    }
    parseKobjXml(kobj);
    _backendUid = KraftXml::childElemText(root, "backendUid");
    setModified(false);

    return true;
}

bool KraftContact::save()
{
    const QString file = storageFile();
    if (file.isEmpty()) {
        qDebug() << "Can not save a contact without a storage key or contacts dir";
        return false;
    }

    // Kraft dirs that were created before there were contacts have no such dir yet.
    const QString contactsDir = DefaultProvider::self()->kraftV2Dir(DefaultProvider::KraftV2Dir::Contacts);
    QDir dir(contactsDir);
    if (!dir.exists() && !dir.mkpath(dir.absolutePath())) {
        qDebug() << "Can not create the contacts dir" << dir.absolutePath();
        return false;
    }

    createUuid();  // only creates one if there is none yet
    setModified(); // to stamp it with the time of this save

    QDomDocument xmldoc(QStringLiteral("kraftcontact"));
    QDomProcessingInstruction instr =
        xmldoc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
    xmldoc.appendChild(instr);

    QDomElement root = xmldoc.createElement(QStringLiteral("kraftcontact"));
    xmldoc.appendChild(root);
    root.appendChild(kobjXml(xmldoc, QStringLiteral("kobj")));
    root.appendChild(KraftXml::textElement(xmldoc, QStringLiteral("backendUid"), _backendUid));

    bool re{false};
    QSaveFile f(file);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        re = f.write(xmldoc.toString().toUtf8()) > -1;
        if (re) {
            re = f.commit();
        }
    }

    if (!re) {
        qDebug() << "Can not write the contact file" << file;
        return false;
    }

    setModified(false);
    return true;
}
