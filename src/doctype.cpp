/***************************************************************************
                 doctype.cpp - doc type class
                             -------------------
    begin                : Oct. 2007
    copyright            : (C) 2007 by Klaas Freitag
    email                : freitag@kde.org
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
#include <QLocale>

// application specific includes
#include "doctype.h"
#include "numbercycle.h"
#include "defaultprovider.h"
#include "stringutil.h"

/**
@author Klaas Freitag
*/

namespace {
const QString AllowDemandStr     {"AllowDemand"};
const QString AllowAlternativeStr{"AllowAlternative"};
const QString HidePricesStr      {"HidePrices"};
const QString SubstPartialInoiceStr      {"SubstractPartialInvoice"};
const QString PartialInvoiceStr  {"PartialInvoice"};
const QString XRechnungTmplStr   {"XRechnungTmpl"};
const QString WatermarkFileStr   {"watermarkFile"};
const QString DocTemplateFileStr {"docTemplateFile"};
const QString IdentNumberCycleStr{"identNumberCycle"};
const QString DocMergeIdentStr   {"docMergeIdent"};
const QString DayCounterDateStr  {"dayCounterDate"};
const QString DayCounterStr  {"dayCounter"};
const QString AppendPDFStr   {"AppendPDFFile"};
const QString DefaultTmplFileName {"invoice.gtmpl"};
const QString XRechnungEnabled {"XRechnungEnabled"};
}

DocType::DocType()
    : KraftObj()
{
    createUuid(); // Create a uuid

    QLocale loc;
    mLocale = loc.bcp47Name();
}

void DocType::parseXml(QDomDocument &domDoc)
{
    QDomElement dte = domDoc.firstChildElement("kraftDocType");
    mName = KraftXml::childElemText(dte, "name");
    mLocale = KraftXml::childElemText(dte, "locale");

    QDomElement followersElem = dte.firstChildElement("followers");
    QDomElement fElem = followersElem.firstChildElement("follower");
    while(!fElem.isNull()) {
        mFollowerList.append(fElem.text());
        fElem = fElem.nextSiblingElement("follower");
    }

    // generic KraftObj XML parsing
    QDomElement kobjElem = dte.firstChildElement("kobj");
    parseKobjXml(kobjElem);
    setModified(false);
}

const QString DocType::toXml() const
{
    const QString kncStr{"kraftDocType"};

    QDomDocument xmldoc(kncStr);
    QDomProcessingInstruction instr = xmldoc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
    xmldoc.appendChild(instr);

    QDomElement root = xmldoc.createElement(kncStr);
    root.setAttribute("schemaVersion", "1");
    xmldoc.appendChild( root );

    root.appendChild(KraftXml::textElement(xmldoc, "name", name()));
    root.appendChild(KraftXml::textElement(xmldoc, "locale", mLocale));
    if (mFollowerList.size() > 0) {
        QDomElement followerElem = xmldoc.createElement("followers");
        for( const QString& f: mFollowerList) {
            followerElem.appendChild(KraftXml::textElement(xmldoc, "follower", f));
        }
        root.appendChild(followerElem);
    }


    // generate the generic KraftObj-XML
    root.appendChild(kobjXml(xmldoc));

    return xmldoc.toString();
}


QString DocType::attributeValueString(const QString& attribName) const
{
    QString re;
    if (attribName.isEmpty()) {
        return re;
    }
    if (hasAttribute(attribName)) {
        const auto att = attribute(attribName);
        re = att.value().toString();
    }
    return re;
}

void DocType::setStringAttribute( const QString& attribName, const QString& val, const QString& defaultValue)
{
    const QString oldAttribVal = attributeValueString(attribName);
    if (oldAttribVal == val) {
        return;
    }
    if (val == defaultValue) {
        removeAttribute(attribName);
        return;
    }

    KraftAttrib attrib(attribName, val, KraftAttrib::Type::String);
    setAttribute(attrib);
}

void DocType::setDtFlag(const QString& name, bool f)
{
    // All default to false!
    if(!f) {
        removeTag(name);
    } else {
        addTag(name);
    }
}

bool DocType::dtFlag(const QString& str) const
{
    bool re = allTags().contains(str);
    return re;
}

bool DocType::allowDemand() const
{
    return dtFlag(AllowDemandStr);
}

void DocType::setAllowDemand(bool b)
{
    setDtFlag(AllowDemandStr, b);
}

bool DocType::allowAlternative() const
{
    return dtFlag(AllowAlternativeStr);
}

void DocType::setAllowAlternative(bool b)
{
    setDtFlag(AllowAlternativeStr, b);
}

bool DocType::pricesHidden() const
{
    return dtFlag(HidePricesStr);
}

void DocType::setPricesHidden(bool b)
{
    setDtFlag(HidePricesStr, b);
}

bool DocType::substractPartialInvoice() const
{
    return dtFlag(SubstPartialInoiceStr);
}

void DocType::setSubstractPartialInvoice(bool b)
{
    setDtFlag(SubstPartialInoiceStr, b);
}

bool DocType::partialInvoice() const
{
    return dtFlag(PartialInvoiceStr);
}

void DocType::setPartialInvoice(bool b)
{
    setDtFlag(PartialInvoiceStr, b);
}

QStringList DocType::follower() const
{
    return mFollowerList;
}

void DocType::setFollowers(const QStringList& followers)
{
    mFollowerList = followers;
}

QString DocType::numberCycleName() const
{
    QString re = NumberCycle::defaultName();
    if (hasAttribute(IdentNumberCycleStr)) {
        re = attribute(IdentNumberCycleStr).value().toString();
    }
    return re;
}

void DocType::setNumberCycleName( const QString& name )
{
    setStringAttribute(IdentNumberCycleStr, name, NumberCycle::defaultName());
}

/* This method looks for the template file for the doctype. The rule is:
 * 1. Set the filename to look for to the name of the doc type, lowercase
 *    and with spaces replaced
 * 2. Check for an attribute for this doc type.
 *    - if that is an absolute path, return it.
 *    - if not absolute, set the filename to look for to that value
 * 3. Look for the name in KRAFT_HOME
 * 4. Look for the name in rel. system Path
 * 5. Look for the name in QStandardPaths
 * 6. If still empty, fall back to default invoice.trml - which also
 *    sets ReportLab as default
 */

QString DocType::templateFile() const
{
    QString tmplFile;
    const auto dfp = DefaultProvider::self();
    QString searchStr;

    QString reportFileName{name().toLower()};
    reportFileName.replace(QChar(' '), QChar('_'));

    if ( hasAttribute(DocTemplateFileStr) ) {
        tmplFile = attribute(DocTemplateFileStr).value().toString();

        qDebug() << "Template File:" << tmplFile;
        if( !tmplFile.isEmpty() ) {
            QFileInfo fi(tmplFile);
            if( fi.isAbsolute() ) {
                return tmplFile;
            } else {
                // it is not an absolute file name, try to find it
                reportFileName = tmplFile;
            }
            tmplFile.clear();
        }
    }    

    // check for weasyprint template
    if (tmplFile.isEmpty())  {
        searchStr = QString("reports/%1.gtmpl").arg(reportFileName);
        tmplFile = dfp->locateFile(searchStr);
    }

    // not found - check invoice.trml
    if (tmplFile.isEmpty())  {
        searchStr = QString("reports/%1").arg(DefaultTmplFileName);
        tmplFile = dfp->locateFile(searchStr);
    }

    if( tmplFile.isEmpty() ) {
        qDebug () << "unable to find a template file for " << name();
    } else {
        qDebug () << "Found template file " << tmplFile;
    }
    return tmplFile;
}

void DocType::setTemplateFile( const QString& name )
{
    setStringAttribute(DocTemplateFileStr, name, DefaultTmplFileName);
}

int DocType::mergeIdent() const
{
    int re{0}; // 0 is default
    if (hasAttribute(DocMergeIdentStr)) {
        const auto att = attribute(DocMergeIdentStr);
        re = att.value().toInt();
    }
    return re;
}

void DocType::setMergeIdent( int ident )
{
    if (hasAttribute(DocMergeIdentStr) && ident == 0) {
        // remove attribute to default to zero
        removeAttribute(DocMergeIdentStr);
        return;
    }

    int oldAttribVal = mergeIdent();
    if (oldAttribVal != ident) {
        KraftAttrib attrib(DocMergeIdentStr, ident, KraftAttrib::Type::Integer);
        setAttribute(attrib);
    }
}

QString DocType::xRechnungTemplate() const
{
    return attributeValueString(XRechnungTmplStr);
}

void DocType::setXRechnungTemplate(const QString& tmpl)
{
    setStringAttribute(XRechnungTmplStr, tmpl);
}


QString DocType::watermarkFile() const
{
    return attributeValueString(WatermarkFileStr);
}

void DocType::setWatermarkFile( const QString& file )
{
    setStringAttribute(WatermarkFileStr, file);
}

QString DocType::appendPDF() const
{
    return attributeValueString(AppendPDFStr);
}

void DocType::setAppendPDFFile(const QString& file)
{
    setStringAttribute(AppendPDFStr, file);
}

// if hot, the id is updated in the database, otherwise not.

QString DocType::name() const
{
    return mName;
}

void DocType::setName( const QString& name )
{
    mName = name;
    setModified();
}

void DocType::setXRechnungEnabled(bool state)
{
    if (state != isXRechnungEnabled()) {
        setAttribute({XRechnungEnabled, state, KraftAttrib::Type::Bool});
    }
}

bool DocType::isXRechnungEnabled() const
{
    bool re{false};
    if (hasAttribute(XRechnungEnabled)) {
        re = attribute(XRechnungEnabled).value().toBool();
    }
    return re;
}

// ===============================================================
DocTypes::DocTypes()
    :XmlDirLister<DocType>(DefaultProvider::KraftV2Dir::DocTypes)
{

}

QStringList DocTypes::allNames()
{
    loadAll();
    QStringList li = map().keys();
    li.sort();
    return li;
}
