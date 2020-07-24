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
#include "texttemplate.h"
#include "grantleetemplate.h"
#include "format.h"
#include "kraftsettings.h"

#include <klocalizedstring.h>

#define TAG( THE_TAG )  QStringLiteral( THE_TAG )
#define DICT( THE_DICT )  QStringLiteral( THE_DICT )

// ==================================================================================

namespace  {

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

    hash.insert( TAG( "PHONE"), i18nc("Printed on the document", "Phone"));
    hash.insert( TAG( "FAX"), i18nc("Printed on the document", "FAX"));
    hash.insert( TAG( "MOBILE"), i18nc("Printed on the document", "Mobile"));
    hash.insert( TAG( "EMAIL"), i18nc("Printed on the document", "Email"));
    hash.insert( TAG( "WEBSITE"), i18nc("Printed on the document", "Website"));

    hash.insert( TAG( "PAGE"), i18nc("Printed on the document", "Page"));
    hash.insert( TAG( "PAGE_OF"), i18nc("the 'of' in page X of Y", "of"));
    hash.insert( TAG( "DOC_NO"), i18nc("Document number on document", "Document No."));
    hash.insert( TAG( "DATE"), i18nc("Date on document", "Date"));
    hash.insert( TAG( "PROJECT"), i18nc("Project label", "Project"));
    hash.insert( TAG( "CUST_ID"), i18nc("Customer ID on document", "Customer Id"));
    hash.insert( TAG( "CURRENCY_SIGN"), DefaultProvider::self()->currencySymbol());


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

const QString CTemplateDocumentTemplate::expand(ArchDoc *archDoc, const KContacts::Addressee& myContact,
                                                const KContacts::Addressee& customerContact)
{
    if (archDoc == nullptr) {
        return QString();
    }
    // create a text template
    TextTemplate tmpl;
    tmpl.setTemplateFileName(_tmplFile);

    /* replace the placeholders */
    /* A placeholder has the format <!-- %VALUE --> */

    const ArchDocPositionList posList = archDoc->positions();
    QString h;

    ArchDocPositionList::const_iterator it;
    int specialPosCnt = 0;
    int taxFreeCnt    = 0;
    int reducedTaxCnt = 0;
    int fullTaxCnt    = 0;

    bool individualTax = false;
    /* Check for the tax settings: If the taxType is not the same for all items,
 * we have individual Tax setting and show the tax marker etc.
 */
    DocPositionBase::TaxType ttype = DocPositionBase::TaxInvalid;
    for ( it = posList.begin(); it != posList.end(); ++it ) {
        ArchDocPosition pos (*it);
        if( ttype == DocPositionBase::TaxInvalid  ) {
            ttype = pos.taxType();
        } else {
            if( ttype != pos.taxType() ) { // different from previous one?
                individualTax = true;
                break;
            }
        }
    }

    /* now loop over the items to fill the template structures */
    for ( it = posList.begin(); it != posList.end(); ++it ) {
        ArchDocPosition pos (*it);
        tmpl.createDictionary( "POSITIONS" );
        tmpl.setValue( DICT("POSITIONS"), TAG( "POS_NUMBER" )
                       , pos.posNumber() );
        tmpl.setValue( DICT("POSITIONS"), TAG("POS_TEXT"),
                       rmlString( pos.text(), QString( "%1text" ).arg( pos.kind().toLower() ) ) );

        // format the amount value of the item, do not show the precision if there is no fraction
        double amount = pos.amount();
        h = Format::localeDoubleToString(amount, *DefaultProvider::self()->locale());

        tmpl.setValue( DICT("POSITIONS"), TAG("POS_AMOUNT"), h );
        tmpl.setValue( DICT("POSITIONS"), TAG("POS_UNIT"), escapeTrml2pdfXML( pos.unit() ) );
        tmpl.setValue( DICT("POSITIONS"), TAG("POS_UNITPRICE"), pos.unitPrice().toString() );
        tmpl.setValue( DICT("POSITIONS"), TAG("POS_TOTAL"), pos.nettoPrice().toString() );
        tmpl.setValue( DICT("POSITIONS"), TAG("POS_KIND"), pos.kind().toLower() );

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
        if ( !pos.kind().isEmpty() ) {
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
        tmpl.setValue( DICT("REDUCED_TAX_ITEMS"), TAG("TAX"), DefaultProvider::self()->locale()->toString( archDoc->reducedTax()) );
        tmpl.setValue( DICT("REDUCED_TAX_ITEMS"), TAG("LAB_TAX_REDUCED_ITEMS"),
                       i18n("items with reduced tax of %1% (%2 pcs.)",
                            DefaultProvider::self()->locale()->toString( archDoc->reducedTax()),
                            QString::number( reducedTaxCnt )) );


        tmpl.createDictionary( "FULL_TAX_ITEMS" );
        tmpl.setValue( DICT("FULL_TAX_ITEMS"), TAG("COUNT"), QString::number( fullTaxCnt ));
        tmpl.setValue( DICT("FULL_TAX_ITEMS"), TAG("TAX"), DefaultProvider::self()->locale()->toString( archDoc->tax()) );
        tmpl.setValue( DICT("FULL_TAX_ITEMS"), TAG("LAB_TAX_FULL_ITEMS"),
                       i18n("No label: items with full tax of %1% (%2 pcs.)",
                            DefaultProvider::self()->locale()->toString( archDoc->tax()),
                            QString::number( fullTaxCnt )) );
    }

    /* now replace stuff in the whole document */
    tmpl.setValue( TAG( "DATE" ), Format::toDateString(archDoc->date(), KraftSettings::self()->dateFormat()));
    tmpl.setValue( TAG( "DOCTYPE" ), escapeTrml2pdfXML( archDoc->docType() ) );
    tmpl.setValue( TAG( "ADDRESS" ), escapeTrml2pdfXML( archDoc->address() ) );

    contactToTemplate( tmpl, "CLIENT", customerContact );
    contactToTemplate( tmpl, "MY", myContact );

    tmpl.setValue( TAG( "DOCID" ),   escapeTrml2pdfXML( archDoc->ident() ) );
    tmpl.setValue( TAG( "PROJECTLABEL" ),   escapeTrml2pdfXML( archDoc->projectLabel() ) );
    tmpl.setValue( TAG( "SALUT" ),   escapeTrml2pdfXML( archDoc->salut() ) );
    tmpl.setValue( TAG( "GOODBYE" ), escapeTrml2pdfXML( archDoc->goodbye() ) );
    tmpl.setValue( TAG( "PRETEXT" ),   rmlString( archDoc->preText() ) );
    tmpl.setValue( TAG( "POSTTEXT" ),  rmlString( archDoc->postText() ) );
    tmpl.setValue( TAG( "BRUTTOSUM" ), archDoc->bruttoSum().toString() );
    tmpl.setValue( TAG( "NETTOSUM" ),  archDoc->nettoSum().toString() );

    h = DefaultProvider::self()->locale()->toString( archDoc->tax() );
    // qDebug () << "Tax in archive document: " << h;
    if ( archDoc->reducedTaxSum().toLong() > 0 ) {
        tmpl.createDictionary( DICT( "SECTION_REDUCED_TAX" ) );
        tmpl.setValue( DICT("SECTION_REDUCED_TAX"), TAG( "REDUCED_TAX_SUM" ),
                       archDoc->reducedTaxSum().toString() );
        h = DefaultProvider::self()->locale()->toString( archDoc->reducedTax() );
        tmpl.setValue( DICT("SECTION_REDUCED_TAX"), TAG( "REDUCED_TAX" ), h );
        tmpl.setValue( DICT("SECTION_REDUCED_TAX"), TAG( "REDUCED_TAX_LABEL" ), i18n( "reduced VAT" ) );
    }
    if ( archDoc->fullTaxSum().toLong() > 0 ) {
        tmpl.createDictionary( DICT( "SECTION_FULL_TAX" ) );
        tmpl.setValue( DICT("SECTION_FULL_TAX"), TAG( "FULL_TAX_SUM" ),
                       archDoc->fullTaxSum().toString() );
        h = DefaultProvider::self()->locale()->toString( archDoc->tax() );
        tmpl.setValue( DICT("SECTION_FULL_TAX"), TAG( "FULL_TAX" ), h );
        tmpl.setValue( DICT("SECTION_FULL_TAX"), TAG( "FULL_TAX_LABEL" ), i18n( "VAT" ) );
    }

    h = DefaultProvider::self()->locale()->toString( archDoc->tax() );
    tmpl.setValue( TAG( "VAT" ), h );

    tmpl.setValue( TAG( "VATSUM" ), archDoc->taxSum().toString() );

    addLabelsToTemplate(tmpl);

    // finalize the template
    const QString output = tmpl.expand();
    return output;
}

// ==================================================================================

GrantleeDocumentTemplate::GrantleeDocumentTemplate(const QString& tmplFile)
    : DocumentTemplate(tmplFile)
{

}

const QString GrantleeDocumentTemplate::expand( ArchDoc *archDoc,
                                               const KContacts::Addressee &myContact,
                                               const KContacts::Addressee &customerContact)
{
    Grantlee::registerMetaType<ArchDocPosition>();
    Grantlee::registerMetaType<ArchDocPositionList>();

    QFileInfo fi(_tmplFile);
    if (!fi.exists()) {
        _errorStr = i18n("Template to convert is not existing!");
    }
    if (!fi.isReadable()) {
        _errorStr = i18n("Can not read template file!");
    }

    QString rendered;
    if (_errorStr.isEmpty()) {

        GrantleeFileTemplate gtmpl(_tmplFile);
        gtmpl.addToObjMapping("doc", archDoc);

        gtmpl.addToMapping(QStringLiteral("me"), contactToVariantHash(myContact));
        gtmpl.addToMapping(QStringLiteral("customer"), contactToVariantHash(customerContact));
        const QVariantHash labelHash = labelVariantHash();
        gtmpl.addToMapping(QStringLiteral("label"), labelHash);
        bool ok;
        rendered = gtmpl.render(ok);
        if (!ok) {
            _errorStr = rendered;
            rendered.clear();
        }
    }
    return rendered;
}
