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

DocumentMan *DocumentMan::self()
{
  K_GLOBAL_STATIC(DocumentMan, mSelf);
  return mSelf;
}

DocumentMan::DocumentMan()
  : mColumnList( "docID, ident, docType, docDescription, clientID, lastModified, date, country, language, projectLabel" ),
    mFullTax( -1 ),
    mReducedTax( -1 )
{

}

DocDigestList DocumentMan::latestDocs( int limit )
{
  DocDigestList ret;

  QString qStr = QString( "SELECT %1 FROM document ORDER BY date" ).arg( mColumnList );

  if( limit > 0 )
    qStr += " LIMIT " + QString::number( limit );
  qStr +=";";
  kDebug() << "Sending sql string " << qStr << endl;

  QSqlQuery query( qStr );

  if( query.isActive() ) {
    while( query.next() ) {
      ret.prepend( digestFromQuery( query ) );
    }
  }

  return ret;
}

DocDigest DocumentMan::digestFromQuery( QSqlQuery& query )
{
  DocDigest dig;

  dig.setId( dbID( query.value(0).toInt() ) );
  const QString ident = query.value(1).toString();
  dig.setIdent( ident );
  dig.setType(  query.value(2).toString() );
  dig.setWhiteboard( KraftDB::self()->mysqlEuroDecode( query.value( 3 ).toString() ) );
  dig.setClientId( query.value(4).toString() );
  dig.setLastModified( query.value(5).toDateTime() );
  dig.setDate(     query.value(6).toDate() );
  dig.setCountryLanguage(  query.value( 7 ).toString(), query.value( 8 ).toString() );
  dig.setProjectLabel( query.value( 9 ).toString() );
  // kDebug() << "Adding document "<< ident << " to the latest list" << endl;

  QSqlQuery q;
  q.prepare("SELECT archDocId, printDate, state FROM archdoc WHERE ident=:ident");
  q.bindValue(":ident", ident);
  q.exec();
  while ( q.next() ) {
    int id = q.value( 0 ).toInt();
    QDateTime dt = q.value(1).toDateTime();
    int state = q.value( 2 ).toInt();
    dig.appendArchDocDigest( ArchDocDigest( dt, state, ident, id ) );
  }
  return dig;
}

DocDigestsTimelineList DocumentMan::docsTimelined()
{
  DocDigestsTimelineList retList; // a list of timelined digest objects

  //mColumnList = "docID, ident, docType, docDescription, clientID, lastModified, date, country, language, projectLabel";
  QString qStr;
  QVariant v = QSqlDatabase::database().driver()->handle();
  kDebug() << "Database:" << v.typeName();
  if(v.isValid() && qstrcmp(v.typeName(), "MYSQL*")==0)
    qStr = QString( "SELECT %1, MONTH(date) as month, YEAR(date) as year FROM document ORDER BY date asc;" ).arg( mColumnList );
  else if(v.isValid() && qstrcmp(v.typeName(), "sqlite3*")==0)
    qStr = QString("SELECT %1, strftime('%m', date) as month, strftime('%Y',date) as year FROM document ORDER BY date asc;" ).arg( mColumnList );
  QSqlQuery query( qStr );
  DocDigestsTimeline timeline;
  DocDigestList digests;

  if( query.isActive() ) {
    while( query.next() ) {
      DocDigest dig = digestFromQuery( query );
      int month = query.value( 10 /* month */ ).toInt();
      int year = query.value( 11 /* year */ ).toInt();
      // kDebug() << "Month: " << month << " in Year: " << year << endl;

      if ( timeline.month() == 0 ) timeline.setMonth( month );
      if ( timeline.year() == 0  ) timeline.setYear( year );

      // kDebug() << "timeline-month=" << timeline.month() << " while month=" << month << endl;
      if ( month != timeline.month() || year != timeline.year() ) {
        // a new month/year pair: set digestlist to timelineobject
        timeline.setDigestList( digests );

        retList.append( timeline );

        digests.clear();
        digests.prepend( dig );

        timeline.clearDigestList();
        timeline.setMonth( month );
        timeline.setYear( year );
      } else {
        digests.prepend( dig );
        // kDebug() << "Prepending to digests lists: " << dig.date() << endl;
      }
    }
    kDebug() << "Final append !" << endl;
    timeline.setDigestList( digests );
    retList.append( timeline );

  }
  return retList;
}

DocGuardedPtr DocumentMan::createDocument( const QString& docType, const QString& copyFromId )
{
  DocGuardedPtr doc = new KraftDoc( );
  doc->newDocument( docType );
  kDebug() << "new document ID: " << doc->docID().toString() << endl;

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

}

