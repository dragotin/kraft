/***************************************************************************
             templatesaverbase  -
                             -------------------
    begin                : 2005-20-01
    copyright            : (C) 2005 by Klaas Freitag
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

// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>

#include <QDir>
#include <QSqlQuery>
#include <QtSql>

#include "documentsaverxml.h"
#include "docposition.h"
#include "kraftdoc.h"
#include "kraftdb.h"
#include "unitmanager.h"
#include "dbids.h"
#include "kraftsettings.h"
#include "doctype.h"
#include "xmldocument.h"
/*
 * Use the kxml_compiler generated XML classes, covered by xmldocument class.
 */
using namespace KraftXml;

DocumentSaverXML::DocumentSaverXML( ) : DocumentSaverBase(),
                                      PosTypePosition( QString::fromLatin1( "Position" ) ),
                                      PosTypeExtraDiscount( QString::fromLatin1( "ExtraDiscount" ) ),
                                      PosTypeHeader( QString::fromLatin1( "Header" ) )
{

}

QString DocumentSaverXML::storagePath()
{
    QString path;
    KStandardDirs stdDirs;

    path = KraftSettings::self()->xmlDocPath();

    if(path.isEmpty()) {
        path = stdDirs.saveLocation( "data", "kraft/documents", true );
    }
    if ( ! path.endsWith( QLatin1Char('/'))) path.append(QLatin1Char('/'));

    QDir dir(path);
    if( !dir.exists() ) {
        dir.mkpath(path);
    }
    if( !dir.exists() ) {
        kDebug() << "ERR: No document save path!";
        path = QLatin1String("/tmp/"); // best compromise...
    }

    return path;
}

// The incoming id is a database id from the documents table.
// It needs to be converted to the ident.
QString DocumentSaverXML::loadFileName( const QString& dbId )
{
    if( dbId.isEmpty() ) return QString::null;

    QSqlQuery query("SELECT ident FROM document where docID="+dbId);
    QString ident;

    while (query.next()) {
         ident = query.value(0).toString();
    }

    if( ident.isEmpty() ) {
        qDebug() << "Could not retrieve doc ident from database cache!";
        return QString::null;
    }

    QString path = storagePath();

    QString file("kraft-");
    file.append( ident );
    file.append(QLatin1String(".xml"));

    QFileInfo fi;
    fi.setFile(path, file);

    return fi.canonicalFilePath();
}

// finds a not yet used fiel name, does never overwrite an existing file.
QString DocumentSaverXML::saveFileName( const QString& ident )
{
    if( ident.isEmpty() ) return QString::null;

    QString path = storagePath();

    QString file("kraft-");
    file.append( ident );
    file.append(QLatin1String(".xml"));

    QFileInfo fi;
    fi.setFile(path, file);

    return fi.absoluteFilePath();
}

bool DocumentSaverXML::saveDocument(KraftDoc *doc )
{
    bool result = false;
    if( ! doc ) return result;

    kDebug() << "############### Document Save XML ################" << endl;
    XmlDocument xmlDoc;
    if( doc->isNew() ) {
        DocType dt( doc->docType() );
        QString ident = dt.generateDocumentIdent( doc );
        doc->setIdent( ident );
    }
    xmlDoc.setKraftDoc(doc);

    QString fileName = saveFileName(doc->ident());
    kDebug() << "Saving to file name " << fileName << " is new: " << doc->isNew();
    if( xmlDoc.writeFile(fileName) ) {
        saveDocumentIndex(doc);
    }

    return true;
}


void DocumentSaverXML::load( const QString& id, KraftDoc *doc )
{
    QString fileName = loadFileName(id);

    kDebug() << "############### Document Load XML ################" << endl;
    bool ok;
    Kraftdocument xmlDoc;
    xmlDoc = Kraftdocument::parseFile( fileName, &ok );

    if( ! ok ) {
        qDebug() << "FATAL: Failed to parse XML document!";
        return;
    }

    dbID dbid;
    dbid = id;
    doc->setDocID(dbid);
    doc->setLastModified( QDate::fromString(xmlDoc.lastModified()) );

    doc->setCountryLanguage( xmlDoc.meta().country(), xmlDoc.meta().language() );

    doc->setAddress( xmlDoc.client().address() );
    doc->setAddressUid( xmlDoc.client().clientId() );

    doc->setDate( QDate::fromString( xmlDoc.header().date()) );
    // doc->setDocID(  );
    doc->setDocType( xmlDoc.header().docType() );
    doc->setIdent( xmlDoc.header().ident() );
    doc->setPreText( xmlDoc.header().preText() );
    doc->setProjectLabel( xmlDoc.header().project() );
    doc->setSalut( xmlDoc.header().salut() );
    doc->setWhiteboard( xmlDoc.header().whiteboard());
    QDate d = QDate::fromString( xmlDoc.header().date(), Qt::ISODate );
    doc->setDate( d );
    doc->setProjectLabel( xmlDoc.header().project() );

    doc->setGoodbye( xmlDoc.footer().goodbye() );
    doc->setPostText( xmlDoc.footer().postText() );

    // parse items.
    Items items = xmlDoc.items();
    foreach( Item item, items.itemList() ) {
        DocPosition *dp = doc->createPosition(DocPositionBase::Position);
        bool ok;
        double amount = item.amount().toDouble(&ok);
        dp->setAmount( amount );
        dp->setPositionNumber( item.number() );
        dp->setText( item.text() );

        dp->setTaxType( item.taxType().toInt(&ok) );
        Einheit unit( item.unit() );
        dp->setUnit( unit );

        double money = item.unitprice().toDouble( &ok );
        Geld g(money/100.0);
        dp->setUnitPrice( g );

        ItemAttribute::List attribs = item.itemAttributeList();
        foreach( ItemAttribute attrib, attribs ) {
            if( attrib.name() == QLatin1String("tag") ) {
                dp->setTag( attrib.value() );
            } else {
                Attribute attribute( attrib.name() );
                attribute.setValue( attrib.value() );
                dp->setAttribute( attribute );
            }
        }
    }
}

DocumentSaverXML::~DocumentSaverXML( )
{

}

// Index-funktions. Writes document table with index data to display the
// doc overview conveniently.
bool DocumentSaverXML::saveDocumentIndex(KraftDoc *doc )
{
    bool result = false;
    if( ! doc ) return result;

    QSqlTableModel model;
    model.setTable("document");
    model.setEditStrategy( QSqlTableModel::OnManualSubmit );
    QSqlRecord record;

    kDebug() << "############### Document Save ################" << endl;

    if( doc->isNew() ) {
        record = model.record();
    } else {
      model.setFilter("docID=" + doc->docID().toString());
      model.select();
      if ( model.rowCount() > 0 ) {
        record = model.record(0);
      } else {
        kError() << "Could not select document record" << endl;
        return result;
      }
       // The document was already saved.
    }

    fillDocumentBuffer( record, doc );

    if( !doc->isNew() ) {
        QString docID = doc->docID().toString();
        kDebug() << "Doc is not new, updating #" << docID << endl;

        record.setValue( "docID", docID );
        model.setRecord(0, record);
        model.submitAll();
    } else {
        model.insertRecord(-1, record);
        model.submitAll();
        dbID id = KraftDB::self()->getLastInsertID();
        doc->setDocID( id );
    }
    kDebug() << "Saved document no " << doc->docID().toString() << endl;

    return result;
}

void DocumentSaverXML::fillDocumentBuffer( QSqlRecord &buf, KraftDoc *doc )
{
    if( doc ) {
      kDebug() << "Adressstring: " << doc->address() << endl;
      buf.setValue( "ident",    doc->ident() );
      buf.setValue( "docType",  doc->docType() );
      buf.setValue( "docDescription", KraftDB::self()->mysqlEuroEncode( doc->whiteboard() ) );
      buf.setValue( "clientID", doc->addressUid() );
      buf.setValue( "clientAddress", doc->address() );
      buf.setValue( "salut",    doc->salut() );
      buf.setValue( "goodbye",  doc->goodbye() );
      buf.setValue( "date",     doc->date() );
      // do not set that because mysql automatically updates the timestamp and
      // sqlite3 has a trigger for it.
      buf.setValue( "lastModified", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
      buf.setValue( "pretext",  KraftDB::self()->mysqlEuroEncode( doc->preText() ) );
      buf.setValue( "posttext", KraftDB::self()->mysqlEuroEncode( doc->postText() ) );
      buf.setValue( "country",  doc->country() );
      buf.setValue( "language", doc->language() );
      buf.setValue( "projectLabel",  doc->projectLabel() );
    }
}

/* END */

