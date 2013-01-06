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

DocumentSaverXML::DocumentSaverXML( ) : DocumentSaverBase(),
                                      PosTypePosition( QString::fromLatin1( "Position" ) ),
                                      PosTypeExtraDiscount( QString::fromLatin1( "ExtraDiscount" ) ),
                                      PosTypeHeader( QString::fromLatin1( "Header" ) )
{

}

bool DocumentSaverXML::saveDocument(KraftDoc *doc )
{
    bool result = false;
    if( ! doc ) return result;

    kDebug() << "############### Document Save XML ################" << endl;
    XmlDocument xmlDoc;
    xmlDoc.setKraftDoc(doc);
    xmlDoc.writeFile( QLatin1String("/tmp/kraft.xml"));

#if 0

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

    if( doc->isNew() ) {
      kDebug() << "Doc is new, inserting" << endl;
      model.insertRecord(-1, record);
      model.submitAll();

      dbID id = KraftDB::self()->getLastInsertID();
      doc->setDocID( id );

      // get the uniq id and write it into the db
      DocType dt( doc->docType() );
      QString ident = dt.generateDocumentIdent( doc );
      doc->setIdent( ident );
      model.setFilter("docID=" + id.toString());
      model.select();
      if ( model.rowCount() > 0 ) {
        model.setData(model.index(0, 1), ident);
        model.submitAll();
      }

    } else {
      kDebug() << "Doc is not new, updating #" << doc->docID().intID() << endl;

      record.setValue( "docID", doc->docID().toString() );
      model.setRecord(0, record);
      model.submitAll();
    }

    saveDocumentPositions( doc );

    kDebug() << "Saved document no " << doc->docID().toString() << endl;
#endif
    return result;
}


void DocumentSaverXML::load( const QString& id, KraftDoc *doc )
{

}

DocumentSaverXML::~DocumentSaverXML( )
{

}

/* END */

