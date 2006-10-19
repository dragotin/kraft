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
#include <qsqlquery.h>
#include <qsqlcursor.h>

#include <kstaticdeleter.h>
#include <kdebug.h>

#include "documentman.h"
#include "docdigest.h"
#include "kraftdb.h"

static KStaticDeleter<DocumentMan> selfDeleter;

DocumentMan *DocumentMan::mSelf = 0;
// DocGuardedPtr DocumentMan::mDocPtr = 0;
DocumentMap DocumentMan::mDocMap = DocumentMap();

DocumentMan *DocumentMan::self()
{
  if ( !mSelf ) {
    selfDeleter.setObject( mSelf, new DocumentMan() );
  }
  return mSelf;
}

DocumentMan::DocumentMan()
{

}

DocDigestList DocumentMan::latestDocs( int limit )
{
  DocDigestList ret;

  QString qStr ="SELECT docID, ident, docType, clientID, lastModified, date FROM document ORDER BY date desc";

  if( limit > 0 )
    qStr += " LIMIT " + QString::number( limit );
  qStr +=";";
  kdDebug() << "Sending sql string " << qStr << endl;

  QSqlQuery query( qStr );// , KraftDB::getDB() );

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
  QSqlCursor archCur( "archdoc" );

  dig.setId( dbID( query.value(0).toInt() ) );
  const QString ident = query.value(1).toString();
  dig.setIdent(    ident );
  dig.setType(     query.value(2).toString() );
  dig.setClientId( query.value(3).toString() );
  dig.setLastModified( query.value(4).toDate() );
  dig.setDate(     query.value(5).toDate() );
  kdDebug() << "Adding document "<< ident << " to the latest list" << endl;

  archCur.select( "ident='" + ident +"'" );
  while ( archCur.next() ) {
    int id = archCur.value( "archDocID" ).toInt();
    QDateTime dt = archCur.value( "printDate" ).toDateTime();
    int state = archCur.value( "state" ).toInt();
    dig.addArchDocDigest( ArchDocDigest( dt, state,  id ) );
  }
  return dig;
}

DocDigestsTimelineList DocumentMan::docsTimelined()
{
  DocDigestsTimelineList retList; // a list of timelined digest objects

  QString qStr ="SELECT docID, ident, docType, clientID, lastModified, date, "
                "MONTH(date) as month, YEAR(date) as year FROM document ORDER BY date asc;";

  kdDebug() << "Sending sql string " << qStr << endl;

  QSqlQuery query( qStr ); // , KraftDB::getDB() );
  DocDigestsTimeline timeline;
  DocDigestList digests;

  if( query.isActive() ) {
    while( query.next() ) {
      DocDigest dig = digestFromQuery( query );
      int month = query.value( 6 /* month */ ).toInt();
      int year = query.value( 7 /* year */ ).toInt();
      kdDebug() << "Month: " << month << " in Year: " << year << endl;

      if ( timeline.month() == 0 ) timeline.setMonth( month );
      if ( timeline.year() == 0 )  timeline.setYear( year );


      kdDebug() << "timeline-month=" << timeline.month() << " while month=" << month << endl;
      if ( month != timeline.month() || year != timeline.year() ) {
        // a new month/year pair: set digestlist to timelineobject
        kdDebug() << "Opening new timeline" << endl;
        timeline.setDigestList( digests );

        kdDebug() << "appending for month " << timeline.month() << " with item cnt " << digests.count() << endl;
        retList.append( timeline );

        digests.clear();
        timeline.setMonth( month );
        timeline.setYear( year );
        digests.prepend( dig );

        timeline.clearDigestList();
      } else {
        digests.prepend( dig );
      }

    }
    timeline.setDigestList( digests );
    retList.append( timeline );

  }
  return retList;
}

DocGuardedPtr DocumentMan::createDocument()
{
  DocGuardedPtr doc = new KraftDoc( );
  doc->newDocument();
  mDocMap[doc->docID().toString()] = doc;

  return doc;
}

DocGuardedPtr DocumentMan::openDocument( const QString& id )
{
  kdDebug() << "Opening Document with id " << id << endl;
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

void DocumentMan::offerNewPosition( const DocPosition& pos )
{
  kdDebug() << "Offering new position to document!" << endl;

  DocumentMap::Iterator it;
  for ( it = mDocMap.begin(); it != mDocMap.end(); ++it ) {
    DocGuardedPtr doc = it.data();
    doc->slotAppendPosition( pos );
  }
}

QStringList DocumentMan::openDocumentsList()
{
  QStringList list;

  DocumentMap::Iterator it;
  for ( it = mDocMap.begin(); it != mDocMap.end(); ++it ) {
    DocGuardedPtr doc = it.data();
    list.append( doc->docIdentifier() );
  }
  return list;
}

DocumentMan::~DocumentMan()
{

}

