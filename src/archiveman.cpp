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
#include "documentman.h"
#include "defaultprovider.h"
#include "format.h"
#include "version.h"

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

  if ( DefaultProvider::self()->writeXmlArchive() ) {
      archiveDocumentXml( doc, archId.toString());
  }

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

QDomElement ArchiveMan::positionsDomElement( DocPositionList *positions, QDomDocument& doc )
{
    QDomElement topElem = doc.createElement( "positions" );
    QDomElement posElem;

    int num = 1;

    DocPositionListIterator it(*positions);
    while( it.hasNext() ) {
        DocPosition *dpb = static_cast<DocPosition*>( it.next() );

        if( dpb->type() == DocPositionBase::Position ) {
            DocPosition *dp = static_cast<DocPosition*>(dpb);

            posElem = doc.createElement( "position" );
            posElem.setAttribute( "number", num++ );
            topElem.appendChild( posElem );
            posElem.appendChild( xmlTextElement( doc, "text", dp->text() ) );

            double am = dp->amount();
            QString h = QString::number(am, 'f', 2 );
            posElem.appendChild( xmlTextElement( doc, "amount", h ));

            Einheit e = dp->unit();
            posElem.appendChild( xmlTextElement( doc, "unit", e.einheit( am ) ) );

            Geld g = dp->unitPrice();
            posElem.appendChild( xmlTextElement( doc, "unitprice", QString::number(g.toDouble(), 'f', 2 )));

            Geld sum(g * am);

            posElem.appendChild( xmlTextElement( doc, "sumprice", QString::number(sum.toDouble(), 'f', 2 ) ) );
        }
    }
    return topElem;
}

QDomDocument ArchiveMan::archiveDocumentXml( KraftDoc *doc, const QString& archId )
{
    QDomDocument xmldoc( "kraftdocument" );
    QDomElement root = xmldoc.createElement( "kraftdocument" );
    root.setAttribute(QStringLiteral("kraft_version"), Kraft::Version::number());

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

    docElem.appendChild( xmlTextElement( xmldoc, "date", Format::toDateString(doc->date(), Format::DateFormatIso)));

    DocPositionList dpList = doc->positions();
    root.appendChild( positionsDomElement(&dpList, xmldoc) );

    QString xml = xmldoc.toString();
    // qDebug() << "Resulting XML: " << xml;

    const QString outputDir = xmlBaseDir();
    const QString filename = archiveFileName( doc->ident(), archId, "xml" );

    const QString xmlFile = QString( "%1/%2" ).arg( outputDir ).arg( filename );

    // qDebug () << "Storing XML to " << xmlFile;

    QFile file( xmlFile );
    if ( file.open( QIODevice::WriteOnly ) ) {
        QTextStream stream( &file );
        stream << xml << "\n";
        file.close();
    } else {
        // qDebug () << "Saving failed";
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

    if( doc->state().isNew() ) {
      // qDebug () << "Strange: Document in archiving is new!";
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
    record.setValue( "country",  loc->name() );
    record.setValue( "language", QLocale::languageToString(DefaultProvider::self()->locale()->language()));
    record.setValue( "tax", UnitManager::self()->tax( doc->date() ) );
    record.setValue( "reducedTax", UnitManager::self()->reducedTax( doc->date() ) );
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

    // qDebug () << "Archiving pos for " << archDocId;
    while ( it.hasNext() ) {
      DocPosition *dp = static_cast<DocPosition*>( it.next() );

      record.setValue( "archDocID", archDocId );
      record.setValue( "ordNumber", 1+cnt /* dp->position() */ );
      record.setValue( "kind", dp->typeStr());
      record.setValue( "text", dp->text() ); // expandItemText( dp ) );
      record.setValue( "amount", dp->amount() );
      record.setValue( "unit", dp->unit().einheit( dp->amount() ) );
      record.setValue( "price", dp->unitPrice().toDouble() );
      record.setValue( "overallPrice", dp->overallPrice().toDouble() );
      record.setValue( "taxType", dp->taxTypeNumeric() );

      if(!model.insertRecord(-1, record)) {
        // qDebug () << model.lastError();
	  }
      cnt++;

      // save the attributes of the positions in the attributes
      // table but with a new host type which reflects the arch state

      // AttributeMap attribs = dp->attributes();
      // attribs.setHost( "ArchPosition" );
      // attribs.save( id );
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
    QString outputDir = DefaultProvider::self()->xmlArchivePath();
    if ( outputDir.isEmpty() ) {
        // stay bug compatible: Before issue #80, this was the pdfOutputDir
        outputDir = DefaultProvider::self()->pdfOutputDir();
    }
    if (outputDir.isEmpty()) {
        outputDir = QStandardPaths::writableLocation( QStandardPaths::AppDataLocation );
    }

    if ( ! outputDir.endsWith('/') ) outputDir += QChar('/');
    ensureDirIsExisting(outputDir);

    return outputDir;
}

QString ArchiveMan::pdfBaseDir() const
{
  QString outputDir = DefaultProvider::self()->pdfOutputDir();
  if ( outputDir.isEmpty() ) {
    outputDir = QStandardPaths::writableLocation( QStandardPaths::AppDataLocation );
  }

  if ( ! outputDir.endsWith('/') ) outputDir += QChar('/');
  ensureDirIsExisting(outputDir);

  return outputDir;

}

QString ArchiveMan::archiveFileName( const QString& docId, const QString& archId, const QString& ext ) const
{
    QString re = QString( "%1_%2.%3" ).arg( docId ).arg( archId ).arg( ext );
    re.replace(QLatin1Char('/'), QLatin1Char('_'));
    return re;
}
