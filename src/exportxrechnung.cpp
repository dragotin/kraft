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
#include <QStandardPaths>
#include <QDebug>
#include <QSqlQuery>
#include <QDir>

#include "exportxrechnung.h"
#include "archdoc.h"
#include "documentman.h"
#include "docposition.h"
#include "kraftdoc.h"
#include "kraftdb.h"
#include "unitmanager.h"
#include "dbids.h"
#include "kraftsettings.h"
#include "doctype.h"
#include "defaultprovider.h"
#include "format.h"
#include "addressprovider.h"
#include "grantleetemplate.h"
#include "documenttemplate.h"


namespace {

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

bool ExporterXRechnung::exportDocument(const ArchDocDigest& digest)
{
    _archDoc.loadFromDb(digest.archDocId());
    _archDoc.setDueDate(_dueDate);
    _archDoc.setBuyerRef(_buyerRef);

    const QString tmplFile = DefaultProvider::self()->locateFile("reports/xrechnung.gtmpl");

    if ( tmplFile.isEmpty() ) {
        qDebug () << "tmplFile is empty, exit reportgenerator!";
        return false;
    } else {
        qDebug () << "Using this template: " << tmplFile;
    }

    lookupCustomerAddress();
    return true;
}

void ExporterXRechnung::lookupCustomerAddress()
{
    const QString clientUid = _archDoc.clientUid();
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
            return;
        }
    }
    QTimer::singleShot(0, this, &ExporterXRechnung::slotSkipLookup);
}

void ExporterXRechnung::slotSkipLookup()
{
    slotAddresseeFound(QString(), _customerContact);
}

void ExporterXRechnung::slotAddresseeFound(const QString& uid, const KContacts::Addressee& contact)
{
    KContacts::Addressee myContact; // leave empty for now
    // now the three pillars archDoc, myContact and mCustomerContact are defined.

    QString output;
    QScopedPointer<DocumentTemplate> templateEngine;

    const QString tmplFile = DefaultProvider::self()->locateFile("reports/xrechnung.gtmpl");
    templateEngine.reset(new GrantleeDocumentTemplate(tmplFile));

    // expand the template...
    const QString expanded = templateEngine->expand(&_archDoc, myContact, contact);

    if (expanded.isEmpty()) {
        // emit failure(i18n("The template expansion failed."));
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

        emit xRechnungTmpFile(fName);
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

