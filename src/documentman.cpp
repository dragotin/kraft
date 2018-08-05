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

#include "documentman.h"
#include "docdigest.h"
#include "kraftdb.h"

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

DocGuardedPtr DocumentMan::createDocument( const QString& docType, const QString& copyFromId )
{
    DocGuardedPtr doc = new KraftDoc( );
    doc->newDocument( docType );
    // qDebug () << "new document ID: " << doc->docID().toString() << endl;

    if ( ! copyFromId.isEmpty() ) {
        // copy the content from the source document to the new doc.
        DocGuardedPtr sourceDoc = openDocument( copyFromId );
        if ( sourceDoc ) {
            *doc = *sourceDoc;
            doc->setPredecessor(sourceDoc->ident());
            delete sourceDoc;
        }
    }

    return doc;
}

DocGuardedPtr DocumentMan::openDocument( const QString& id )
{
  // qDebug () << "Opening Document with id " << id << endl;
  DocGuardedPtr doc;

  doc = new KraftDoc();
  doc->openDocument( id );
  return doc;
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

