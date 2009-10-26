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
#include <q3sqlcursor.h>
#include <qsqlrecord.h>
#include <qsqlindex.h>
#include <qfile.h>
#include <q3textstream.h>
#include <qglobal.h>

#include <k3staticdeleter.h>
#include <kstandarddirs.h>

#include <kdebug.h>

#include "archiveman.h"
#include "kraftdoc.h"
#include "kraftdb.h"
#include "unitmanager.h"
#include "dbids.h"
#include "kraftsettings.h"
#include "documentman.h"

static K3StaticDeleter<ArchiveMan> selfDeleter;


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

  dbID archId = archiveDocumentDb( doc );

  archiveDocumentXml( doc, archId.toString() );

  return archId;
}

QString ArchiveMan::documentID( dbID archID ) const
{
  QString re;

  Q3SqlCursor cur("archdoc");
  cur.setMode( Q3SqlCursor::ReadOnly );
  cur.select( QString( "archDocID=%1" ).arg( archID.toInt() ) );

  if ( cur.next() ) {
    re = cur.value( "ident" ).toString();
  }

  return re;
}

QDomElement ArchiveMan::xmlTextElement( QDomDocument doc, const QString& name, const QString& value )
{
  QDomElement elem = doc.createElement( name );
  QDomText t = doc.createTextNode( value );
  elem.appendChild( t );
  return elem;
}

QDomDocument ArchiveMan::archiveDocumentXml( KraftDoc *doc, const QString& archId )
{
  QDomDocument xmldoc( "kraftdocument" );
  QDomElement root = xmldoc.createElement( "kraftdocument" );
  // Fixme:
  xmldoc.appendChild( root );
  QDomElement cust = xmldoc.createElement( "client" );
  root.appendChild( cust );
  cust.appendChild( xmlTextElement( xmldoc, "address", doc->address() ) );
  cust.appendChild( xmlTextElement( xmldoc, "clientId", doc->addressUid() ) );

  QDomElement docElem = xmldoc.createElement( "docframe" );
  root.appendChild( docElem );
  docElem.appendChild( xmlTextElement( xmldoc, "docType", doc->docType() ) );
  docElem.appendChild( xmlTextElement( xmldoc, "docDesc", doc->whiteboard() ) );
  docElem.appendChild( xmlTextElement( xmldoc, "ident", doc->ident() ) );
  docElem.appendChild( xmlTextElement( xmldoc, "preText", doc->preText() ) );
  docElem.appendChild( xmlTextElement( xmldoc, "postText", doc->postText() ) );
  docElem.appendChild( xmlTextElement( xmldoc, "projectLabel", doc->projectLabel() ) );
  docElem.appendChild( xmlTextElement( xmldoc, "salut", doc->salut() ) );
  docElem.appendChild( xmlTextElement( xmldoc, "goodbye", doc->goodbye() ) );

  docElem.appendChild( xmlTextElement( xmldoc, "date",
                                       doc->locale()->formatDate( doc->date() ) ) );

  root.appendChild( doc->positions().domElement( xmldoc ) );

  QString xml = xmldoc.toString();
  // kDebug() << "Resulting XML: " << xml << endl;

  QString outputDir = ArchiveMan::self()->xmlBaseDir();
  QString filename = ArchiveMan::self()->archiveFileName( doc->ident(), archId, "xml" );

  QString xmlFile = QString( "%1/%2" ).arg( outputDir ).arg( filename );

  kDebug() << "Storing XML to " << xmlFile << endl;

  if ( KraftSettings::self()->self()->doXmlArchive() ) {
    QFile file( xmlFile );
    if ( file.open( QIODevice::WriteOnly ) ) {
      Q3TextStream stream( &file );
      stream << xml << "\n";
      file.close();
    } else {
      kDebug() << "Saving failed" << endl;
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
   | clientUid     | varchar(32)  | YES  |     | NULL              |                |
   | salut         | varchar(255) | YES  |     | NULL              |                |
   | goodbye       | varchar(128) | YES  |     | NULL              |                |
   | printDate     | timestamp    | NO   |     | CURRENT_TIMESTAMP |                |
   | date          | date         | YES  |     | NULL              |                |
   | pretext       | text         | YES  |     | NULL              |                |
   | posttext      | text         | YES  |     | NULL              |                |
   | country       | varchar(32)  | YES  |     | NULL              |                |
   | language      | varchar(32)  | YES  |     | NULL              |                |
   | projectLabel  | varchar(255) | YES  |     | NULL              |                |
   | state         | int(11)      | YES  |     | NULL              |                |
   +---------------+--------------+------+-----+-------------------+----------------+
*/
    if( ! doc ) return dbID();

    Q3SqlCursor cur("archdoc");
    cur.setMode( Q3SqlCursor::Writable );
    QSqlRecord *record = 0;

    if( doc->isNew() ) {
	kDebug() << "Strange: Document in archiving is new!" << endl;
    }

    record = cur.primeInsert();
    record->setValue( "ident", doc->ident() );
    record->setValue( "docType", doc->docType() );
    record->setValue( "docDescription", KraftDB::self()->mysqlEuroEncode( doc->whiteboard() ) );
    record->setValue( "clientAddress", doc->address() );
    record->setValue( "clientUid", doc->addressUid() );
    record->setValue( "salut", doc->salut() );
    record->setValue( "goodbye", doc->goodbye() );
    record->setValue( "printDate", "NOW()" );
    record->setValue( "date", doc->date() );
    record->setValue( "pretext",  KraftDB::self()->mysqlEuroEncode(doc->preText() ) );
    record->setValue( "posttext", KraftDB::self()->mysqlEuroEncode(doc->postText() ) );
    record->setValue( "projectLabel", KraftDB::self()->mysqlEuroEncode(doc->projectLabel() ) );
    record->setValue( "country",  doc->country() );
    record->setValue( "language", doc->language() );
    record->setValue( "tax", DocumentMan::self()->tax( doc->date() ) );
    record->setValue( "reducedTax", DocumentMan::self()->reducedTax( doc->date() ) );
    cur.insert();

    dbID id = KraftDB::self()->getLastInsertID();
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

    Q3SqlCursor cur("archdocpos");
    cur.setMode( Q3SqlCursor::Writable );

    int cnt = 0;

    DocPositionList posList = doc->positions();
    DocPositionListIterator it( posList );

    kDebug() << "Archiving pos for " << archDocId << endl;
    while ( it.hasNext() ) {
      DocPosition *dp = static_cast<DocPosition*>( it.next() );

      QSqlRecord *record = cur.primeInsert();

      record->setValue( "archDocID", archDocId );
      record->setValue( "ordNumber", 1+cnt /* dp->position() */ );
      record->setValue( "kind", dp->attribute( DocPosition::Kind ) );

      record->setValue( "text", dp->text() ); // expandItemText( dp ) );
      record->setValue( "amount", dp->amount() );
      record->setValue( "unit", dp->unit().einheit( dp->amount() ) );
      record->setValue( "price", dp->unitPrice().toDouble() );
      record->setValue( "overallPrice", dp->overallPrice().toDouble() );
      record->setValue( "taxType", dp->taxTypeNumeric() );

      cur.insert();
      dbID id = KraftDB::self()->getLastInsertID();
      // kDebug() << "Inserted for id " << id.toString() << endl;
      cnt++;

      // save the attributes of the positions in the attributes
      // table but with a new host type which reflects the arch state
      AttributeMap attribs = dp->attributes();
      attribs.setHost( "ArchPosition" );
      attribs.save( id );
    }
    return cnt;
}

QString ArchiveMan::xmlBaseDir() const
{
  KStandardDirs stdDirs;
  QString outputDir = KraftSettings::self()->self()->pdfOutputDir();
  if ( outputDir.isEmpty() ) {
    outputDir = stdDirs.saveLocation( "data", "kraft/archiveXml", true );
  }

  if ( ! outputDir.endsWith( "/" ) ) outputDir += "/";

  return outputDir;
}

QString ArchiveMan::pdfBaseDir() const
{
  KStandardDirs stdDirs;
  QString outputDir = KraftSettings::self()->self()->pdfOutputDir();
  if ( outputDir.isEmpty() ) {
    outputDir = stdDirs.saveLocation( "data", "kraft/archivePdf", true );
  }

  if ( ! outputDir.endsWith( "/" ) ) outputDir += "/";

  return outputDir;

}

QString ArchiveMan::archiveFileName( const QString& docId, const QString& archId, const QString& ext ) const
{
  return QString( "%1_%2.%3" ).arg( docId ).arg( archId ).arg( ext );
}
