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

#include <utime.h>

#include "documentsaverdb.h"
#include "documentsaverxml.h"

Q_GLOBAL_STATIC(DocumentMan, mSelf)

DocumentMan *DocumentMan::self()
{
  return mSelf;
}

DocumentMan::DocumentMan()
{

}

DocGuardedPtr DocumentMan::copyDocument(const QString& copyFromIdent)
{
    DocGuardedPtr doc = new KraftDoc( );
    if ( ! copyFromIdent.isEmpty() ) {
        // copy the content from the source document to the new doc.
        DocGuardedPtr sourceDoc = openDocumentByIdent(copyFromIdent);
        if ( sourceDoc ) {
            *doc = *sourceDoc; // copies all data from the previous doc
            doc->setPredecessor(QString()); // clear the predecessor
        }
        doc->setLastModified( QDateTime::currentDateTime());
    }
    return doc;
}

DocGuardedPtr DocumentMan::createDocument( const QString& docType, const QString& copyFromIdent, const DocPositionList& listToCopy)
{
    DocGuardedPtr doc = new KraftDoc();
    // qDebug () << "new document ID: " << doc->docID().toString();

    if ( ! copyFromIdent.isEmpty() ) {
        // copy the content from the source document to the new doc.
        DocGuardedPtr sourceDoc = openDocumentByIdent(copyFromIdent);
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
                                          sourceDocType.name(), sourceDoc->ident()));
                    }
                }
            }

            doc->setPredecessor(sourceDoc->ident());

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

DocGuardedPtr DocumentMan::openDocumentByIdent( const QString& ident )
{
    // qDebug () << "Opening Document with id " << id << endl;
    DocGuardedPtr doc = new KraftDoc();
    if (!doc->openDocument(DefaultProvider::self()->documentPersister(), ident)) {
        delete doc;
        return nullptr;
    }
    return doc;
}

DocGuardedPtr DocumentMan::loadMetaFromFilename(const QString& xmlFile)
{
    DocumentSaverXML docLoad;
    DocGuardedPtr doc = new KraftDoc();
    if (!docLoad.loadFromFile(xmlFile, doc, true)) {
        delete doc;
        return nullptr;
    }
    return doc;
}

bool DocumentMan::saveDocument(KraftDoc* doc)
{
    if (!doc) {
        return false;
    }
    return doc->saveDocument(DefaultProvider::self()->documentPersister());
}

bool DocumentMan::reloadDocument(KraftDoc* doc)
{
    if (!doc) {
        return false;
    }
    return doc->reloadDocument(DefaultProvider::self()->documentPersister());

}

bool DocumentMan::convertDbToXml(const QString& docID, const QString& basePath)
{
    bool ok{true};

    DocumentSaverDB docLoad;
    KraftDoc doc;

    if (docLoad.loadByIdent(docID, &doc)) {

        DocumentSaverXML docSave;
        docSave.setBasePath(basePath);
        if (!docSave.saveDocument(&doc)) {
            qDebug() << "failed to save document as XML" << docID;
            ok = false;
        } else {
            // File was written successfully. Tweak the modification time to the
            // last modified date of the document.
            const QString& fileName = docSave.lastSavedFileName();
            const QDateTime& lastModified = doc.lastModified();

            if (lastModified.isValid()) {
            /*
             *        The utimbuf structure is:
             *
             *        struct utimbuf {
             *          time_t actime;       // access time
             *          time_t modtime;      // modification time
             *        };
             */
            struct tm time;
            time.tm_sec = lastModified.time().second();
            time.tm_min = lastModified.time().minute();
            time.tm_hour = lastModified.time().hour();
            time.tm_mday = lastModified.date().day();
            time.tm_mon = lastModified.date().month()-1;
            time.tm_year = lastModified.date().year()-1900;
            struct utimbuf utime_par;
            utime_par.modtime = mktime(&time);
            // utime_par.actime  = mktime()
            utime(fileName.toUtf8().constData(), &utime_par);
            } else {
                qDebug() << "Invalid time stamp for last modified for" << fileName;
            }
        }
    } else {
        qDebug() << "Failed to load from db" << docID;
        ok = false;
    }

    return ok;

}

DocumentMan::~DocumentMan()
{

}

