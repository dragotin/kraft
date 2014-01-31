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

#include <kdebug.h>
#include <kglobal.h>

#include "documentman.h"
#include "docdigest.h"
#include "kraftdb.h"
#include "documentsaverxml.h"
#include "owncloudsync.h"
#include "kraftsettings.h"

// DocGuardedPtr DocumentMan::mDocPtr = 0;
DocumentMap DocumentMan::mDocMap = DocumentMap();

DocumentMan *DocumentMan::self()
{
  K_GLOBAL_STATIC(DocumentMan, mSelf);
  return mSelf;
}

DocumentMan::DocumentMan()
  : mColumnList( "docID, ident, docType, docDescription, clientID, lastModified, date, country, language, projectLabel" ),
    mFullTax( -1 ),
    mReducedTax( -1 ),
    _oCSync( new ownCloudSync )
{
    DocumentSaverXML xmlDocSaver;

    _oCSync->setSyncDir(xmlDocSaver.storagePath());
    QTimer::singleShot(0, _oCSync, SLOT(slotCheckConnection()));

}

DocGuardedPtr DocumentMan::createDocument( const QString& docType, const QString& copyFromId )
{
  DocGuardedPtr doc = new KraftDoc( );
  doc->newDocument( docType );
  kDebug() << "new document ID: " << doc->docID().toString() << endl;
  mDocMap[doc->docID().toString()] = doc;

  if ( ! copyFromId.isEmpty() ) {
    // copy the content from the source document to the new doc.
    DocGuardedPtr sourceDoc = openDocument( copyFromId );
    if ( sourceDoc ) {
      *doc = *sourceDoc;
    }
  }

  return doc;
}

DocGuardedPtr DocumentMan::openDocument( const QString& id )
{
  kDebug() << "Opening Document with id " << id << endl;
  DocGuardedPtr doc;

  if( mDocMap.contains( id ) ){
    doc = mDocMap[id];
  } else {
    doc = new KraftDoc();
    doc->openDocument( id );
    mDocMap[id] = doc;
  }
  return doc;
}

QStringList DocumentMan::openDocumentsList()
{
  QStringList list;

  DocumentMap::Iterator it;
  for ( it = mDocMap.begin(); it != mDocMap.end(); ++it ) {
    DocGuardedPtr doc = it.value();
    list.append( doc->docIdentifier() );
  }
  return list;
}

void DocumentMan::clearTaxCache()
{
  mFullTax = -1;
  mReducedTax = -1;
}

bool DocumentMan::saveDocument( KraftDoc *doc )
{
    if( ! doc ) {
        return false;
    }

    if( 1|| KraftSettings::self()->useOwnCloud() ) {
        DocumentSaverXML xmlDocSaver;
        if( xmlDocSaver.saveDocument( doc ) ) {
            // saveDocumentIndex(doc);
            QString storage = xmlDocSaver.storagePath();

            _oCSync->startSync( storage );

        }
    }

    return true;
}

double DocumentMan::fullTax( const QDate& date )
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
  kDebug() << "** Datestring: " << dateStr;
  q.bindValue( ":date", dateStr );
  q.exec();

  if ( q.next() ) {
    mFullTax    = q.value( 0 ).toDouble();
    mReducedTax = q.value( 1 ).toDouble();
    mTaxDate = date;
    kDebug() << "* Taxes: " << mFullTax << "/" << mReducedTax << " from " << q.value( 2 ).toDate();
  }
  return ( mFullTax > 0 && mReducedTax > 0 );
}

DocumentMan::~DocumentMan()
{
    delete _oCSync;
}

