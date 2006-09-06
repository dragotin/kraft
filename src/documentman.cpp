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

  QString qStr ="SELECT docID, ident, docType, clientID, lastModified, date FROM document ORDER BY date,lastModified desc";

  if( limit > 0 )
    qStr += " LIMIT " + QString::number( limit );
  qStr +=";";
  kdDebug() << "Sending sql string " << qStr << endl;

  QSqlQuery query( qStr, KraftDB::getDB() );

  if( query.isActive() ) {
    while( query.next() ) {
      DocDigest dig;
      dig.setId( dbID( query.value(0).toInt() ) );
      const QString ident = query.value(1).toString();
      dig.setIdent(    ident );
      dig.setType(     query.value(2).toString() );
      dig.setClientId( query.value(3).toString() );
      dig.setLastModified( query.value(4).toDate() );
      dig.setDate(     query.value(5).toDate() );
      kdDebug() << "Adding document "<< ident << " to the latest list" << endl;
      ret.append( dig );
    }
  }

  return ret;
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

