/***************************************************************************
            KTextTemplate based template for Kraft Documents
                             -------------------
    begin                : March 2020
    copyright            : (C) 2020 by Klaas Freitag
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

#include "documenttemplate.h"
#include "defaultprovider.h"
#include "epcqrcode.h"
#include "grantleetemplate.h"
#include "format.h"
#include "kraftsettings.h"
#include "version.h"
#include "documentman.h"
#include "kraftdoc.h"
#include "docposition.h"
#include "reportitemlist.h"

#include <QTemporaryFile>
#include <klocalizedstring.h>

#define TAG( THE_TAG )  QStringLiteral( THE_TAG )
#define DICT( THE_DICT )  QStringLiteral( THE_DICT )

// ==================================================================================

namespace Template {

QVariantHash contactToVariantHash(const KContacts::Addressee& contact )
{
    QVariantHash hash;

    QString n = contact.realName();
    if (n.isEmpty()) n = TAG("Not set!");
    hash.insert( TAG( "NAME" ),  n.toHtmlEscaped());

    if( contact.isEmpty() ) return hash;


    QString co = contact.organization();
    if( co.isEmpty() ) {
        co = contact.realName();
    }
    hash.insert( TAG( "ORGANISATION" ), co.toHtmlEscaped());
    const QUrl url = contact.url().url();
    hash.insert( TAG( "URL" ),   url.url().toHtmlEscaped());
    hash.insert( TAG( "EMAIL" ), contact.preferredEmail().toHtmlEscaped());
    hash.insert( TAG( "PHONE" ), contact.phoneNumber( KContacts::PhoneNumber::Work ).number().toHtmlEscaped());
    hash.insert( TAG( "FAX" ),   contact.phoneNumber( KContacts::PhoneNumber::Fax ).number().toHtmlEscaped());
    hash.insert( TAG( "MOBILE" ),  contact.phoneNumber( KContacts::PhoneNumber::Cell ).number().toHtmlEscaped());

    KContacts::Address address;
    address = contact.address( KContacts::Address::Pref );
    if( address.isEmpty() )
        address = contact.address(KContacts::Address::Work );
    if( address.isEmpty() )
        address = contact.address(KContacts::Address::Home );
    if( address.isEmpty() )
        address = contact.address(KContacts::Address::Postal );

    hash.insert( TAG( "POSTBOX" ), address.postOfficeBox().toHtmlEscaped());

    hash.insert( TAG( "EXTENDED" ), address.extended().toHtmlEscaped());
    hash.insert( TAG( "STREET" ), address.street().toHtmlEscaped());
    hash.insert( TAG( "LOCALITY" ), address.locality().toHtmlEscaped());
    hash.insert( TAG( "REGION" ), address.region().toHtmlEscaped());
    hash.insert( TAG( "POSTCODE" ), address.postalCode().toHtmlEscaped());
    hash.insert( TAG( "COUNTRY" ), address.country().toHtmlEscaped());
    hash.insert( TAG( "REGION" ), address.region().toHtmlEscaped());
    hash.insert( TAG("LABEL" ), address.label().toHtmlEscaped());
    return hash;
}

QVariantHash labelVariantHash()
{
    QVariantHash hash;

    hash.insert( TAG( "NO_SHORT"), i18nc("Sequence number printed on the document", "No.") );
    hash.insert( TAG( "ITEM"), i18nc("Document item printed on the document", "Item") );
    hash.insert( TAG( "QUANTITY_SHORT"), i18nc("Abbrev. of Quantity printed on the document", "Qty.") );
    hash.insert( TAG( "UNIT"), i18nc("Unit printed on the document", "Unit") );
    hash.insert( TAG( "PRICE"), i18nc("Price of an item printed on the document", "Price") );
    hash.insert( TAG( "SUM"), i18nc("Printed on the document", "Sum") );
    hash.insert( TAG( "NET"), i18nc("printed on the document", "Net") );
    hash.insert( TAG( "VAT"), i18nc("Printed on the document", "VAT") );
    hash.insert( TAG( "TYPE"), i18nc("Document type, printed on the document", "Type") );

    hash.insert( TAG( "PHONE"), i18nc("Printed on the document", "Phone"));
    hash.insert( TAG( "FAX"), i18nc("Printed on the document", "Fax"));
    hash.insert( TAG( "MOBILE"), i18nc("Printed on the document", "Mobile"));
    hash.insert( TAG( "EMAIL"), i18nc("Printed on the document", "Email"));
    hash.insert( TAG( "WEBSITE"), i18nc("Printed on the document", "Website"));

    hash.insert( TAG( "PAGE"), i18nc("Printed on the document", "Page"));
    hash.insert( TAG( "PREDECESSOR"), i18nc("Label of Predecessor document number", "Predecessor-Doc"));
    hash.insert( TAG( "PAGE_OF"), i18nc("the 'of' in page X of Y", "of"));
    hash.insert( TAG( "DOC_NO"), i18nc("Document number on document", "Document No."));
    hash.insert( TAG( "DATE"), i18nc("Date on document", "Date"));
    hash.insert( TAG( "PROJECT"), i18nc("Project label", "Project"));
    hash.insert( TAG( "CUST_ID"), i18nc("Customer ID on document", "Customer ID"));
    hash.insert( TAG( "CURRENCY_SIGN"), DefaultProvider::self()->currencySymbol());

    return hash;
}
}

namespace {



QVariantHash kraftVariantHash()
{
    QVariantHash hash;
    QString h = QString("Kraft %1 %2").arg(Kraft::Version::number()).
            arg(Kraft::Version::codeName());
    hash.insert(TAG("VERSION"), h);

    h = QString("DB-Scheme %1").arg(Kraft::Version::dbSchemaVersion());
    hash.insert(TAG("DB_SCHEME"), h);

    h = qgetenv("USER");
    if (h.isEmpty())
        h = qgetenv("USERNAME");
    hash.insert(TAG("SYS_USER"), h);

    h = qgetenv("HOSTNAME");
    if (h.isEmpty())
        h = qgetenv("HOST");
    if (!h.isEmpty())
        hash.insert(TAG("HOSTNAME"), h);

    return hash;
}

QString generateEPCQRCodeFile(KraftDoc *doc)
{
    QString tempFile;
    if (!doc) return tempFile;

    const QString bacName = KraftSettings::self()->bankAccountName();
    const QString bacIBAN = KraftSettings::self()->bankAccountIBAN();
    const QString bacBIC  = KraftSettings::self()->bankAccountBIC();
    EPCQRCode qrCode;
    const QString reason = i18nc("Credit Transfer reason string, 1=DocType, 2=DocIdent, 3=Date, ie. Invoice 2022-183 dated 2022-03-22",
                                 "%1 %2 dated %3",doc->docType(), doc->ident(), doc->dateStr());
    const QString svgText = qrCode.asSvg(doc->bruttoSum(), bacName, bacBIC, bacIBAN, reason);

    // -- save the EPC QR Code to a temp file
    if (svgText.isEmpty()) {
        qWarning() << "Failed to generate SVG text";
    } else {
        QTemporaryFile tFile(QString("%1/XXXXXX.svg").arg(QDir::tempPath()));
        tFile.setAutoRemove(false);
        if (tFile.open()) {
            tempFile = tFile.fileName();
            QTextStream stream(&tFile);
            stream << svgText;
            tFile.close();
        }
    }
    return tempFile;
}
}

// ==================================================================================

DocumentTemplate::DocumentTemplate( const QString& tmplFile )
    :_tmplFile(tmplFile)
{

}

// ==================================================================================

GrantleeDocumentTemplate::GrantleeDocumentTemplate(const QString& tmplFile)
    : DocumentTemplate(tmplFile)
{

}

const QString GrantleeDocumentTemplate::expand( const QString& uuid,
                                                const KContacts::Addressee &myContact,
                                                const KContacts::Addressee &customerContact)
{

    // that was needed before with ArchDocPosition, which used GRANTLEE_BEGIN_LOOKUP;
    // Grantlee::registerMetaType<ReportItemList>();
    // Grantlee::registerMetaType<ReportItem>();

    QFileInfo fi(_tmplFile);
    if (!fi.exists()) {
        _errorStr = i18n("Template to convert is not existing!");
    }
    if (!fi.isReadable()) {
        _errorStr = i18n("Can not read template file!");
    }

    KraftDoc *doc = DocumentMan::self()->openDocumentByUuid(uuid);

    if (doc == nullptr) {
        _errorStr = i18n("Could not open document %1", uuid);
    }

    QString rendered;
    if (_errorStr.isEmpty()) {

        GrantleeFileTemplate gtmpl(_tmplFile);

        gtmpl.addToObjMapping("doc", doc);

        const auto mtt = Template::contactToVariantHash(myContact);
        gtmpl.addToMappingHash(QStringLiteral("me"), mtt);

        const auto cct = Template::contactToVariantHash(customerContact);
        gtmpl.addToMappingHash(QStringLiteral("customer"), cct);

        const QVariantHash labelHash = Template::labelVariantHash();
        gtmpl.addToMappingHash(QStringLiteral("label"), labelHash);

        // -- save the EPC QR Code which is written into a temp file
        QVariantHash epcHash;

        auto qrcodefile = generateEPCQRCodeFile(doc);
        epcHash.insert("valid", false);
        epcHash.insert("show", false);
        if (qrcodefile.isEmpty()) {
            qDebug() << "No Giro Code file available.";
        } else {
            _tmpFiles.append(qrcodefile); // remember file to delete later.
            qDebug() << "Generated Giro Code file" << qrcodefile;

            epcHash.insert("svgfilename", QVariant(qrcodefile));
            epcHash["valid"] = true;
            epcHash["show"] = true;

            // there is a setting value of the maximum sum the EPC Code should
            // be printed on the document. The idea is that for very big sums,
            // the QR code should not be displayed.
            double maxEPCSum = KraftSettings::self()->displayEPCCodeMaxSum();

            if (doc->bruttoSum().toDouble() > maxEPCSum) {
                epcHash["show"] = false;
            }
            gtmpl.addToMappingHash(QStringLiteral("epcqrcode"), epcHash);
        }

        const QVariantHash kraftHash = kraftVariantHash();
        gtmpl.addToMappingHash(QStringLiteral("kraft"), kraftHash);

        bool ok;
        rendered = gtmpl.render(ok);
        if (!ok) {
            _errorStr = rendered;
            rendered.clear();
        }
    }
    delete doc;
    return rendered;
}
