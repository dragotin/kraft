/***************************************************************************
             exporterXRechnung  - Save Documents as XRechnung
                             -------------------
    begin                : Feb. 2022
    copyright            : (C) 2022 by Klaas Freitag
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

// include files for Qt
#include <QDebug>
#include <QDir>
#include <QTemporaryFile>
#include <QRegularExpression>
#include <QTimer>

#include "exportxrechnung.h"
#include "documentman.h"
#include "docposition.h"
#include "kraftdoc.h"
#include "kraftdb.h"
#include "doctype.h"
#include "format.h"
#include "addressprovider.h"
#include "documenttemplate.h"
#include "myidentity.h"
#include "defaultprovider.h"

#include <KLocalizedString>

ExporterXRechnung::ExporterXRechnung(QObject *parent)
    : QObject(parent),
      _validateWithSchema {false}
{
    mAddressProvider = new AddressProvider(this);
    connect(mAddressProvider, &AddressProvider::lookupResult,
            this, &ExporterXRechnung::slotAddresseeFound);

}

void ExporterXRechnung::setDueDate(const QDate& d)
{
    _dueDate = d;
}

void ExporterXRechnung::setBuyerRef(const QString& br)
{
    _buyerRef = br;
}

QString ExporterXRechnung::templateFile() const
{
    Q_ASSERT(!_docTypeStr.isEmpty());
    DocTypes dts;
    DocType dt = dts.get(_docTypeStr);

    const QString re = dt.xRechnungTemplate();

    return re;
}

bool ExporterXRechnung::exportDocument(const QString& uuid)
{
    _uuid = uuid;
    KraftDoc *doc = DocumentMan::self()->openDocumentByUuid(uuid);
    _docTypeStr = doc->docTypeStr();
    _error.clear();

    const QString clientUid = doc->addressUid();
    _customerContact = KContacts::Addressee();

    AddressProvider::LookupState state = mAddressProvider->lookupAddressee( clientUid );
    switch( state ) {
    case AddressProvider::LookupFromCache:
        _customerContact = mAddressProvider->getAddresseeFromCache(clientUid);
        break;
    case AddressProvider::LookupNotFound:
    case AddressProvider::ItemError:
    case AddressProvider::BackendError:
        // set an empty contact
        break;
    case AddressProvider::LookupOngoing:
    case AddressProvider::LookupStarted:
        // Not much to do, just wait and let the addressprovider
        // hit the slotAddresseFound
        return true;
    }

    QTimer::singleShot(0, this, &ExporterXRechnung::slotSkipLookup);
    return true;
}

void ExporterXRechnung::slotSkipLookup()
{
    slotAddresseeFound(QString(), _customerContact);
}

/* Check the seller data that the receiver of the invoice will reject it for if
 * missing. Without this, an incomplete configuration results in an XRechnung
 * that looks fine but gets refused by the validator of the recipient.
 */
QStringList ExporterXRechnung::missingSellerData(const KContacts::Addressee& myContact,
                                                 const QMap<QString, QString>& own) const
{
    QStringList re;

    if (myContact.organization().isEmpty() && myContact.realName().isEmpty())
        re << i18n("Company name (BT-27)");
    if (myContact.realName().isEmpty())
        re << i18n("Contact name (BT-41)");
    // BT-42 is required, the template falls back to the mobile if there is no work phone.
    if (myContact.phoneNumber(KContacts::PhoneNumber::Work).number().isEmpty() &&
        myContact.phoneNumber(KContacts::PhoneNumber::Cell).number().isEmpty())
        re << i18n("Contact phone number (BT-42)");
    // The email is both the contact address and the electronic address BT-34.
    if (myContact.preferredEmail().isEmpty())
        re << i18n("Contact email address (BT-43/BT-34)");
    // Either of the two is enough, both on a german invoice (§14 UStG) and for the
    // EN16931 rule BR-S-02.
    const QString vatId = own.value(QStringLiteral("VATID"));
    if (vatId.isEmpty() && own.value(QStringLiteral("TAXNUMBER")).isEmpty())
        re << i18n("VAT identifier or tax number (BT-31/BT-32)");

    // BR-CO-9: the VAT identifier has to start with the country code, ie. DE123456789.
    if (!vatId.isEmpty() &&
        !QRegularExpression(QStringLiteral("^[A-Z]{2}[A-Z0-9]+$")).match(vatId).hasMatch())
        re << i18n("VAT identifier with a leading country code, ie. DE123456789 (BT-31)");
    if (own.value(QStringLiteral("IBAN")).isEmpty())
        re << i18n("Bank account IBAN (BT-84)");

    KContacts::Address addr = myContact.address(KContacts::Address::Pref);
    if (addr.isEmpty())
        addr = myContact.address(KContacts::Address::Work);
    if (addr.isEmpty())
        addr = myContact.address(KContacts::Address::Home);
    if (addr.isEmpty())
        addr = myContact.address(KContacts::Address::Postal);

    if (addr.street().isEmpty())
        re << i18n("Street of the address (BT-35)");
    if (addr.locality().isEmpty())
        re << i18n("City of the address (BT-37)");
    if (addr.postalCode().isEmpty())
        re << i18n("Post code of the address (BT-38)");

    return re;
}

/* The customer side of the invoice. The electronic address BT-49 is mandatory for
 * an XRechnung and has no sensible default, so an export without it has to fail
 * rather than produce an invoice the recipient rejects.
 */
QStringList ExporterXRechnung::missingBuyerData(const KContacts::Addressee& customer) const
{
    QStringList re;

    if (customer.isEmpty()) {
        // No contact at all, ie. the document was written with a manually typed
        // address. Naming every single field would not help here.
        re << i18n("the whole customer address. The document has no contact from the "
                   "address book assigned");
        return re;
    }

    if (customer.realName().isEmpty() && customer.organization().isEmpty())
        re << i18n("Customer name (BT-44)");
    // BT-49, mandatory for an XRechnung.
    if (customer.preferredEmail().isEmpty())
        re << i18n("Customer email address, used as the electronic address (BT-49)");

    KContacts::Address addr = customer.address(KContacts::Address::Pref);
    if (addr.isEmpty())
        addr = customer.address(KContacts::Address::Work);
    if (addr.isEmpty())
        addr = customer.address(KContacts::Address::Home);
    if (addr.isEmpty())
        addr = customer.address(KContacts::Address::Postal);

    if (addr.street().isEmpty())
        re << i18n("Customer street (BT-50)");
    if (addr.locality().isEmpty())
        re << i18n("Customer city (BT-52)");
    if (addr.postalCode().isEmpty())
        re << i18n("Customer post code (BT-53)");

    return re;
}

void ExporterXRechnung::slotAddresseeFound(const QString& uid, const KContacts::Addressee& contact)
{
    Q_UNUSED(uid)
    MyIdentity identity;
    const KContacts::Addressee myContact = identity.contact();
    // now the three pillars archDoc, myContact and mCustomerContact are defined.

    // The raw values, so the checks below see what the user actually entered.
    const QMap<QString, QString> own = MyIdentity::ownBusinessData();

    // Not part of missingSellerData() because the fix is not in the settings but
    // in the locale Kraft is started with.
    if (own.value(QStringLiteral("COUNTRYCODE")).isEmpty()) {
        _error = i18n("The country code of the seller address (BT-40) can not be derived from the "
                      "locale %1 that Kraft runs under. Please start Kraft with a locale that "
                      "has a country, ie. de_DE instead of C.",
                      DefaultProvider::self()->locale()->name());
        Q_EMIT xRechnungTmpFile(QString());
        return;
    }

    const QStringList missing = missingSellerData(myContact, own);
    if (!missing.isEmpty()) {
        _error = i18n("The own identity is incomplete for an XRechnung. Please add the "
                      "following in the Own Identity page of the Kraft settings: %1",
                      missing.join(QStringLiteral(", ")));
        Q_EMIT xRechnungTmpFile(QString());
        return;
    }

    const QStringList missingBuyer = missingBuyerData(contact);
    if (!missingBuyer.isEmpty()) {
        _error = i18n("The customer data is incomplete for an XRechnung. Please add the "
                      "following to the address book contact of the customer: %1",
                      missingBuyer.join(QStringLiteral(", ")));
        Q_EMIT xRechnungTmpFile(QString());
        return;
    }

    QScopedPointer<DocumentTemplate> templateEngine;

    const QString tmplFile = templateFile();
    if (tmplFile.isEmpty()) {
        qDebug() << "Empty template file -> exit!";
        _error = i18n("Could not find the XRechnung template file");
    } else {
        qDebug() << "Using this XRechnung Template:" << tmplFile;
    }
    templateEngine.reset(new GrantleeDocumentTemplate(tmplFile));

    QVariantHash xr;
    xr.insert("dueDate", _dueDate);
    xr.insert("buyerRef", _buyerRef);
    templateEngine->addExtraHash("xrechnung", xr);

    const QString expanded = templateEngine->expand(_uuid, myContact, contact);

    if (expanded.isEmpty()) {
        // Q_EMIT failure(i18n("The template expansion failed."));
        qDebug() << "Expansion failed, empty result" << templateEngine->error();
        _error = templateEngine->error();
    }

    QTemporaryFile tempFile("/tmp/xrech_XXXXXX");
    tempFile.setAutoRemove(false);

    if (!tempFile.open()) {
        _error = i18n("A temporary file could not be created");
    }
    QString fName;
    if (_error.isEmpty()) {
        fName = tempFile.fileName();
        qDebug() << "########## XRechnung written to" << fName;

        QTextStream outStream(&tempFile);
        outStream << expanded;
        tempFile.close();
#if 0
        if (_validateWithSchema && _schema.isValid()) {
            QFile file(fName);
            file.open(QIODevice::ReadOnly);
            QXmlSchemaValidator validator(_schema);
            if (validator.validate(&file, QUrl::fromLocalFile(file.fileName())))
                qDebug() << "instance document is valid";
            else
                qDebug() << "instance document is invalid";
        }
#endif
    }
    Q_EMIT xRechnungTmpFile(fName);
}

ExporterXRechnung::~ExporterXRechnung( )
{

}

/* END */

