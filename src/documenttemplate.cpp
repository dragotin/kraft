/***************************************************************************
            Template for Kraft Documents - Grantlee and ctemplate
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
#include "texttemplate.h"
#include "grantleetemplate.h"
#include "format.h"
#include "kraftsettings.h"
#include "version.h"
#include "documentman.h"
#include "kraftdoc.h"
#include "docposition.h"

#include <klocalizedstring.h>

#define TAG( THE_TAG )  QStringLiteral( THE_TAG )
#define DICT( THE_DICT )  QStringLiteral( THE_DICT )

// ==================================================================================

namespace {

QString escapeTrml2pdfXML( const QString& str )
{
    return( str.toHtmlEscaped() );
}

QString rmlString( const QString& str, const QString& paraStyle = QString() )
{
    QString rml;

    QString style( paraStyle );
    if ( style.isEmpty() ) style = QStringLiteral("text");

    // QStringList li = QStringList::split( "\n", escapeTrml2pdfXML( str ) );
    QStringList li = escapeTrml2pdfXML( str ).split( "\n" );
    rml = QString( "<para style=\"%1\">" ).arg( style );
    rml += li.join( QString( "</para><para style=\"%1\">" ).arg( style ) ) + "</para>";
    // qDebug () << "Returning " << rml;
    return rml;
}

QVariantHash contactToVariantHash(const KContacts::Addressee& contact )
{
    QVariantHash hash;

    QString n = contact.realName();
    if (n.isEmpty()) n = QStringLiteral("Not set!");
    hash.insert( QStringLiteral( "NAME" ),  escapeTrml2pdfXML(n) );

    if( contact.isEmpty() ) return hash;


    QString co = contact.organization();
    if( co.isEmpty() ) {
        co = contact.realName();
    }
    hash.insert( QStringLiteral( "ORGANISATION" ), escapeTrml2pdfXML( co ) );
    const QUrl url = contact.url().url();
    hash.insert( QStringLiteral( "URL" ),   escapeTrml2pdfXML( url.url() ) );
    hash.insert( QStringLiteral( "EMAIL" ), escapeTrml2pdfXML( contact.preferredEmail() ) );
    hash.insert( QStringLiteral( "PHONE" ), escapeTrml2pdfXML( contact.phoneNumber( KContacts::PhoneNumber::Work ).number() ) );
    hash.insert( QStringLiteral( "FAX" ),   escapeTrml2pdfXML( contact.phoneNumber( KContacts::PhoneNumber::Fax ).number() ) );
    hash.insert( QStringLiteral( "CELL" ),  escapeTrml2pdfXML( contact.phoneNumber( KContacts::PhoneNumber::Cell ).number() ) );

    KContacts::Address address;
    address = contact.address( KContacts::Address::Pref );
    if( address.isEmpty() )
        address = contact.address(KContacts::Address::Work );
    if( address.isEmpty() )
        address = contact.address(KContacts::Address::Home );
    if( address.isEmpty() )
        address = contact.address(KContacts::Address::Postal );

    hash.insert( QStringLiteral( "POSTBOX" ),
                 escapeTrml2pdfXML( address.postOfficeBox() ) );

    hash.insert( QStringLiteral( "EXTENDED" ),
                 escapeTrml2pdfXML( address.extended() ) );
    hash.insert( QStringLiteral( "STREET" ),
                 escapeTrml2pdfXML( address.street() ) );
    hash.insert( QStringLiteral( "LOCALITY" ),
                 escapeTrml2pdfXML( address.locality() ) );
    hash.insert( QStringLiteral( "REGION" ),
                 escapeTrml2pdfXML( address.region() ) );
    hash.insert( QStringLiteral( "POSTCODE" ),
                 escapeTrml2pdfXML( address.postalCode() ) );
    hash.insert( QStringLiteral( "COUNTRY" ),
                 escapeTrml2pdfXML( address.country() ) );
    hash.insert( QStringLiteral( "REGION" ),
                 escapeTrml2pdfXML( address.region() ) );
    hash.insert( QStringLiteral("LABEL" ),
                 escapeTrml2pdfXML( address.label() ) );
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
    hash.insert( TAG( "FAX"), i18nc("Printed on the document", "FAX"));
    hash.insert( TAG( "MOBILE"), i18nc("Printed on the document", "Mobile"));
    hash.insert( TAG( "EMAIL"), i18nc("Printed on the document", "Email"));
    hash.insert( TAG( "WEBSITE"), i18nc("Printed on the document", "Website"));

    hash.insert( TAG( "PAGE"), i18nc("Printed on the document", "Page"));
    hash.insert( TAG( "PREDECESSOR"), i18nc("Label of Predecessor document number", "Predecessor-Doc"));
    hash.insert( TAG( "PAGE_OF"), i18nc("the 'of' in page X of Y", "of"));
    hash.insert( TAG( "DOC_NO"), i18nc("Document number on document", "Document No."));
    hash.insert( TAG( "DATE"), i18nc("Date on document", "Date"));
    hash.insert( TAG( "PROJECT"), i18nc("Project label", "Project"));
    hash.insert( TAG( "CUST_ID"), i18nc("Customer ID on document", "Customer Id"));
    hash.insert( TAG( "CURRENCY_SIGN"), DefaultProvider::self()->currencySymbol());

    return hash;
}

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

void variantHashToTemplate( TextTemplate& tmpl, const QString& prefix, const QVariantHash& hash)
{
    QVariantHash::const_iterator i;
    for (i = hash.constBegin(); i != hash.constEnd(); ++i) {
        QString key = i.key();
        if (!prefix.isEmpty()) {
            key = QString("%1_%2").arg(prefix).arg(i.key());
        }
        tmpl.setValue(key, i.value().toString());
    }
}

void contactToTemplate( TextTemplate& tmpl, const QString& prefix, const KContacts::Addressee& contact )
{
    const QVariantHash hash = contactToVariantHash(contact);

    variantHashToTemplate(tmpl, prefix, hash);
}

void addLabelsToTemplate(TextTemplate& tmpl)
{
    const QVariantHash hash = labelVariantHash();
    variantHashToTemplate(tmpl, QStringLiteral("LAB"), hash);
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
CTemplateDocumentTemplate::CTemplateDocumentTemplate(const QString& tmplFile)
    :DocumentTemplate(tmplFile)
{

}

const QString CTemplateDocumentTemplate::expand(const QString& uuid, const KContacts::Addressee& myContact,
                                                const KContacts::Addressee& customerContact)
{
    if (uuid.isEmpty()) {
        return QString();
    }
    // create a text template
    TextTemplate tmpl;
    tmpl.setTemplateFileName(_tmplFile);

    KraftDoc *doc = DocumentMan::self()->openDocumentByUuid(uuid);

    /* replace the placeholders */
    /* A placeholder has the format <!-- %VALUE --> */

    const DocPositionList posList = doc->positions();

    bool individualTax = posList.hasIndividualTaxes();
    /* Check for the tax settings: If the taxType is not the same for all items,
 * we have individual Tax setting and show the tax marker etc.
 */
#if 0
    DocPositionBase::TaxType ttype = posList.listTaxation();
    for ( DocPositionBase *p : posList) {
        DocPositionBase pos = *p;
        if( ttype == DocPositionBase::TaxInvalid  ) {
            ttype = pos.taxType();
        } else {
            if( ttype != pos.taxType() ) { // different from previous one?
                individualTax = true;
                break;
            }
        }
    }
#endif
    /* now loop over the items to fill the template structures */
    int specialPosCnt{0};
    int taxFreeCnt{0}, reducedTaxCnt{0}, fullTaxCnt{0};
    QString h;

    for (DocPositionBase *p : posList) {
        DocPosition *posPtr = static_cast<DocPosition*>(p);
        DocPosition pos = *posPtr;
        tmpl.createDictionary( "POSITIONS" );
        tmpl.setValue( DICT("POSITIONS"), TAG( "POS_NUMBER" ), QString::number(pos.positionNumber()));
        tmpl.setValue( DICT("POSITIONS"), TAG("POS_TEXT"),
                       rmlString(pos.text(), QString( "text" )));

        // format the amount value of the item, do not show the precision if there is no fraction
        double amount = pos.amount();
        h = Format::localeDoubleToString(amount, *DefaultProvider::self()->locale());

        tmpl.setValue( DICT("POSITIONS"), TAG("POS_AMOUNT"), h );
        h = pos.unit().einheitSingular();
        if (pos.amount() > 1.0) h = pos.unit().einheitPlural();
        tmpl.setValue( DICT("POSITIONS"), TAG("POS_UNIT"), escapeTrml2pdfXML(h) );
        tmpl.setValue( DICT("POSITIONS"), TAG("POS_UNITPRICE"), pos.unitPrice().toLocaleString() );
        tmpl.setValue( DICT("POSITIONS"), TAG("POS_TOTAL"), pos.overallPrice().toLocaleString() );
        tmpl.setValue( DICT("POSITIONS"), TAG("POS_KIND"), QString() );

        QString taxType;

        if( individualTax ) {
            if( pos.taxType() == 1 ) {
                taxFreeCnt++;
                taxType = "TAX_FREE";
            } else if( pos.taxType() == 2 ) {
                reducedTaxCnt++;
                taxType = "REDUCED_TAX";
            } else {
                // ATTENTION: Default for all non known tax types is full tax.
                fullTaxCnt++;
                taxType = "FULL_TAX";
            }

            tmpl.createSubDictionary( "POSITIONS", taxType );
        }

        /* item kind: Normal, alternative or demand item. For normal items, the kind is empty.
   */
        if (pos.type() == DocPositionBase::Demand || pos.type() == DocPositionBase::Alternative) {
            specialPosCnt++;
        }
    }
    if ( specialPosCnt ) {
        tmpl.createDictionary( "SPECIAL_POS" );
        tmpl.setValue( DICT("SPECIAL_POS"), TAG("COUNT"), QString::number( specialPosCnt ) );
        tmpl.setValue( DICT("SPECIAL_POS"), TAG("LAB_SPECIAL_ITEMS"),
                       i18n("Please note: This offer contains %1 alternative or demand positions, printed in italic font. These do not add to the overall sum.",
                            QString::number( specialPosCnt ) ) );
    }

    /*
 * Just show the tax index if we have multiple tax settings
 */
    if( individualTax ) {
        tmpl.createDictionary( "TAX_FREE_ITEMS" );
        tmpl.setValue( DICT("TAX_FREE_ITEMS"), TAG("COUNT"), QString::number( taxFreeCnt ));
        tmpl.setValue( DICT("TAX_FREE_ITEMS"), TAG( "LAB_TAX_FREE_ITEMS"),
                       i18n("tax free items (%1 pcs.)", QString::number( taxFreeCnt )) );

        tmpl.createDictionary( "REDUCED_TAX_ITEMS" );
        tmpl.setValue( DICT("REDUCED_TAX_ITEMS"), TAG("COUNT"), QString::number( reducedTaxCnt ));
        tmpl.setValue( DICT("REDUCED_TAX_ITEMS"), TAG("TAX"), doc->reducedTaxPercentStr());
        tmpl.setValue( DICT("REDUCED_TAX_ITEMS"), TAG("LAB_TAX_REDUCED_ITEMS"),
                       i18n("items with reduced tax of %1% (%2 pcs.)",
                            doc->reducedTaxPercentStr(),
                            QString::number(reducedTaxCnt)) );


        tmpl.createDictionary( "FULL_TAX_ITEMS" );
        tmpl.setValue( DICT("FULL_TAX_ITEMS"), TAG("COUNT"), QString::number( fullTaxCnt ));
        tmpl.setValue( DICT("FULL_TAX_ITEMS"), TAG("TAX"), doc->fullTaxPercentStr() );
        tmpl.setValue( DICT("FULL_TAX_ITEMS"), TAG("LAB_TAX_FULL_ITEMS"),
                       i18n("No label: items with full tax of %1% (%2 pcs.)",
                            doc->fullTaxPercentStr(), QString::number( fullTaxCnt )));
    }

    /* now replace stuff in the whole document */
    tmpl.setValue( TAG( "DATE" ), Format::toDateString(doc->date(), KraftSettings::self()->dateFormat()));
    tmpl.setValue( TAG( "DOCTYPE" ), escapeTrml2pdfXML( doc->docType() ) );
    tmpl.setValue( TAG( "ADDRESS" ), escapeTrml2pdfXML( doc->address() ) );

    contactToTemplate( tmpl, "CLIENT", customerContact );
    contactToTemplate( tmpl, "MY", myContact );

    tmpl.setValue( TAG( "DOCID" ),   escapeTrml2pdfXML( doc->ident() ) );
    tmpl.setValue( TAG( "PROJECTLABEL" ),   escapeTrml2pdfXML( doc->projectLabel() ) );
    tmpl.setValue( TAG( "SALUT" ),   escapeTrml2pdfXML( doc->salut() ) );
    tmpl.setValue( TAG( "GOODBYE" ), escapeTrml2pdfXML( doc->goodbye() ) );
    tmpl.setValue( TAG( "PRETEXT" ),   rmlString( doc->preText() ) );
    tmpl.setValue( TAG( "POSTTEXT" ),  rmlString( doc->postText() ) );
    tmpl.setValue( TAG( "BRUTTOSUM" ), doc->bruttoSumStr());
    tmpl.setValue( TAG( "NETTOSUM" ),  doc->nettoSumStr());

    // qDebug () << "Tax in archive document: " << h;
    if ( doc->reducedTaxSum().toLong() != 0 ) {
        tmpl.createDictionary( DICT( "SECTION_REDUCED_TAX" ) );
        tmpl.setValue( DICT("SECTION_REDUCED_TAX"), TAG( "REDUCED_TAX_SUM" ),
                       doc->reducedTaxSumStr());
        tmpl.setValue( DICT("SECTION_REDUCED_TAX"), TAG("REDUCED_TAX"), doc->reducedTaxPercentStr());
        tmpl.setValue( DICT("SECTION_REDUCED_TAX"), TAG("REDUCED_TAX_LABEL"), i18n("reduced VAT"));
    }
    if ( doc->fullTaxSum().toLong() != 0 ) {
        tmpl.createDictionary( DICT( "SECTION_FULL_TAX" ) );
        tmpl.setValue( DICT("SECTION_FULL_TAX"), TAG( "FULL_TAX_SUM" ),
                       doc->fullTaxSumStr());
        tmpl.setValue( DICT("SECTION_FULL_TAX"), TAG( "FULL_TAX" ), doc->fullTaxPercentStr() );
        tmpl.setValue( DICT("SECTION_FULL_TAX"), TAG( "FULL_TAX_LABEL" ), i18n( "VAT" ) );
    }

    // legacy values, not used in the official doc
    tmpl.setValue( TAG( "VAT" ), doc->fullTaxPercentStr());
    tmpl.setValue( TAG( "VATSUM" ), doc->fullTaxSumStr());

    addLabelsToTemplate(tmpl);

#if 0
    /* this is still disabled as reportlab can not read SVG files
     * When it can or the EPC QR Code can be generated as PNG, this needs to be added
     * to the template:
     *
     *   {{#EPC_QR_CODE}}
     *    <hr/>
     *    <illustration width="120" height="120">
     *      <image x="5" y="5" width="110" height="110" file="{{SVG_FILE_NAME}}" />
     *    </illustration>
     *    {{/EPC_QR_CODE}}
     */

    QString qrcodefile;
    if (archDoc->isInvoice()) {
        qrcodefile = generateEPCQRCodeFile(archDoc);
        _tmpFiles.append(qrcodefile);
        tmpl.createDictionary( DICT( "EPC_QR_CODE" ) );
        tmpl.setValue( DICT("EPC_QR_CODE"), TAG( "SVG_FILE_NAME" ), qrcodefile);
    }
#endif
    // finalize the template
    const QString output = tmpl.expand();

    return output;
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
#if 0
    // that was needed before with ArchDocPosition, which used GRANTLEE_BEGIN_LOOKUP
    Grantlee::registerMetaType<KraftObj>();
    Grantlee::registerMetaType<DocPositionBase>();
    Grantlee::registerMetaType<DocPosition>();
    Grantlee::registerMetaType<DocPositionList>();
#endif

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

        const auto mtt = contactToVariantHash(myContact);
        gtmpl.addToMappingHash(QStringLiteral("me"), mtt);

        const auto cct = contactToVariantHash(customerContact);
        gtmpl.addToMappingHash(QStringLiteral("customer"), cct);

        const QVariantHash labelHash = labelVariantHash();
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
    return rendered;
}
