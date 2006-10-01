/***************************************************************************
                       archiveman.cpp  - Archive Manager
                             -------------------
    begin                : July 2006
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
#include <qsqlcursor.h>
#include <qsqlrecord.h>
#include <qsqlindex.h>
#include <qfile.h>
#include <qtextstream.h>

#include <kstaticdeleter.h>
#include <kstandarddirs.h>

#include <kdebug.h>

#include "archiveman.h"
#include "kraftdoc.h"
#include "kraftdb.h"
#include "unitmanager.h"
#include "dbids.h"
#include "kraftsettings.h"

static KStaticDeleter<ArchiveMan> selfDeleter;

ArchiveMan* ArchiveMan::mSelf = 0;

ArchiveMan *ArchiveMan::self()
{
  if ( !mSelf ) {
    selfDeleter.setObject( mSelf, new ArchiveMan() );
  }
  return mSelf;
}

ArchiveMan::ArchiveMan()
{

}

ArchiveMan::~ArchiveMan()
{

}

dbID ArchiveMan::archiveDocument( KraftDoc *doc )
{
  if( ! doc ) return dbID();

  mDomDoc = archiveDocumentXml( doc );

  mCachedDocId = archiveDocumentDb( doc );

  return mCachedDocId;
}

QDomElement ArchiveMan::xmlTextElement( QDomDocument doc, const QString& name, const QString& value )
{
  QDomElement elem = doc.createElement( name );
  QDomText t = doc.createTextNode( value );
  elem.appendChild( t );
  return elem;
}

QDomDocument ArchiveMan::archiveDocumentXml( KraftDoc *doc )
{
  QDomDocument xmldoc( "kraftdocument" );
  QDomElement root = xmldoc.createElement( "kraftdocument" );
  xmldoc.appendChild( root );
  QDomElement cust = xmldoc.createElement( "client" );
  root.appendChild( cust );
  cust.appendChild( xmlTextElement( xmldoc, "address", doc->address() ) );
  cust.appendChild( xmlTextElement( xmldoc, "clientId", doc->addressUid() ) );

  QDomElement docElem = xmldoc.createElement( "docframe" );
  root.appendChild( docElem );
  docElem.appendChild( xmlTextElement( xmldoc, "docType", doc->docType() ) );
  docElem.appendChild( xmlTextElement( xmldoc, "ident", doc->ident() ) );
  docElem.appendChild( xmlTextElement( xmldoc, "preText", doc->preText() ) );
  docElem.appendChild( xmlTextElement( xmldoc, "postText", doc->postText() ) );
  docElem.appendChild( xmlTextElement( xmldoc, "salut", doc->salut() ) );
  docElem.appendChild( xmlTextElement( xmldoc, "goodbye", doc->goodbye() ) );

  docElem.appendChild( xmlTextElement( xmldoc, "date",
                                       KGlobal().locale()->formatDate( doc->date() ) ) );

  root.appendChild( doc->positions().domElement( xmldoc ) );

  QString xml = xmldoc.toString();
  // kdDebug() << "Resulting XML: " << xml << endl;

  QString path = KraftSettings::self()->xmlArchivePath();
  if ( path.isEmpty() ) {
    KStandardDirs stdDirs;
    path = stdDirs.saveLocation( "data", "kraft/archiveXml", true );
  }

  QString xmlFile = QString( "%1%2.xml" ).arg( path ).arg( doc->ident() );
  kdDebug() << "Storing to " << xmlFile << endl;

  if ( KraftSettings::self()->doXmlArchive() ) {
    QFile file( xmlFile );
    if ( file.open( IO_WriteOnly ) ) {
      QTextStream stream( &file );
      stream << xml << "\n";
      file.close();
    } else {
      kdDebug() << "Saving failed" << endl;
    }
  }
  return xmldoc ;
}

dbID ArchiveMan::archiveDocumentDb( KraftDoc *doc )
{
/*
  mysql> describe archdoc;
  +---------------+--------------+------+-----+-------------------+----------------+
  | Field         | Type         | Null | Key | Default           | Extra          |
  +---------------+--------------+------+-----+-------------------+----------------+
  | archDocID     | int(11)      | NO   | PRI | NULL              | auto_increment |
  | ident         | varchar(32)  | YES  | MUL | NULL              |                |
  | docType       | varchar(255) | YES  |     | NULL              |                |
  | clientAddress | text         | YES  |     | NULL              |                |
  | salut         | varchar(255) | YES  |     | NULL              |                |
  | goodbye       | varchar(128) | YES  |     | NULL              |                |
  | printDate     | timestamp    | YES  |     | CURRENT_TIMESTAMP |                |
  | date          | date         | YES  |     | NULL              |                |
  | pretext       | text         | YES  |     | NULL              |                |
  | posttext      | text         | YES  |     | NULL              |                |
  +---------------+--------------+------+-----+-------------------+----------------+
*/
    if( ! doc ) return dbID();
    if( ! KraftDB::getDB() ) return 0;

    QSqlCursor cur("archdoc");
    cur.setMode( QSqlCursor::Writable );
    QSqlRecord *record = 0;

    if( doc->isNew() ) {
	kdDebug() << "Strange: Document in archiving is new!" << endl;
    }

    record = cur.primeInsert();
    record->setValue( "ident", doc->ident() );
    record->setValue( "docType", doc->docType() );
    record->setValue( "clientAddress", doc->address() );
    record->setValue( "salut", doc->salut() );
    record->setValue( "goodbye", doc->goodbye() );
    record->setValue( "printDate", "NOW()" );
    record->setValue( "date", doc->date() );
    record->setValue( "pretext", doc->preText() );
    record->setValue( "posttext", doc->postText() );
    cur.insert();

    dbID id = KraftDB::getLastInsertID();
    archivePos( id.toInt(), doc );

    return id;
}

int ArchiveMan::archivePos( int archDocId, KraftDoc *doc )
{
  /*
    mysql> describe archdocpos;
    +-----------+--------------+------+-----+---------+----------------+
    | Field     | Type         | Null | Key | Default | Extra          |
    +-----------+--------------+------+-----+---------+----------------+
    | archPosID | int(11)      | NO   | PRI | NULL    | auto_increment |
    | archDocID | int(11)      | NO   | MUL |         |                |
    | ordNumber | int(11)      | NO   |     |         |                |
    | text      | text         | YES  |     | NULL    |                |
    | amount    | decimal(6,2) | YES  |     | NULL    |                |
    | unit      | varchar(64)  | YES  |     | NULL    |                |
    | price     | decimal(6,2) | YES  |     | NULL    |                |
    | vat       | decimal(3,1) | YES  |     | 0.0     |                |
    +-----------+--------------+------+-----+---------+----------------+
  */
    if( ! doc ) return -1;

    QSqlCursor cur("archdocpos");
    cur.setMode( QSqlCursor::Writable );

    int cnt = 0;
    DocPositionBase *dpb;
    DocPositionList posList = doc->positions();

    for( dpb = posList.first(); dpb; dpb = posList.next() ) {
      kdDebug() << "Archiving pos for " << archDocId << endl;
	if( dpb->type() == DocPositionBase::Position ) {
	    DocPosition *dp = static_cast<DocPosition*>(dpb);

	    QSqlRecord *record = cur.primeInsert();

	    record->setValue( "archDocID", archDocId );
	    record->setValue( "ordNumber", 1+cnt /* dp->position() */ );
	    record->setValue( "text", dp->text() );
	    record->setValue( "amount", dp->amount() );
	    record->setValue( "unit", dp->unit().einheit( dp->amount() ) );
	    record->setValue( "price", dp->unitPrice().toDouble() );
	    record->setValue( "vat", 16.0 ); // FIXME !!

	    cur.insert();
            kdDebug() << "SQL-Error: " << cur.lastError().text() << endl;
            dbID id = KraftDB::getLastInsertID();
            kdDebug() << "Inserted for id " << id.toString() << endl;
	    cnt++;
	} else {
          kdDebug() << "Unknown position type, can not archive" << endl;
        }
    }
    return cnt;
}
