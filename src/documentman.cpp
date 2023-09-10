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
#include "docdigest.h"
#include "kraftdb.h"
#include "doctype.h"
#include "kraftsettings.h"

Q_GLOBAL_STATIC(DocumentMan, mSelf)

DocumentMan *DocumentMan::self()
{
  return mSelf;
}

DocumentMan::DocumentMan()
  : mFullTax( -1 ),
    mReducedTax( -1 )
{

}

DocGuardedPtr DocumentMan::copyDocument( const QString& copyFromId )
{
    DocGuardedPtr doc = new KraftDoc( );
    if ( ! copyFromId.isEmpty() ) {
        // copy the content from the source document to the new doc.
        DocGuardedPtr sourceDoc = new KraftDoc();
        sourceDoc->openDocument( copyFromId );
        if ( sourceDoc ) {
            *doc = *sourceDoc; // copies all data from the previous doc
            doc->setPredecessor(QString()); // clear the predecessor
        }
        doc->setLastModified( QDateTime::currentDateTime());
        delete sourceDoc;
        _openDocs[doc->docIdStr()] = doc;
    }
    return doc;
}

DocGuardedPtr DocumentMan::createDocument( const QString& docType, const QString& copyFromId, const DocPositionList& listToCopy)
{
    DocGuardedPtr doc;
    // qDebug () << "new document ID: " << doc->docID().toString();

    if ( ! copyFromId.isEmpty() ) {
        // copy the content from the source document to the new doc.
        doc = copyDocument( copyFromId );

        DocType sourceDocType( doc->docType() );
        Geld sourceNetto = doc->nettoSum();
        const QString sourceIdent = doc->ident();
        const QString sourcePre = doc->preTextRaw();
        const QString sourcePost = doc->postTextRaw();

        doc->setIdent(QString());
        doc->setDocType(docType); // sets the defaults for the new doc type
        doc->deleteItems();       // remove all items that exist so far
        doc->setPositionList(listToCopy, true);

        // check for relations between old and new doc
        // for new docs check if it should substract the sum of the predecessor doc
        DocType newDocType(docType);
        if( newDocType.substractPartialInvoice() ) {
            if( sourceDocType.partialInvoice()  ) {
                DocPosition *pos = doc->createPosition(DocPositionBase::Position);
                if(pos) {
                    Einheit e;
                    pos->setUnit(e);
                    pos->setAmount(1.0);
                    Geld ng(-1 * sourceNetto.toLong());

                    pos->setUnitPrice(ng);
                    pos->setText(i18nc("Text to be inserted into a doc, if the sum of another doc needs to be substracted "
                                       "ie. in a final invoice if there was a partial invoice before "
                                       "%1 is substited by the doc type, %2 by the id of the predecessor doc.",
                                       "Substract sum from %1 %2",
                                       sourceDocType.name(), sourceIdent));
                }
            }
        }

        doc->setPredecessor(sourceIdent);

        // Take the default pre- and posttext for the new docType, or, if that is empty, the texts of the old doc
        QString newText = DefaultProvider::self()->defaultText( docType, KraftDoc::Header );
        if (newText.isEmpty() ) {
            newText = sourcePre;
        }
        doc->setPreTextRaw(newText);

        newText = DefaultProvider::self()->defaultText( docType, KraftDoc::Footer );
        if (newText.isEmpty() ) {
            newText = sourcePost;
        }
        doc->setPostTextRaw(newText);
    } else {
        // Absolute new document
        doc = new KraftDoc;
        doc->setDocType(docType);
        doc->setPreTextRaw(DefaultProvider::self()->defaultText(docType, KraftDoc::Header));
        doc->setPostTextRaw(DefaultProvider::self()->defaultText(docType, KraftDoc::Footer));
        doc->setGoodbye( KraftSettings::greeting() );
    }

    // set the proper texts and other data
    doc->setLastModified( QDateTime::currentDateTime());

    return doc;
}

DocGuardedPtr DocumentMan::openDocumentbyIdent( const QString& ident )
{
    QSqlQuery q;
    q.prepare("SELECT docID FROM document WHERE ident=:ident");
    q.bindValue(":ident", ident);
    q.exec();
    if( q.next() ) {
        const QString id = q.value(0).toString();
        return openDocument(id);
    }
    return nullptr;
}

DocGuardedPtr DocumentMan::openDocument( const QString& id )
{
    // qDebug () << "Opening Document with id " << id;
    if (_openDocs.contains(id)) {
        // already open
        return _openDocs[id];
    }

    DocGuardedPtr doc = new KraftDoc();
    doc->openDocument( id );

    _openDocs[id] = doc;

    return doc;
}

void DocumentMan::closeDocument(DocGuardedPtr doc)
{
    if (_openDocs.contains(doc->docIdStr())) {
        _openDocs.remove(doc->docIdStr());
    }
}

void DocumentMan::clearTaxCache()
{
  mFullTax = -1;
  mReducedTax = -1;
}

double DocumentMan::tax( const QDate& date )
{
  if ( mFullTax < 0 || date != mTaxDate )
    readTaxes( date );
  return mFullTax;
}

double DocumentMan::reducedTax( const QDate& date )
{
  if ( mReducedTax < 0 || date != mTaxDate )
    readTaxes( date );
  return mReducedTax;
}

bool DocumentMan::readTaxes( const QDate& date )
{
  QString sql;
  QSqlQuery q;
  sql = "SELECT fullTax, reducedTax, startDate FROM taxes ";
  sql += "WHERE startDate <= :date ORDER BY startDate DESC LIMIT 1";

  q.prepare( sql );
  QString dateStr = date.toString( "yyyy-MM-dd" );
  // qDebug () << "** Datestring: " << dateStr;
  q.bindValue( ":date", dateStr );
  q.exec();

  if ( q.next() ) {
    mFullTax    = q.value( 0 ).toDouble();
    mReducedTax = q.value( 1 ).toDouble();
    mTaxDate = date;
    // qDebug () << "* Taxes: " << mFullTax << "/" << mReducedTax << " from " << q.value( 2 ).toDate();
  }
  return ( mFullTax > 0 && mReducedTax > 0 );
}

DocumentMan::~DocumentMan()
{

}

