/***************************************************************************
             templatesaverbase  -
                             -------------------
    begin                : 2005-20-01
    copyright            : (C) 2005 by Klaas Freitag
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

// include files for Qt


#include <qsqlcursor.h>
#include <qsqlrecord.h>
#include <qsqlindex.h>

// include files for KDE
#include <klocale.h>
#include <kdebug.h>

#include "documentsaverdb.h"
#include "kraftdoc.h"
#include "kraftdb.h"
#include "unitmanager.h"
#include "dbids.h"


/* Table document:
 * +----------+--------------+------+-----+---------+----------------+
 * | Field    | Type         | Null | Key | Default | Extra          |
 * +----------+--------------+------+-----+---------+----------------+
 * | docID    | int(11)      |      | PRI | NULL    | auto_increment |
 * | ident    | varchar(32)  | YES  | MUL | NULL    |                |
 * | docType  | varchar(255) | YES  |     | NULL    |                |
 * | clientID | int(11)      | YES  | MUL | NULL    |                |
 * | salut    | varchar(255) | YES  |     | NULL    |                |
 * | date     | date         | YES  |     | NULL    |                |
 * | pretext  | mediumtext   | YES  |     | NULL    |                |
 * | posttext | mediumtext   | YES  |     | NULL    |                |
 * +----------+--------------+------+-----+---------+----------------+
 */

DocumentSaverDB::DocumentSaverDB( ) : DocumentSaverBase()
{

}

bool DocumentSaverDB::saveDocument(KraftDoc *doc )
{
    bool result = false;
    if( ! doc ) return result;

    QSqlCursor cur("document");
    cur.setMode( QSqlCursor::Writable );
    QSqlRecord *record = 0;

    if( doc->isNew() ) {
        record = cur.primeInsert();

    } else {
      cur.select( "docID=" + doc->docID().toString() );
      if ( cur.next() ) {
        record = cur.primeUpdate();
      } else {
        kdError() << "Could not select document record" << endl;
        return result;
      }
       // The document was already saved.
    }

    fillDocumentBuffer( record, doc );

    if( doc->isNew() ) {
      kdDebug() << "Doc is new, inserting" << endl;
      cur.insert();
      dbID id = KraftDB::getLastInsertID();
      doc->setDocID( id );
    } else {
      kdDebug() << "Doc is not new, updating #" << doc->docID().intID() << endl;

      record->setValue( "docID", doc->docID().toString() );
      cur.update();
    }

    saveDocumentPositions( doc );

    kdDebug() << "Saved document no " << doc->docID().toString() << endl;

    return result;
}

void DocumentSaverDB::saveDocumentPositions( KraftDoc *doc )
{
  DocPositionList posList = doc->positions();

  DocPositionBase *dpb = 0;

  // invert all pos numbers to avoid a unique violation
  // FIXME: We need non-numeric ids
  QSqlQuery upq;
  upq.prepare( "UPDATE docposition SET ordNumber = -1 * ordNumber WHERE docID=" +  doc->docID().toString() );
  upq.exec();

  int ordNumber = 1;

  for( dpb = posList.first(); dpb; dpb = posList.next() ) {
     if( dpb->type() == DocPositionBase::Position ) {
       DocPosition *dp = static_cast<DocPosition*>(dpb);
       QSqlRecord *record = 0;
       QSqlCursor cur( "docposition" );
       bool doInsert = true;

       int posDbID = dp->dbId().toInt();
       kdDebug() << "Position DB-Id: " << posDbID << endl;
       if( posDbID > -1 ) {
         const QString selStr = QString("docID=%1 AND positionID=%2").arg( doc->docID().toInt() ).arg( posDbID );
         kdDebug() << "Selecting with " << selStr << endl;
         cur.select( selStr );
         if ( cur.next() ) {
           if( ! dp->toDelete() )
             record = cur.primeUpdate();
           doInsert = false;
         } else {
           kdError() << "ERR: Could not select document position record" << endl;
           return;
         }
       } else {
          // The record is new
         record = cur.primeInsert();
       }
       if( dp->toDelete() ) {
         if( !doInsert ) {
           // the position is already existing, delete it
           cur.primeDelete();
           cur.del();
         }

         continue;
       }

       if( record ) {
         kdDebug() << "Updating position " << dp->position() << " is " << dp->text() << endl;
         record->setValue( "docID",     doc->docID().toInt() );
         record->setValue( "ordNumber", ordNumber );
         record->setValue( "text",      dp->text() );
         record->setValue( "amount",    dp->amount() );
         record->setValue( "unit",      dp->unit().id() );
         record->setValue( "price",     dp->unitPrice().toDouble() );

         ordNumber++; // FIXME

         if( doInsert ) {
           kdDebug() << "Inserting!" << endl;
           cur.insert();
           dp->setDbId( KraftDB::getLastInsertID().toInt() );
         } else {
           kdDebug() << "Updating!" << endl;
           cur.update();

         }
       } else {
         kdDebug() << "ERR: No record object found!" << endl;
       }
       QSqlError err = cur.lastError();
       if( err.type() != QSqlError::None ) {
         kdDebug() << "SQL-ERR: " << err.text() << endl;
       }
    }
  }
#if 0
  // now remove the removed positions
  QSqlCursor cur( "docposition" );
  DBIdList rPos = doc->removePositionList();
  DBIdList::iterator it;
  for ( it = rPos.begin(); it != rPos.end(); ++it ) {
    const QString selector = QString("docID=%1 AND positionID=%2").arg( doc->docID().toInt()).arg((*it).toInt() );
    cur.select( selector );

    if( cur.next() ) {
      cur.primeDelete();
      cur.del();
      kdDebug() << "Removed position " << (*it).toString() << endl;
    }
  }
#endif
}

void DocumentSaverDB::fillDocumentBuffer( QSqlRecord *buf, KraftDoc *doc )
{
    if( buf && doc ) {
      kdDebug() << "Adressstring: " << doc->address() << endl;
      buf->setValue( "ident",    doc->ident() );
      buf->setValue( "docType",  doc->docType() );
      buf->setValue( "clientID", doc->addressUid() );
      buf->setValue( "clientAddress", doc->address() );
      buf->setValue( "salut",    doc->salut() );
      buf->setValue( "goodbye",  doc->goodbye() );
      buf->setValue( "date",     doc->date() );
      buf->setValue( "lastModified", "NOW()" );
      buf->setValue( "pretext",  doc->preText() );
      buf->setValue( "posttext", doc->postText() );
    }
}

void DocumentSaverDB::load( const QString& id, KraftDoc *doc )
{
    QSqlCursor cur("document");
    kdDebug() << "Loading document id " << id << endl;

    cur.select( "docID=" + id );

    if( cur.next())
    {
        kdDebug() << "loading document with id " << id << endl;
        dbID dbid;
        dbid = id;
        doc->setDocID( dbid );

        doc->setIdent(      cur.value( "ident"    ).toString() );
        doc->setDocType(    cur.value( "docType"  ).toString() );
        doc->setAddressUid( cur.value( "clientID" ).toString() );
        doc->setAddress(    cur.value( "clientAddress" ).toString() );
        doc->setSalut(      cur.value( "salut"    ).toString() );
        doc->setGoodbye(    cur.value( "goodbye"  ).toString() );
        doc->setDate(       cur.value( "date"     ).toDate() );
        doc->setPreText(    cur.value( "pretext"  ).toString() );
        doc->setPostText(   cur.value( "posttext" ).toString() );
    }

    loadPositions( id, doc );
}
/* docposition:
  +------------+--------------+------+-----+---------+----------------+
  | Field      | Type         | Null | Key | Default | Extra          |
  +------------+--------------+------+-----+---------+----------------+
  | positionID | int(11)      |      | PRI | NULL    | auto_increment |
  | docID      | int(11)      |      | MUL | 0       |                |
  | ordNumber  | int(11)      |      |     | 0       |                |
  | text       | mediumtext   | YES  |     | NULL    |                |
  | amount     | decimal(6,2) | YES  |     | NULL    |                |
  | unit       | varchar(64)  | YES  |     | NULL    |                |
  | price      | decimal(6,2) | YES  |     | NULL    |                |
  +------------+--------------+------+-----+---------+----------------+
*/
void DocumentSaverDB::loadPositions( const QString& id, KraftDoc *doc )
{
    QSqlCursor cur("docposition");
    QSqlIndex posIndex = cur.index( "ordNumber" );
    cur.select( "docID=" + id, posIndex );

    while( cur.next() ) {
        kdDebug() << "loading document position for document id " << id << endl;
        DocPosition *dp = doc->createPosition();
        dp->setDbId( cur.value("positionID").toInt() );
        dp->setText( cur.value("text").toString() );
        dp->setAmount( cur.value("amount").toDouble() );

        dp->setUnit( UnitManager::getUnit( cur.value("unit").toInt() ) );
        dp->setUnitPrice( cur.value("price").toDouble() );
    }
}

QString DocumentSaverDB::generateNewDocumentId() const
{
  QString re;

  QDateTime d = QDateTime::currentDateTime();
  re = QString::number( d.toTime_t() );

  return re;
}

DocumentSaverDB::~DocumentSaverDB( )
{

}

/* END */


#include "documentsaverdb.moc"
