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

void ExporterXRechnung::slotAddresseeFound(const QString& uid, const KContacts::Addressee& contact)
{
    Q_UNUSED(uid)
    KContacts::Addressee myContact; // leave empty for now
    // now the three pillars archDoc, myContact and mCustomerContact are defined.

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

