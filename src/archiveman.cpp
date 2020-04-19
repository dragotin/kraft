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

#include <QSqlTableModel>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlIndex>
#include <QFile>
#include <QTextStream>
#include <QDebug>

#include "archiveman.h"
#include "kraftdoc.h"
#include "kraftdb.h"
#include "unitmanager.h"
#include "dbids.h"
#include "kraftsettings.h"
#include "documentman.h"
#include "defaultprovider.h"

Q_GLOBAL_STATIC(ArchiveMan, mSelf)

ArchiveMan *ArchiveMan::self()
{
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

  QSqlQuery q;
  q.prepare("SELECT ident FROM archdoc WHERE archDocID=:id");
  q.bindValue(":id", archID.toInt());
  q.exec();

  if ( q.next() ) {
    re = q.value( 0 ).toString();
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
  docElem.appendChild( xmlTextElement( xmldoc, "predecessor", doc->predecessor() ) );
  docElem.appendChild( xmlTextElement( xmldoc, "preText", doc->preText() ) );
  docElem.appendChild( xmlTextElement( xmldoc, "postText", doc->postText() ) );
  docElem.appendChild( xmlTextElement( xmldoc, "projectLabel", doc->projectLabel() ) );
  docElem.appendChild( xmlTextElement( xmldoc, "salut", doc->salut() ) );
  docElem.appendChild( xmlTextElement( xmldoc, "goodbye", doc->goodbye() ) );

  docElem.appendChild( xmlTextElement( xmldoc, "date",
                                       DefaultProvider::self()->locale()->toString( doc->date() ) ));

  root.appendChild( doc->positions().domElement( xmldoc ) );

  QString xml = xmldoc.toString();
  // qDebug() << "Resulting XML: " << xml << endl;

  QString outputDir = ArchiveMan::self()->xmlBaseDir();
  QString filename = ArchiveMan::self()->archiveFileName( doc->ident(), archId, "xml" );

  QString xmlFile = QString( "%1/%2" ).arg( outputDir ).arg( filename );

  // qDebug () << "Storing XML to " << xmlFile << endl;

  if ( KraftSettings::self()->doXmlArchive() ) {
    QFile file( xmlFile );
    if ( file.open( QIODevice::WriteOnly ) ) {
      QTextStream stream( &file );
      stream << xml << "\n";
      file.close();
    } else {
      // qDebug () << "Saving failed" << endl;
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

    QSqlTableModel model;
    model.setTable("archdoc");
    QSqlRecord record = model.record();

    if( doc->isNew() ) {
      // qDebug () << "Strange: Document in archiving is new!" << endl;
    }
    record.setValue( "ident", doc->ident() );
    record.setValue( "docType", doc->docType() );
    record.setValue( "docDescription", KraftDB::self()->mysqlEuroEncode( doc->whiteboard() ) );
    record.setValue( "clientAddress", doc->address() );
    record.setValue( "clientUid", doc->addressUid() );
    record.setValue( "salut", doc->salut() );
    record.setValue( "goodbye", doc->goodbye() );
    record.setValue( "printDate", KraftDB::self()->currentTimeStamp() );
    record.setValue( "date", doc->date() );
    record.setValue( "pretext",  KraftDB::self()->mysqlEuroEncode(doc->preText() ) );
    record.setValue( "posttext", KraftDB::self()->mysqlEuroEncode(doc->postText() ) );
    record.setValue( "projectLabel", KraftDB::self()->mysqlEuroEncode(doc->projectLabel() ) );
    record.setValue( "predecessor", doc->predecessor() );
    QLocale *loc = DefaultProvider::self()->locale();
    record.setValue( "country",  loc->bcp47Name() );
    record.setValue( "language", "" );
    record.setValue( "tax", DocumentMan::self()->tax( doc->date() ) );
    record.setValue( "reducedTax", DocumentMan::self()->reducedTax( doc->date() ) );
    if(!model.insertRecord(-1, record)) {
      // qDebug () << model.lastError();
	}
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

    QSqlTableModel model;
    model.setTable("archdocpos");
    QSqlRecord record = model.record();

    int cnt = 0;

    DocPositionList posList = doc->positions();
    DocPositionListIterator it( posList );

    // qDebug () << "Archiving pos for " << archDocId << endl;
    while ( it.hasNext() ) {
      DocPosition *dp = static_cast<DocPosition*>( it.next() );

      record.setValue( "archDocID", archDocId );
      record.setValue( "ordNumber", 1+cnt /* dp->position() */ );
      record.setValue( "kind", dp->attribute( DocPosition::Kind ) );
      record.setValue( "text", dp->text() ); // expandItemText( dp ) );
      record.setValue( "amount", dp->amount() );
      record.setValue( "unit", dp->unit().einheit( dp->amount() ) );
      record.setValue( "price", dp->unitPrice().toDouble() );
      record.setValue( "overallPrice", dp->overallPrice().toDouble() );
      record.setValue( "taxType", dp->taxTypeNumeric() );

      if(!model.insertRecord(-1, record)) {
        // qDebug () << model.lastError();
	  }
      dbID id = KraftDB::self()->getLastInsertID();
      // qDebug() << "Inserted for id " << id.toString() << endl;
      cnt++;

      // save the attributes of the positions in the attributes
      // table but with a new host type which reflects the arch state
      AttributeMap attribs = dp->attributes();
      attribs.setHost( "ArchPosition" );
      attribs.save( id );
    }
    return cnt;
}

void ArchiveMan::ensureDirIsExisting( const QString& dir ) const
{
    if( ! QFile::exists(dir)) {
        qDebug() << "pdfBaseDir does not exist! Trying to create" << dir;
        QDir d;
        if( d.mkpath(dir) ) {
            qDebug() << "Successfully created" << dir;
        } else {
            qDebug() << "Failed to create" << dir;
        }
    }
}

QString ArchiveMan::xmlBaseDir() const
{
  QString outputDir = KraftSettings::self()->pdfOutputDir();
  if ( outputDir.isEmpty() ) {
    outputDir = QStandardPaths::writableLocation( QStandardPaths::AppDataLocation );
  }

  if ( ! outputDir.endsWith( "/" ) ) outputDir += "/";
  ensureDirIsExisting(outputDir);

  return outputDir;
}

QString ArchiveMan::pdfBaseDir() const
{
  QString outputDir = KraftSettings::self()->pdfOutputDir();
  if ( outputDir.isEmpty() ) {
    outputDir = QStandardPaths::writableLocation( QStandardPaths::AppDataLocation );
  }

  if ( ! outputDir.endsWith( "/" ) ) outputDir += "/";
  ensureDirIsExisting(outputDir);

  return outputDir;

}

QString ArchiveMan::archiveFileName( const QString& docId, const QString& archId, const QString& ext ) const
{
    QString re = QString( "%1_%2.%3" ).arg( docId ).arg( archId ).arg( ext );
    re.replace(QLatin1Char('/'), QLatin1Char('_'));
    return re;
}
