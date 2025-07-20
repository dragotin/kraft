/***************************************************************************
             DocumentSaverXML  - Save Documents as XML
                             -------------------
    begin                : Jan. 2021
    copyright            : (C) 2021 by Klaas Freitag
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
#include <QSqlQuery>
#include <QDir>
#include <QDomDocument>
#include <QFileDevice>
#include <QSaveFile>

#include "models/docbasemodel.h"

#include "documentsaverxml.h"
#include "docposition.h"
#include "docdigest.h"
#include "kraftdoc.h"
#include "unitmanager.h"
#include "defaultprovider.h"
#include "kraftattrib.h"
#include "xmldocindex.h"
#include "stringutil.h"

using namespace KraftXml;

namespace {


int xmlAppendItemsToGroup( QDomDocument& xmldoc, QDomElement itemGroupElem, KraftDoc *doc)
{
/*
    <type>Normal</type>
     <text>second item </text>
     <amount>4.40</amount>
     <unit>cbm</unit>
     <taxType>reduced</taxType>
     <unitprice>33.33</unitprice>
     <itemprice>146.65</itemprice>
     <tag>Pflanzen</tag>
     <tag>Work</tag>
*/
    int cnt {0};

    const auto positions = doc->positions();
    for (DocPosition *item : positions) {
        if (item->toDelete())
            continue;
        QDomElement itemType = xmldoc.createElement("item");

        itemType.appendChild(textElement(xmldoc, "type", item->typeStr()));
        itemType.appendChild(textElement(xmldoc, "text", item->text()));

        itemType.appendChild(textElement(xmldoc, "amount", QString::number(item->amount(), 'f', 2)));
        itemType.appendChild(textElement(xmldoc, "unit", item->unit().einheitSingular()));

        QString ttStr;
        DocPosition::Tax tt = item->taxType();
        // The Full Taxtype is default.
        if (tt == DocPosition::Tax::Full)
            ttStr = QStringLiteral("Full");
        else if (tt == DocPosition::Tax::Reduced)
            ttStr = QStringLiteral("Reduced");
        else if (tt == DocPosition::Tax::None)
            ttStr = QStringLiteral("None");
        else
            ttStr = QStringLiteral("Invalid");

        itemType.appendChild(textElement(xmldoc, "taxtype", ttStr));

        itemType.appendChild(textElement(xmldoc, "unitprice", QString::number(item->unitPrice().toDouble(), 'f', 2)));
        itemType.appendChild(textElement(xmldoc, "itemprice", QString::number(item->overallPrice().toDouble(), 'f', 2)));

        const QMap<QString, KraftAttrib> attribs = item->attributes();
        for(const auto &k : attribs) {
            QDomElement attribElem = xmldoc.createElement("attrib");
            itemType.appendChild(attribElem);
            attribElem.appendChild(textElement(xmldoc, "name", k.name()));
            attribElem.appendChild(textElement(xmldoc, "value", k.value().toString()));
            attribElem.appendChild(textElement(xmldoc, "type", k.typeString()));
        }

        const QStringList& tags = item->allTags();
        for(const QString& tag : tags) {
            itemType.appendChild(textElement(xmldoc, "tag", tag));
        }
        itemGroupElem.appendChild(itemType);
        cnt++;
    }
    return cnt;
}

QDomDocument xmlDocument(KraftDoc *doc)
{
    QDomDocument xmldoc( "kraftdocument" );
    QDomProcessingInstruction instr = xmldoc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
    xmldoc.appendChild(instr);

    QDomElement root = xmldoc.createElement( "kraftdocument" );
    root.setAttribute("schemaVersion", "1");
    xmldoc.appendChild( root );

    QDomElement meta = xmldoc.createElement( "meta" );
    root.appendChild(meta);
    meta.appendChild(textElement(xmldoc, "docType", doc->docType()));
    meta.appendChild(textElement(xmldoc, "docDesc", doc->whiteboard()));
    meta.appendChild(textElement(xmldoc, "currency", DefaultProvider::self()->locale()->currencySymbol(QLocale::CurrencyIsoCode)));
    meta.appendChild(textElement(xmldoc, "country", DefaultProvider::self()->locale()->territoryToString(DefaultProvider::self()->locale()->territory())));
    meta.appendChild(textElement(xmldoc, "locale", DefaultProvider::self()->locale()->languageToString(DefaultProvider::self()->locale()->language())));
    meta.appendChild(textElement(xmldoc, "ident", doc->ident() ) );
    if (doc->uuid().isEmpty())
        doc->createUuid();
    meta.appendChild(textElement(xmldoc, "uuid", doc->uuid()));
    const QDate d = doc->date();
    meta.appendChild(textElement(xmldoc, "date", d.toString(Qt::ISODate)));
    meta.appendChild(textElement(xmldoc, "state", doc->state().stateString()));

    // -------- taxes
    QDomElement taxReduced = xmldoc.createElement("tax");
    meta.appendChild(taxReduced);
    taxReduced.appendChild(textElement(xmldoc, "type", "Reduced"));
    double t = UnitManager::self()->reducedTax(doc->date());
    taxReduced.appendChild(textElement(xmldoc, "percent", QString::number(t, 'f', 2)));

    QDomElement taxFull = xmldoc.createElement("tax");
    meta.appendChild(taxFull);
    taxFull.appendChild(textElement(xmldoc, "type", "Full"));
    t = UnitManager::self()->tax(doc->date());
    taxFull.appendChild(textElement(xmldoc, "percent", QString::number(t, 'f', 2)));


    // -------- owner etc.
    QString owner = QString::fromLocal8Bit(qgetenv("USER"));
    if (owner.isEmpty())
        owner = QString::fromLocal8Bit(qgetenv("USERNAME"));
    if (!owner.isEmpty()) {
        meta.appendChild(textElement(xmldoc, "owner", owner));
    }

    QDateTime dt = doc->lastModified();
    if (!dt.isValid())
        dt = QDateTime::currentDateTime();
    meta.appendChild(textElement(xmldoc, "lastModified", dt.toString(Qt::ISODate)));

    // -------- predecessor
    const QString pred = doc->predecessor();
    if (!pred.isEmpty()) {
        meta.appendChild(textElement(xmldoc, "predecessor", pred));
    }

    // -------- doc attributes and tags future extensions
    const QMap<QString,KraftAttrib> attrs = doc->attributes();
    for (const auto &attr : attrs) {
        meta.appendChild(attr.toXml(xmldoc));
    }

    const auto allTags = doc->allTags();
    for (const QString& tag : allTags) {
        meta.appendChild(textElement(xmldoc, "tag", tag));
    }

    // **** Next toplevel: client
    QDomElement cust = xmldoc.createElement( "client" );
    root.appendChild( cust );
    cust.appendChild( textElement( xmldoc, "address", doc->address() ) );
    cust.appendChild( textElement( xmldoc, "clientId", doc->addressUid() ) );

    // **** Next toplevel: header
    QDomElement headerElem = xmldoc.createElement( "header" );
    root.appendChild( headerElem );

    const QString prjLabel = doc->projectLabel();
    if (!prjLabel.isEmpty()) {
        QDomElement projectElem = xmldoc.createElement( "project" );
        headerElem.appendChild( projectElem );
        projectElem.appendChild( textElement(xmldoc, "name", prjLabel));
    }

    // -------- time of supply
    const QDate tosStart = doc->timeOfSupplyStart().date();
    if (tosStart.isValid()) {
        QDate tosEnd   = doc->timeOfSupplyEnd().date();
        QDomElement tos = xmldoc.createElement("timeOfSupply");
        meta.appendChild(tos);
        tos.appendChild(textElement(xmldoc, "start", tosStart.toString(Qt::ISODate)));
        if (!tosEnd.isValid())
            tosEnd = tosStart;
        tos.appendChild(textElement(xmldoc, "end", tosEnd.toString(Qt::ISODate)));
    }

    headerElem.appendChild( textElement( xmldoc, "salut", doc->salut()));
    headerElem.appendChild( textElement( xmldoc, "preText", doc->preText()));
    // FIXME: add Header attributs

    // **** Next toplevel: itemGroup, for now only one
    QDomElement itemGroupElem = xmldoc.createElement( "itemGroup" );
    root.appendChild( itemGroupElem );
    itemGroupElem.appendChild( textElement(xmldoc, "name", QStringLiteral("General")));
    itemGroupElem.appendChild( textElement(xmldoc, "collapsed", QStringLiteral("false")));

    auto cnt = xmlAppendItemsToGroup(xmldoc, itemGroupElem, doc);
    qDebug() << "Amount of items went to XML" << cnt;

    // **** Next toplevel: footer
    QDomElement itemGroupFooter = xmldoc.createElement("footer");
    root.appendChild(itemGroupFooter);
    itemGroupFooter.appendChild( textElement( xmldoc, "postText", doc->postText()));
    itemGroupFooter.appendChild( textElement( xmldoc, "goodbye", doc->goodbye()));

    // **** Next toplevel: totals
    QDomElement itemGroupTotals = xmldoc.createElement("totals");
    root.appendChild(itemGroupTotals);
    itemGroupTotals.appendChild( textElement(xmldoc, "netto", QString::number(doc->nettoSum().toDouble(), 'f', 2)));

    QDomElement taxSumReduced = xmldoc.createElement("taxsum");
    itemGroupTotals.appendChild(taxSumReduced);
    taxSumReduced.appendChild(textElement(xmldoc, "type", "Reduced"));
    t = doc->reducedTaxSum().toDouble();
    taxSumReduced.appendChild(textElement(xmldoc, "total", QString::number(t, 'f', 2)));

    QDomElement taxSumFull = xmldoc.createElement("taxsum");
    itemGroupTotals.appendChild(taxSumFull);
    taxSumFull.appendChild(textElement(xmldoc, "type", "Full"));
    t = doc->fullTaxSum().toDouble();
    taxSumFull.appendChild(textElement(xmldoc, "total", QString::number(t, 'f', 2)));

    itemGroupTotals.appendChild( textElement(xmldoc, "brutto", QString::number(doc->bruttoSum().toDouble(), 'f', 2)));

    return xmldoc;
}

bool loadMetaBlock(const QDomDocument& domDoc, KraftDoc *doc)
{
    bool res {true};

    QDomElement kraftdocElem = domDoc.firstChildElement("kraftdocument");
    QDomElement metaElem = kraftdocElem.firstChildElement("meta");

    Q_ASSERT(!metaElem.isNull());

    QString t = childElemText(metaElem, "docType");
    doc->setDocType(t);

    t = childElemText(metaElem, "docDesc");
    doc->setWhiteboard(t);

    // Currently the locale of the docs is hardcoded to the locale Kraft is
    // running under. It can not be set into the document.
    t = childElemText(metaElem, "currency");
    t = childElemText(metaElem, "country");
    t = childElemText(metaElem, "locale");

    t = childElemText(metaElem, "ident");
    doc->setIdent(t);

    t = childElemText(metaElem, "uuid");
    doc->setUuid(t);

    QDate d = childElemDate(metaElem, "date");
    doc->setDate(d);

    t = childElemText(metaElem, "state");
    doc->state().setStateFromString(t);

    // Tax: Unused so far, as tax is taken from documentman,  which loads it according to the date of the doc.
    QDomElement taxElem = metaElem.firstChildElement("tax");
    while (!taxElem.isNull()) {
        t = childElemText(taxElem, "type");
        t = childElemText(taxElem, "value");
        taxElem = taxElem.nextSiblingElement("tax");
    }

    // owner
    t = childElemText(metaElem, "owner");
    doc->setOwner(t);

    // last modified
    t = childElemText(metaElem, "lastModified");
    QDateTime dt = QDateTime::fromString(t, Qt::ISODate);
    doc->setLastModified(dt);

    t = childElemText(metaElem, "predecessor");
    doc->setPredecessor(t);

    QDomElement attrElem = metaElem.firstChildElement("attrib");
    while (!attrElem.isNull()) {
        doc->setAttribute(KraftAttrib(attrElem));
        attrElem = attrElem.nextSiblingElement("attrib");
    }

    // Tags for future fun
    QDomElement tagElem = metaElem.firstChildElement("tag");
    while (!tagElem.isNull()) {
        t = tagElem.text();
        doc->addTag(t);
        tagElem = tagElem.nextSiblingElement("tag");
    }
    return res;
}

bool loadClientBlock(const QDomDocument& domDoc, KraftDoc *doc)
{
    bool res {true};

    QDomElement kraftdocElem = domDoc.firstChildElement("kraftdocument");
    QDomElement clientElem = kraftdocElem.firstChildElement("client");

    Q_ASSERT(!clientElem.isNull());

    const QString t = childElemText(clientElem, "address");
    doc->setAddress(t);
    const QString id = childElemText(clientElem, "clientId");
    doc->setAddressUid(id);

    return res;
}

bool loadHeaderBlock(const QDomDocument& domDoc, KraftDoc *doc)
{
    bool res {true};

    QDomElement kraftdocElem = domDoc.firstChildElement("kraftdocument");
    QDomElement headerElem = kraftdocElem.firstChildElement("header");;

    QDomElement prjElem = headerElem.firstChildElement("project");
    QString t = childElemText(prjElem, "name");
    doc->setProjectLabel(t);
    // TODO: There can also be a project ID

    const QDomElement tosElem = headerElem.firstChildElement("timeOfSupply");
    if (!tosElem.isNull()) {
        const QDate d = childElemDate(tosElem, "start");
        const QDate dEnd = childElemDate(tosElem, "end");
        doc->setTimeOfSupply(QDateTime(d, QTime(0, 0)),
                             QDateTime(dEnd, QTime(23, 59, 59)));
    }

    t = childElemText(headerElem, "salut");
    doc->setSalut(t);

    t = childElemText(headerElem, "preText");
    doc->setPreTextRaw(t);

    QDomElement customElem = headerElem.firstChildElement("attrib");
    while (!customElem.isNull()) {
        t = childElemText(customElem, "name");
        t = childElemText(customElem, "value");
        t = childElemText(customElem, "type");
        customElem = customElem.nextSiblingElement("attrib");
    }

    return res;
}

bool loadItems(const QDomDocument& domDoc, KraftDoc *doc)
{
    bool res {true};

    QDomElement kraftdocElem = domDoc.firstChildElement("kraftdocument");

    // TODO: There should be more than one item Groups
    QDomElement groupElem = kraftdocElem.firstChildElement("itemGroup");
    // TODO: itemGroup name and attribs.
    int itemCnt = 0;
    while( !groupElem.isNull()) {
        QDomElement itemElem = groupElem.firstChildElement("item");
        while (!itemElem.isNull()) {
            QString t = childElemText(itemElem, "type");
            DocPosition::Type itemType = DocPosition::typeStrToType(t);

            DocPosition *item = doc->createPosition(itemType);
            item->setPositionNumber(++itemCnt);
            t = childElemText(itemElem, "text");
            item->setText(t);

            t = childElemText(itemElem, "amount");
            double a = t.toDouble();
            item->setAmount(a);
            t = childElemText(itemElem, "unit");
            item->setUnit(UnitManager::self()->getUnit(t));

            t = childElemText(itemElem, "taxtype");
            item->setTaxType(t);

            t = childElemText(itemElem, "unitprice");
            item->setUnitPrice(Geld(t.toDouble()));
            t = childElemText(itemElem, "itemprice");

            Geld g(item->overallPrice());
            // qDebug() << "Geld" << g.toLocaleString() << t.toDouble();
            if (itemType == DocPosition::Type::Position)
                Q_ASSERT(!(g != Geld(t.toDouble())));

            QDomElement attrElem = itemElem.firstChildElement("attrib");
            while (!attrElem.isNull()) {
                const KraftAttrib attr(attrElem);
                item->setAttribute(attr);
                attrElem = attrElem.nextSiblingElement("attrib");
            }

            QDomElement tagElem = itemElem.firstChildElement("tag");
            QStringList tags;
            while (!tagElem.isNull()) {
                tags.append(tagElem.text());
                tagElem = tagElem.nextSiblingElement("tag");
            }
            if (tags.size() > 0) {
                item->setTags(tags);
            }

            // Go to next item
            itemElem = itemElem.nextSiblingElement("item");
        }
        groupElem = groupElem.nextSiblingElement("itemGroup");
    }

    return res;
}

bool loadFooter(const QDomDocument& domDoc, KraftDoc *doc)
{
    bool res {true};

    QDomElement kraftdocElem = domDoc.firstChildElement("kraftdocument");
    QDomElement footerElem = kraftdocElem.firstChildElement("footer");

    QString t = childElemText(footerElem, "postText");
    doc->setPostTextRaw(t);
    t = childElemText(footerElem, "goodbye");
    doc->setGoodbye(t);

    return res;
}

bool loadTotals(const QDomDocument& domDoc, XML::Totals& totals)
{
    bool res {true};

    QDomElement kraftdocElem = domDoc.firstChildElement("kraftdocument");
    QDomElement totalsElem = kraftdocElem.firstChildElement("totals");

    QString t = childElemText(totalsElem, "netto");
    totals._netto = Geld(t.toDouble());
    t = childElemText(totalsElem, "brutto");
    totals._brutto = Geld(t.toDouble());

    QDomElement taxSumElem = totalsElem.firstChildElement("taxSum");
    for( int i = 0; i < 2; i++) {
        t = childElemText(taxSumElem, "type");
        QString sum = childElemText(taxSumElem, "value");
        Geld g(sum.toDouble());
        if (t == "Reduced") {
            totals._redTax = g;
        }
        if (t == "Full") {
            totals._fullTax = g;
        }
        taxSumElem = taxSumElem.nextSiblingElement("taxSum");
    }
    return res;
}

} // namespace end

void DocumentSaverXML::setArchiveMode(bool am)
{
    _archiveMode = am;
}

QString DocumentSaverXML::xmlDocFileName(KraftDoc *doc)
{
    const QDate d = doc->date();

    QString path{DefaultProvider::self()->kraftV2Dir(DefaultProvider::KraftV2Dir::XmlDocs)};

    path.append(QString("/%1/%2/").arg(d.year(), 4, 10, QLatin1Char('0')).arg(d.month(), 2, 10, QLatin1Char('0')));
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(path);
    }

    const QString file = QString("%1.xml").arg(doc->uuid());

    return dir.filePath(file);
}

QString DocumentSaverXML::xmlDocFileNameFromIdent(const QString& id)
{
    XmlDocIndex indx;

    const QFileInfo p = indx.xmlPathByIdent(id);

    return  p.filePath();
}

DocumentSaverXML::DocumentSaverXML()
    : DocumentSaverBase(),
      _archiveMode(false)
{

}

bool DocumentSaverXML::verifyXmlFile(const QUrl& schemaFile, const QString& xmlFile)
{
    Q_UNUSED(schemaFile)
    Q_UNUSED(xmlFile)

    // FIXME implement a verification
    return true;
}

QString DocumentSaverXML::lastSavedFileName() const
{
    return _lastSaveFile;
}

XML::Totals DocumentSaverXML::getLastTotals() const
{
    return _totals;
}

bool DocumentSaverXML::saveDocument(KraftDoc *doc)
{
    bool result = false;
    if( ! doc ) return result;

    QDateTime saveLastModified = doc->lastModified();
    if (!_archiveMode) {
        doc->setLastModified(QDateTime::currentDateTime());
    }

    bool newState{false};
    if (doc->state().isNew()) {
        qDebug() << "Saving a new document!";
        newState = true; //
        // set document state to draft now for creating the save doc
        doc->state().setState(KraftDocState::State::Draft);
    } else if (!_archiveMode && doc->state().is(KraftDocState::State::Converted)) {
        // set a converted doc that was changed to Draft
        doc->state().setState(KraftDocState::State::Draft);
        // delete the current ident
        doc->setIdent(QString());
    }
    doc->createUuid(); // create a UUID just in case...

    QDomDocument xmldoc = xmlDocument(doc);
    const QString xml = xmldoc.toString();
    const QString xmlFile = xmlDocFileName(doc);

    // TODO: Write to temp file first, and move only if it validates.
    qDebug () << "Storing XML to " << xmlFile;

    bool re{false};
    QSaveFile file( xmlFile );
    if ( file.open( QIODevice::WriteOnly | QIODevice::Text) ) {
        re = file.write(xml.toUtf8());

        if (re) {
            re = file.commit();
        }
    }
    if (!re) {
        qDebug () << "Saving failed";
        doc->setLastModified(saveLastModified);
        return false;
    }


    _lastSaveFile = xmlFile;

    const QUrl schemaFile = QUrl::fromLocalFile(DefaultProvider::self()->locateFile("xml/kraftdoc.xsd"));
    result = verifyXmlFile(schemaFile, xmlFile);

    XmlDocIndex indx;
    if (newState) {
        indx.addEntry(doc);
    } else {
        indx.updateEntry(doc);
    }

    // qDebug () << "Saved document no " << doc->docID().toString() << endl;
    if (newState) {
        // retore the new state in the doc for subsequent funcs - no idea which ones...
        doc->state().setState(KraftDocState::State::New);
    }

    return result;
}

bool DocumentSaverXML::loadByUuid(const QString& uuid, KraftDoc *doc)
{
    if (uuid.isEmpty()) {
        qDebug() << "LoadByUuid: UUID to load is empty!";
        return false;
    }

    XmlDocIndex indx;
    const QFileInfo xmlFile = indx.xmlPathByUuid(uuid);

    return loadFromFile(xmlFile, doc);
}

bool DocumentSaverXML::loadByIdent(const QString& id, KraftDoc *doc)
{
    if (id.isEmpty()) {
        qDebug() << "Document Id to load is empty!";
        return false;
    }

    const QFileInfo xmlFile{xmlDocFileNameFromIdent(id)};

    return loadFromFile(xmlFile, doc);
}

bool DocumentSaverXML::loadFromFile(const QFileInfo& xmlFile, KraftDoc *doc, bool onlyMeta)
{
    if (!xmlFile.exists()) {
        qDebug() << "File to load does not exist" << xmlFile.filePath();
        return false;
    }

    if (!xmlFile.isReadable()) {
        qDebug() << "File to load not readable" << xmlFile.filePath();
        return false;
    }

    QFile file(xmlFile.filePath());
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Unable to open xml document file";
        return false;
    }

    QDomDocument _domDoc;

    const QByteArray arr = file.readAll();
    QDomDocument::ParseResult pe = _domDoc.setContent(arr);
    if (!pe) {
        qDebug() << "Unable to set file content as xml:" << pe.errorMessage << "in line"<<pe.errorLine;
        file.close();
        return false;
    }
    file.close();

    bool ok;

    ok = loadMetaBlock(_domDoc, doc);
    if (ok) {
        ok = loadClientBlock(_domDoc, doc);
    }

    if (!onlyMeta) {
        if (ok) {
            ok = loadHeaderBlock(_domDoc, doc);
        }
        if (ok) {
            ok = loadItems(_domDoc, doc);
        }
        if (ok) {
            ok = loadFooter(_domDoc, doc);
        }
        if (ok) {
            ok = loadTotals(_domDoc, _totals);
        }
    }

    if (ok) {
        qDebug() << "*** Kraft document" << xmlFile << "successfully loaded";
    }
    return ok;
}

int DocumentSaverXML::addDigestsToModel(DocBaseModel *model)
{
    int cnt{0};
    XmlDocIndex indx;

    const QMultiMap<QDate, QString> dateMap = indx.dateMap();
    QList<QDate> dates = dateMap.uniqueKeys();

    std::sort(dates.begin(), dates.end(), [](QDate const& l, QDate const& r) {
        return l < r;
    });

    for( const QDate& d : std::as_const(dates)) {
        const QList<QString> files = dateMap.values(d);
        QString yearStr = QString::number(d.year());

        for( const QString& fragm : files) {
            // we have the year and the uuid to find the entry from the index
            DocDigest dd = indx.findDigest(yearStr, fragm);
            model->addData(dd);
            cnt++;
        }
    }
    qDebug() << "Added"<< cnt << "digests to" << model->objectName();

    return cnt;
}

DocumentSaverXML::~DocumentSaverXML( )
{

}

/* END */

