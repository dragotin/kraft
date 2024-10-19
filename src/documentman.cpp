/***************************************************************************
                       documentman.cpp  - Document Manager
                             -------------------
    begin                : 2006
    copyright            : (C) 2006 by Klaas Freitag
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
#include <QSqlQuery>
#include <QSqlDriver>

#include <QDebug>

#include <klocalizedstring.h>

#include "documentman.h"
#include "defaultprovider.h"
#include "doctype.h"
#include "kraftsettings.h"

#include "defaultprovider.h"
#include "documentsaverxml.h"

Q_GLOBAL_STATIC(DocumentMan, mSelf)

DocumentMan *DocumentMan::self()
{
  return mSelf;
}

DocumentMan::DocumentMan()
{

}

DocGuardedPtr DocumentMan::copyDocument(const QString& copyFromUuid)
{
    DocGuardedPtr doc = new KraftDoc( );
    if ( ! copyFromUuid.isEmpty() ) {
        // copy the content from the source document to the new doc.
        DocGuardedPtr sourceDoc = openDocumentByUuid(copyFromUuid);
        if ( sourceDoc ) {
            *doc = *sourceDoc; // copies all data from the previous doc
            doc->setPredecessor(QString()); // clear the predecessor
            delete sourceDoc;
        }
        doc->setLastModified( QDateTime::currentDateTime());
    }
    return doc;
}

DocGuardedPtr DocumentMan::createDocument( const QString& docType, const QString& copyFromUuid, const DocPositionList& listToCopy)
{
    DocGuardedPtr doc = new KraftDoc();
    // qDebug () << "new document ID: " << doc->docID().toString();

    if ( ! copyFromUuid.isEmpty() ) {
        // copy the content from the source document to the new doc.
        DocGuardedPtr sourceDoc = openDocumentByUuid(copyFromUuid);
        if ( sourceDoc ) {
            *doc = *sourceDoc; // copies all data from the previous doc
            doc->setIdent(QString());
            doc->setDocType(docType); // sets the defaults for the new doc type
            doc->deleteItems();       // remove all items that exist so far
            doc->setPositionList(listToCopy, true);

            // check for relations between old and new doc
            DocType sourceDocType( sourceDoc->docType() );
            // for new docs check if it should substract the sum of the predecessor doc
            DocType newDocType(docType);
            if( newDocType.substractPartialInvoice() ) {
                if( sourceDocType.partialInvoice()  ) {
                    Geld g = sourceDoc->nettoSum();
                    DocPosition *pos = doc->createPosition(DocPositionBase::Position);
                    if(pos) {
                        Einheit e;
                        pos->setUnit(e);
                        pos->setAmount(1.0);
                        Geld ng(-1 * g.toLong());

                        pos->setUnitPrice(ng);
                        pos->setText(i18nc("Text to be inserted into a doc, if the sum of another doc needs to be substracted "
					   "ie. in a final invoice if there was a partial invoice before"
					   "%1 is substited by the doc type, %2 by the id of the predecessor doc.",
					   "Substract sum from %1 %2",
                                          sourceDocType.name(), sourceDoc->docIdentifier()));
                    }
                }
            }

            doc->setPredecessor(sourceDoc->uuid());

            // Take the default pre- and posttext for the new docType, or, if that is empty, the texts of the old doc
            QString newText = DefaultProvider::self()->defaultText( docType, KraftDoc::Part::Header );
            if (newText.isEmpty() ) {
                newText = sourceDoc->preTextRaw();
            }
            doc->setPreTextRaw(newText);

            newText = DefaultProvider::self()->defaultText( docType, KraftDoc::Part::Footer );
            if (newText.isEmpty() ) {
                newText = sourceDoc->postTextRaw();
            }
            doc->setPostTextRaw(newText);

            delete sourceDoc;
        }
    } else {
        // Absolute new document
        doc->setDocType(docType);
        doc->setPreTextRaw(DefaultProvider::self()->defaultText(docType, KraftDoc::Part::Header));
        doc->setPostTextRaw(DefaultProvider::self()->defaultText(docType, KraftDoc::Part::Footer));
        doc->setGoodbye( KraftSettings::greeting() );
    }

    // set the proper texts and other data
    doc->setLastModified( QDateTime::currentDateTime());

    return doc;
}

DocGuardedPtr DocumentMan::openDocumentByUuid(const QString& uuid)
{
    // qDebug () << "Opening Document with uuid" << uuid << endl;
    DocGuardedPtr doc = new KraftDoc();
    if (!doc->openDocument(DefaultProvider::self()->documentPersister(), uuid)) {
        delete doc;
        return nullptr;
    }
    return doc;
}

bool DocumentMan::loadMetaFromFilename(const QString& xmlFile, KraftDoc *doc)
{
    DocumentSaverXML docLoad;
    QFileInfo fi(xmlFile);

    if (doc && docLoad.loadFromFile(fi, doc, true)) {
        return true;
    }
    qDebug() << "Failed to load file" << xmlFile;
    return false;
}

void DocumentMan::setDocProcessingError(const QString& errStr)
{
    Q_UNUSED(errStr);
}

bool DocumentMan::saveDocument(KraftDoc* doc)
{
    if (!doc) {
        return false;
    }
    bool ok = doc->saveDocument(DefaultProvider::self()->documentPersister());

    return ok;
}

bool DocumentMan::reloadDocument(KraftDoc* doc)
{
    if (!doc) {
        return false;
    }
    return doc->reloadDocument(DefaultProvider::self()->documentPersister());

}

DocumentMan::~DocumentMan()
{

}

