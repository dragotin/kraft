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

#include "exportxrechnung.h"
#include "documentman.h"
#include "docposition.h"
#include "kraftdoc.h"
#include "kraftdb.h"
#include "doctype.h"
#include "format.h"
#include "addressprovider.h"
#include "documenttemplate.h"


namespace {

QString xRechnungTemplate()
{
    DocType dt("Rechnung"); // FIXME hardcoded

    const QString re = dt.xRechnungTemplate();

    return re;
}

}

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
    return xRechnungTemplate();
}

bool ExporterXRechnung::exportDocument(const QString& uuid)
{
    // FIXME: load the doc and set DueDate und BuyerRef
    KraftDoc *doc = DocumentMan::self()->openDocumentByUuid(uuid);
    // _archDoc.setDueDate(_dueDate);
    // _archDoc.setBuyerRef(_buyerRef);

    if (xRechnungTemplate().isEmpty()) {
        qDebug () << "tmplFile is empty, exit reportgenerator!";
        return false;
    }

    const QString clientUid = doc->addressUid();
    delete doc;
    _customerContact = KContacts::Addressee();

    if( ! clientUid.isEmpty() ) {
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
    KContacts::Addressee myContact; // leave empty for now
    // now the three pillars archDoc, myContact and mCustomerContact are defined.

    QScopedPointer<DocumentTemplate> templateEngine;

    const QString tmplFile = xRechnungTemplate();
    if (tmplFile.isEmpty()) {
        qDebug() << "Empty template file -> exit!";
    } else {
        qDebug() << "Using this XRechnung Template:" << tmplFile;
    }
    templateEngine.reset(new GrantleeDocumentTemplate(tmplFile));

    const QString uuid; // FIXME
    // expand the template...
    const QString expanded = templateEngine->expand(uuid, myContact, contact);

    if (expanded.isEmpty()) {
        // Q_EMIT failure(i18n("The template expansion failed."));
        qDebug() << "Expansion failed, empty result";
        return;
    }

    QTemporaryFile tempFile("/tmp/xrech_XXXXXX");
    tempFile.setAutoRemove(false);


    if (tempFile.open()) {
        const QString fName = tempFile.fileName();
        qDebug() << "########## XRechnung written to" << fName;

        QTextStream outStream(&tempFile);
        outStream << expanded;
        tempFile.close();

        Q_EMIT xRechnungTmpFile(fName);
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
}

ExporterXRechnung::~ExporterXRechnung( )
{

}

/* END */

