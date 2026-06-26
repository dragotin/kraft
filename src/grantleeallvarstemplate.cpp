/***************************************************************************
        Description of template variabbles for Kraft Documents
                             -------------------
    begin                : June 2026
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

#include <QObject>
#include <QMetaObject>
#include <QMetaProperty>
#include <QStringLiteral>

#include <klocalizedstring.h>
#include "kraftdoc.h"

#include "grantleeallvarstemplate.h"
#include "kraftdoc.h"
#include "documentman.h"
#include "grantleetemplate.h"

using namespace Qt::StringLiterals;

namespace {

// The descriptions are keyed by namespace prefix ("doc", "contact", ...) and then
// by variable name. Kept as a function-local static so the i18nc() calls run at
// runtime (after the translation catalog is loaded), not during static init.
QString getDesc(const QString& prefix, const QString& name)
{
    // in i18nc the first argument is a hint for the translator
    static const QMap<QString, QMap<QString, QString> > descriptions = {
        { u"doc"_s, {
            { u"address"_s, i18nc("Document Template var description",
                "Formatted address string of the customer") },
            { u"bruttoSumNum"_s, i18nc("Document Template var description",
                "Brutto sum as numerical value, not localized") },
            { u"bruttoSumStr"_s, i18nc("Document Template var description",
                "Brutto sum localized as user string") },
            { u"buyerReference"_s, i18nc("Document Template var description",
                "Buyer reference string, mainly for XRechnung") },
            { u"clientUid"_s, i18nc("Document Template var description",
                "UID of the client/customer") },
            { u"dateStr"_s, i18nc("Document Template var description",
                "Date as localized string according to user settings") },
            { u"dateStrISO"_s, i18nc("Document Template var description",
                "date string ISO formatted, for XRechnung") },
            { u"docIDStr"_s, i18nc("Document Template var description",
                "Document ID as string") },
            { u"docIdentifier"_s, i18nc("Document Template var description",
                "Human readable document identifier, e.g. 'Invoice 2026-001' or a draft marker") },
            { u"docType"_s, i18nc("Document Template var description",
                "Localized document type, e.g. Invoice, Offer or Delivery Receipt") },
            { u"dueDateStrISO"_s, i18nc("Document Template var description",
                "Payment due date, ISO formatted, for XRechnung") },
            { u"fullTaxPercentNum"_s, i18nc("Document Template var description",
                "Full tax rate in percent as numerical value, not localized") },
            { u"fullTaxPercentStr"_s, i18nc("Document Template var description",
                "Full tax rate in percent localized as user string") },
            { u"fullTaxSumNum"_s, i18nc("Document Template var description",
                "Tax amount of the full taxed items as numerical value, not localized") },
            { u"fullTaxSumStr"_s, i18nc("Document Template var description",
                "Tax amount of the full taxed items localized as user string") },
            { u"fullTaxesDocument"_s, i18nc("Document Template var description",
                "True if all items of the document are taxed with the full tax rate") },
            { u"goodbye"_s, i18nc("Document Template var description",
                "Closing greeting line of the document, e.g. 'Kind regards'") },
            { u"hasIndividualTaxation"_s, i18nc("Document Template var description",
                "True if the document mixes different tax rates across its items") },
            { u"ident"_s, i18nc("Document Template var description",
                "Document identification number, 'draft' for unsaved documents") },
            { u"individualTaxesDocument"_s, i18nc("Document Template var description",
                "True if the document mixes different tax rates across its items") },
            { u"isDraftState"_s, i18nc("Document Template var description",
                "True if the document is still in draft state") },
            { u"isInvoice"_s, i18nc("Document Template var description",
                "True if the document is an invoice") },
            { u"items"_s, i18nc("Document Template var description",
                "List of the document positions (line items)") },
            { u"nettoSumNum"_s, i18nc("Document Template var description",
                "Netto sum as numerical value, not localized") },
            { u"nettoSumStr"_s, i18nc("Document Template var description",
                "Netto sum localized as user string") },
            { u"noTaxesDocument"_s, i18nc("Document Template var description",
                "True if none of the document items are taxed") },
            { u"owner"_s, i18nc("Document Template var description",
                "Owner of the document") },
            { u"postText"_s, i18nc("Document Template var description",
                "Text printed after the document positions, as plain text") },
            { u"postTextHtml"_s, i18nc("Document Template var description",
                "Text printed after the document positions, as HTML") },
            { u"preText"_s, i18nc("Document Template var description",
                "Text printed before the document positions, as plain text") },
            { u"preTextHtml"_s, i18nc("Document Template var description",
                "Text printed before the document positions, as HTML") },
            { u"predecessor"_s, i18nc("Document Template var description",
                "Identifier of the predecessor document this one was created from") },
            { u"projectLabel"_s, i18nc("Document Template var description",
                "Label of the project the document belongs to") },
            { u"reducedTaxPercentNum"_s, i18nc("Document Template var description",
                "Reduced tax rate in percent as numerical value, not localized") },
            { u"reducedTaxPercentStr"_s, i18nc("Document Template var description",
                "Reduced tax rate in percent localized as user string") },
            { u"reducedTaxSumNum"_s, i18nc("Document Template var description",
                "Tax amount of the reduced taxed items as numerical value, not localized") },
            { u"reducedTaxSumStr"_s, i18nc("Document Template var description",
                "Tax amount of the reduced taxed items localized as user string") },
            { u"reducedTaxesDocument"_s, i18nc("Document Template var description",
                "True if all items of the document are taxed with the reduced tax rate") },
            { u"salut"_s, i18nc("Document Template var description",
                "Salutation line of the document, e.g. 'Dear Mr. Smith'") },
            { u"state"_s, i18nc("Document Template var description",
                "Document state as string, e.g. Draft, Final, Retracted or Invalid") },
            { u"taxMarkerFull"_s, i18nc("Document Template var description",
                "Marker value identifying items taxed with the full tax rate") },
            { u"taxMarkerReduced"_s, i18nc("Document Template var description",
                "Marker value identifying items taxed with the reduced tax rate") },
            { u"taxPercentNum"_s, i18nc("Document Template var description",
                "Applicable tax rate in percent as numerical value, not localized") },
            { u"taxPercentStr"_s, i18nc("Document Template var description",
                "Applicable tax rate in percent localized as user string") },
            { u"taxSumNum"_s, i18nc("Document Template var description",
                "Total tax amount as numerical value, not localized") },
            { u"taxSumStr"_s, i18nc("Document Template var description",
                "Total tax amount localized as user string") },
            { u"timeOfSupplyEnd"_s, i18nc("Document Template var description",
                "End of the time of supply as localized string according to user settings") },
            { u"timeOfSupplyEndISO"_s, i18nc("Document Template var description",
                "End of the time of supply, ISO formatted, for XRechnung") },
            { u"timeOfSupplyMultiDay"_s, i18nc("Document Template var description",
                "True if the time of supply spans more than a single day") },
            { u"timeOfSupplyStart"_s, i18nc("Document Template var description",
                "Start of the time of supply as localized string according to user settings") },
            { u"timeOfSupplyStartISO"_s, i18nc("Document Template var description",
                "Start of the time of supply, ISO formatted, for XRechnung") },
            { u"timeOfSupplyValid"_s, i18nc("Document Template var description",
                "True if a valid time of supply is set on the document") },
            { u"objectName"_s, i18nc("Document Template var description",
                "General object name") }
        } },
        { u"me"_s, {
            { u"NAME"_s, i18nc("Document Template var description",
                "Real name of the contact") },
            { u"ORGANISATION"_s, i18nc("Document Template var description",
                "Organization of the contact, falls back to the real name") },
            { u"URL"_s, i18nc("Document Template var description",
                "Website URL of the contact") },
            { u"EMAIL"_s, i18nc("Document Template var description",
                "Preferred email address of the contact") },
            { u"PHONE"_s, i18nc("Document Template var description",
                "Work phone number of the contact") },
            { u"FAX"_s, i18nc("Document Template var description",
                "Fax number of the contact") },
            { u"MOBILE"_s, i18nc("Document Template var description",
                "Mobile phone number of the contact") },
            { u"POSTBOX"_s, i18nc("Document Template var description",
                "Post office box of the contact address") },
            { u"EXTENDED"_s, i18nc("Document Template var description",
                "Extended address line of the contact address") },
            { u"STREET"_s, i18nc("Document Template var description",
                "Street of the contact address") },
            { u"LOCALITY"_s, i18nc("Document Template var description",
                "Locality (city) of the contact address") },
            { u"REGION"_s, i18nc("Document Template var description",
                "Region (state) of the contact address") },
            { u"POSTCODE"_s, i18nc("Document Template var description",
                "Postal code of the contact address") },
            { u"COUNTRY"_s, i18nc("Document Template var description",
                "Country of the contact address") },
            { u"LABEL"_s, i18nc("Document Template var description",
                "Formatted address label of the contact") }
        } },
        { u"label"_s, {
            { u"NO_SHORT"_s, i18nc("Document Template var description",
                "Label for the position sequence number column, e.g. 'No.'") },
            { u"ITEM"_s, i18nc("Document Template var description",
                "Label for the item column, e.g. 'Item'") },
            { u"QUANTITY_SHORT"_s, i18nc("Document Template var description",
                "Label for the quantity column, e.g. 'Qty.'") },
            { u"UNIT"_s, i18nc("Document Template var description",
                "Label for the unit column, e.g. 'Unit'") },
            { u"PRICE"_s, i18nc("Document Template var description",
                "Label for the price column, e.g. 'Price'") },
            { u"SUM"_s, i18nc("Document Template var description",
                "Label for the sum column, e.g. 'Sum'") },
            { u"NET"_s, i18nc("Document Template var description",
                "Label for the net amount, e.g. 'Net'") },
            { u"VAT"_s, i18nc("Document Template var description",
                "Label for the value added tax, e.g. 'VAT'") },
            { u"TYPE"_s, i18nc("Document Template var description",
                "Label for the document type, e.g. 'Type'") },
            { u"PHONE"_s, i18nc("Document Template var description",
                "Label for the phone number, e.g. 'Phone'") },
            { u"FAX"_s, i18nc("Document Template var description",
                "Label for the fax number, e.g. 'Fax'") },
            { u"MOBILE"_s, i18nc("Document Template var description",
                "Label for the mobile number, e.g. 'Mobile'") },
            { u"EMAIL"_s, i18nc("Document Template var description",
                "Label for the email address, e.g. 'Email'") },
            { u"WEBSITE"_s, i18nc("Document Template var description",
                "Label for the website, e.g. 'Website'") },
            { u"PAGE"_s, i18nc("Document Template var description",
                "Label for the page number, e.g. 'Page'") },
            { u"PREDECESSOR"_s, i18nc("Document Template var description",
                "Label for the predecessor document number") },
            { u"PAGE_OF"_s, i18nc("Document Template var description",
                "The 'of' word in 'page X of Y'") },
            { u"DOC_NO"_s, i18nc("Document Template var description",
                "Label for the document number, e.g. 'Document No.'") },
            { u"DATE"_s, i18nc("Document Template var description",
                "Label for the document date, e.g. 'Date'") },
            { u"PROJECT"_s, i18nc("Document Template var description",
                "Label for the project, e.g. 'Project'") },
            { u"CUST_ID"_s, i18nc("Document Template var description",
                "Label for the customer ID, e.g. 'Customer ID'") },
            { u"CURRENCY_SIGN"_s, i18nc("Document Template var description",
                "The configured currency symbol") },
            { u"TIMEOFSUPPLY"_s, i18nc("Document Template var description",
                "Label for the time of supply, e.g. 'Time of supply'") }
        } },
        { u"items"_s, {
            { u"kind"_s, i18nc("Document Template var description",
                "Kind of the item, e.g. Position, Text, Demand or Alternative") },
            { u"itemNumber"_s, i18nc("Document Template var description",
                "Sequential number of the item within the document") },
            { u"text"_s, i18nc("Document Template var description",
                "Text of the item, as plain text") },
            { u"htmlText"_s, i18nc("Document Template var description",
                "Text of the item, as HTML") },
            { u"amount"_s, i18nc("Document Template var description",
                "Quantity of the item localized as user string") },
            { u"amountNum"_s, i18nc("Document Template var description",
                "Quantity of the item as numerical value, not localized") },
            { u"unit"_s, i18nc("Document Template var description",
                "Unit of the item, e.g. 'piece' or 'hour'") },
            { u"unitCode"_s, i18nc("Document Template var description",
                "UN/ECE Recommendation 20 unit code of the item, for XRechnung") },
            { u"unitPrice"_s, i18nc("Document Template var description",
                "Unit price of the item localized as user string") },
            { u"unitPriceNum"_s, i18nc("Document Template var description",
                "Unit price of the item as numerical value, not localized") },
            { u"nettoPrice"_s, i18nc("Document Template var description",
                "Net total price of the item localized as user string") },
            { u"nettoPriceNum"_s, i18nc("Document Template var description",
                "Net total price of the item as numerical value, not localized") },
            { u"taxMarker"_s, i18nc("Document Template var description",
                "Marker value identifying the tax rate applied to the item") },
            { u"objectName"_s, i18nc("Document Template var description",
                "General object name") }
        } },
        { u"epcqrcode"_s, {
            { u"valid"_s, i18nc("Document Template var description",
                "True if a valid EPC QR code was generated for the document") },
            { u"show"_s, i18nc("Document Template var description",
                "True if the EPC QR code should be shown on the document") },
            { u"svgfilename"_s, i18nc("Document Template var description",
                "File path of the generated EPC QR code SVG image") }
        } },
        { u"kraft"_s, {
            { u"VERSION"_s, i18nc("Document Template var description",
                "Kraft version string including code name") },
            { u"DB_SCHEME"_s, i18nc("Document Template var description",
                "Database schema version of the running Kraft instance") },
            { u"SYS_USER"_s, i18nc("Document Template var description",
                "System user name running Kraft") },
            { u"HOSTNAME"_s, i18nc("Document Template var description",
                "Host name of the machine running Kraft") }
        } }
    };

    const auto prefixIt = descriptions.constFind(prefix);
    if (prefix.isEmpty() || prefixIt == descriptions.constEnd()) {
        qDebug() << "No descriptions for prefix" << (prefix.isEmpty() ? "<empty>" : prefix);
        return {};
    }

    return prefixIt->value(name);
}

void loopProperties(QObject *obj, TemplateNameSpace& tns)
{
    auto *metaobject = obj->metaObject();
    int count = metaobject->propertyCount();
    QMap<QString, int> nameToIndx;

    for (int i=0; i < count; ++i) {
        QMetaProperty metaproperty = metaobject->property(i);
        const QString name = QString::fromUtf8(metaproperty.name());
        nameToIndx.insert(name, i);
    }

    for( const QString& name : nameToIndx.keys()) {
        int i = nameToIndx[name];
        QMetaProperty metaproperty = metaobject->property(i);
        const QVariant example = metaproperty.read(obj);
        QString desc{getDesc(tns.prefix(), name)};
        if (desc.isEmpty()) {
            qDebug() << "*** No description for" << name;
        }
        auto *tmplvar = new TemplateVariable(name, desc, example.toString());
        tns.addVar(tmplvar);
    }
}

void loopHash(const QVariantHash& hash, TemplateNameSpace &tns)
{
    auto keys = hash.keys();
    std::sort(keys.begin(), keys.end());
    for( const QString& name : keys) {
        const QVariant example = hash[name];
        QString desc{getDesc(tns.prefix(), name)};
        if (desc.isEmpty()) {
            qDebug() << "*** No description for" << name;
        }
        auto *tmplvar = new TemplateVariable(name, desc, example.toString());
        tns.addVar(tmplvar);
    }

}

}

// ==================================================================================

TemplateVariable::TemplateVariable(const QString& name, const QString& desc, const QString& example)
    :QObject(),
      _name(name), _desc(desc), _exampleVal(example)
{

}

// ==================================================================================

TemplateNameSpace::TemplateNameSpace(const QString& p)
    : _prefix{p}
{

}

TemplateNameSpace::~TemplateNameSpace()
{
    qDeleteAll(_vars);
}

GrantleeAllVarsTemplate::GrantleeAllVarsTemplate(const QString& tmplFile)
    :DocumentTemplate(tmplFile)
{

}

const QString GrantleeAllVarsTemplate::expand(const QString& uuid,
                     const KContacts::Addressee &myContact,
                     const KContacts::Addressee &customerContact)
{
    Q_UNUSED(customerContact)
    KraftDoc *doc = DocumentMan::self()->openDocumentByUuid(uuid);
    QString rendered;

    if (doc == nullptr) {
        _errorStr = i18n("Could not open document %1", uuid);
        return rendered;
    }

    qDebug() << "========================================";

    GrantleeFileTemplate gtmpl(_tmplFile);

    // == The contact variables
    TemplateNameSpace tnsContact(MeContactPrefix);
    tnsContact.setObjectName(i18n("Contact Variables"));
    tnsContact.setDesc(i18n("Variables of a contact, both own identity (prefix `me`) and the customer contact (prefix `customer`)"));

    const auto mtt = contactToVariantHash(myContact);
    loopHash(mtt, tnsContact);
    gtmpl.addToObjMapping(tnsContact.prefix(), &tnsContact);

    // == The document variables
    TemplateNameSpace tns(DocPrefix);
    tns.setObjectName(i18n("Document Variables"));
    tns.setDesc(i18n("Specific document variables"));

    loopProperties(doc, tns);
    gtmpl.addToObjMapping(tns.prefix(), &tns);

    // == The document labels
    TemplateNameSpace tnsLabels(LabelsPrefix);
    tnsLabels.setObjectName(i18n("Document Labels"));
    tnsLabels.setDesc(i18n("Document Labels, translated"));

    const auto labels = labelVariantHash();
    loopHash(labels, tnsLabels);
    gtmpl.addToObjMapping(tnsLabels.prefix(), &tnsLabels);

    // == The report item properties
    TemplateNameSpace tnsRepi(ItemsPrefix);
    tnsRepi.setObjectName(i18n("Report Item Variables"));
    tnsRepi.setDesc(i18n("Report items variables, from a loop over `doc.items`"));

    const auto items = doc->reportItemList();
    ReportItem *repi;
    ReportItem repiObj;
    if (items.count()>0) {
        repi = items.at(0);
    } else {
        repi = &repiObj;
    }
    loopProperties(repi, tnsRepi);
    gtmpl.addToObjMapping(tnsRepi.prefix(), &tnsRepi);

    // == The EPC Code
    TemplateNameSpace tnsEPC(EPCPrefix);
    tnsEPC.setObjectName(i18n("Kraft EPC QR Code"));
    tnsEPC.setDesc(i18n("A generated EPC QR Code"));
    QVariantHash qrc = generateQRCodeHash(doc);
    loopHash(qrc, tnsEPC);
    gtmpl.addToObjMapping(tnsEPC.prefix(), &tnsEPC);

    // == A few Kraft system values
    TemplateNameSpace tnsKraft(KraftPrefix);
    tnsKraft.setObjectName(i18n("Kraft Information Variables"));
    tnsKraft.setDesc(i18n("Kraft Information Variables"));

    const auto kraft = kraftVariantHash();
    loopHash(kraft, tnsKraft);
    gtmpl.addToObjMapping(tnsKraft.prefix(), &tnsKraft);

    bool ok;
    rendered = gtmpl.render(ok);


    delete doc;
    return rendered;
}