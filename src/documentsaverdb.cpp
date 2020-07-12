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
#include <QSqlRecord>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QSqlError>

// include files for KDE
#include <QDebug>

#include "documentsaverdb.h"
#include "docposition.h"
#include "kraftdoc.h"
#include "kraftdb.h"
#include "unitmanager.h"
#include "dbids.h"
#include "kraftsettings.h"
#include "doctype.h"
#include "defaultprovider.h"

/* Table document:
 * +----------------+--------------+------+-----+-------------------+----------------+
 * | Field          | Type         | Null | Key | Default           | Extra          |
 * +----------------+--------------+------+-----+-------------------+----------------+
 * | docID          | int(11)      | NO   | PRI | NULL              | auto_increment |
 * | ident          | varchar(32)  | YES  | MUL | NULL              |                |
 * | docType        | varchar(255) | YES  |     | NULL              |                |
 * | docDescription | text         | YES  |     | NULL              |                |
 * | clientID       | varchar(32)  | YES  | MUL | NULL              |                |
 * | clientAddress  | text         | YES  |     | NULL              |                |
 * | salut          | varchar(255) | YES  |     | NULL              |                |
 * | goodbye        | varchar(128) | YES  |     | NULL              |                |
 * | lastModified   | timestamp    | NO   |     | CURRENT_TIMESTAMP |                |
 * | date           | date         | YES  |     | NULL              |                |
 * | pretext        | text         | YES  |     | NULL              |                |
 * | posttext       | text         | YES  |     | NULL              |                |
 * | country        | varchar(32)  | YES  |     | NULL              |                |
 * | language       | varchar(32)  | YES  |     | NULL              |                |
 * | projectLabel   | varchar(255) | YES  |     | NULL              |                |
 * | predecessor    | varchar(32)  | YES  |     | NULL              |                |
 * +----------------+--------------+------+-----+-------------------+----------------+
 * 14 rows in set (0.00 sec)
 *
 */

namespace {
void checkAndSet(bool& changes, QSqlRecord& record, const QString& name, const QVariant& setValue)
{
    Q_ASSERT(record.contains(name)); // the record must have the column

    if( record.value(name) != setValue) {
        record.setValue(name, setValue);
        changes = true;
    }
}


bool fillDocumentBuffer(QSqlRecord &buf, KraftDoc *doc)
{
    bool changes {false};
    if( doc ) {
        checkAndSet(changes, buf, "ident", doc->ident());
        checkAndSet(changes, buf, "docType", doc->docType());
        checkAndSet(changes, buf, "docDescription", KraftDB::self()->mysqlEuroEncode(doc->whiteboard()));
        checkAndSet(changes, buf, "clientID", doc->addressUid());
        checkAndSet(changes, buf, "clientAddress", doc->address());
        checkAndSet(changes, buf, "salut", doc->salut());
        checkAndSet(changes, buf, "goodbye", doc->goodbye());
        checkAndSet(changes, buf, "date", doc->date());
        checkAndSet(changes, buf, "pretext", KraftDB::self()->mysqlEuroEncode( doc->preText()));
        checkAndSet(changes, buf, "posttext", KraftDB::self()->mysqlEuroEncode( doc->postText()));
        checkAndSet(changes, buf, "country", DefaultProvider::self()->locale()->bcp47Name());
        checkAndSet(changes, buf, "language", "");
        checkAndSet(changes, buf, "projectLabel", doc->projectLabel());
        checkAndSet(changes, buf, "predecessor", doc->predecessor());

    }
    return changes;
}
}

DocumentSaverDB::DocumentSaverDB( ) : DocumentSaverBase(),
                                      PosTypePosition( QString::fromLatin1( "Position" ) ),
                                      PosTypeExtraDiscount( QString::fromLatin1( "ExtraDiscount" ) ),
                                      PosTypeHeader( QString::fromLatin1( "Header" ) )
{

}

bool DocumentSaverDB::saveDocument(KraftDoc *doc )
{
    bool result = false;
    if( ! doc ) return result;

    QSqlTableModel model;
    model.setTable("document");

    QSqlRecord record;

    // qDebug () << "############### Document Save ################" << endl;

    if( doc->isNew() ) {
        record = model.record();
    } else {
      model.setFilter("docID=" + doc->docID().toString());
      model.select();
      if ( model.rowCount() > 0 ) {
        record = model.record(0);
      } else {
        qCritical() << "Could not select document record" << endl;
        return result;
      }
       // The document was already saved.
    }

    if( !doc->isNew() && doc->docTypeChanged() ) {
        // an existing doc has a new document type. Fix the doc number cycle and pick a new ident
        DocType dt( doc->docType() );
        QString ident = dt.generateDocumentIdent( doc->date(), doc->docType(),
                                                  doc->addressUid() );
        doc->setIdent( ident );
    }

    bool hasChanges = fillDocumentBuffer( record, doc );

    if( doc->isNew() ) {
        // qDebug () << "Doc is new, inserting" << endl;
        if( !model.insertRecord(-1, record)) {
            QSqlError err = model.lastError();
            // qDebug () << "################# SQL Error: " << err.text();
        }
        model.submitAll();

        dbID id = KraftDB::self()->getLastInsertID();
        doc->setDocID( id );

        // get the uniq id and write it into the db
        DocType dt( doc->docType() );
        QString ident = dt.generateDocumentIdent( doc->date(), doc->docType(), doc->addressUid() );
        doc->setIdent( ident );
        model.setFilter("docID=" + id.toString());
        model.select();
        if ( model.rowCount() > 0 ) {
            model.setData(model.index(0, 1), ident);
            model.submitAll();
        }

    } else {
        // qDebug () << "Doc is not new, updating #" << doc->docID().intID() << endl;
        checkAndSet(hasChanges, record, "docID", doc->docID().toString());
        if (!hasChanges) {
            // if there haven't been changes in the document record, we update the changes
            // timestamp manually, otherwise it is not updated at all.
            const QString dt = QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss");
            checkAndSet(hasChanges, record, "lastModified", QVariant(dt));
        }
    }
    model.setRecord(0, record);
    model.submitAll();

    saveDocumentPositions( doc );

    // qDebug () << "Saved document no " << doc->docID().toString() << endl;

    return result;
}

void DocumentSaverDB::saveDocumentPositions( KraftDoc *doc )
{
    DocPositionList posList = doc->positions();

    // invert all pos numbers to avoid a unique violation
    // FIXME: We need non-numeric ids
    QSqlQuery upq;
    QString queryStr = "UPDATE docposition SET ordNumber = -1 * ordNumber WHERE docID=";
    queryStr +=  doc->docID().toString();
    queryStr += " AND ordNumber > 0";
    upq.prepare( queryStr );
    upq.exec();

    int ordNumber = 1;

    QSqlTableModel model;
    model.setTable("docposition");
    model.setEditStrategy(QSqlTableModel::OnManualSubmit);

    QVector<int> deleteIds;

    DocPositionListIterator it( posList );
    while( it.hasNext() ) {
        DocPositionBase *dpb = it.next();

        DocPosition *dp = static_cast<DocPosition*>(dpb);
        QSqlRecord record ;
        bool doInsert = true;

        int posDbID = dp->dbId().toInt();
        if( posDbID > -1 ) {
            const QString selStr = QString("docID=%1 AND positionID=%2").arg( doc->docID().toInt() ).arg( posDbID );
            // qDebug() << "Selecting with " << selStr << endl;
            model.setFilter( selStr );
            model.select();
            if ( model.rowCount() > 0 ) {
                if( ! dp->toDelete() )
                    record = model.record(0);
                doInsert = false;
            } else {
                qCritical() << "ERR: Could not select document position record" << endl;
                return;
            }
        } else {
            // The record is new
            record = model.record();
        }

        if( dp->toDelete() ) {
            // qDebug () << "This one is to delete, do it!" << endl;

            if( doInsert ) {
                qWarning() << "Attempt to delete a toInsert-Item, obscure" << endl;
            }
            // delete all existing attributes
            dp->attributes().dbDeleteAll( dp->dbId() );

            model.removeRow(0);
            model.submitAll();

            continue;
        }

        if( record.count() > 0 ) {
            // qDebug() << "Updating position " << dp->position() << " is " << dp->text() << endl;
            QString typeStr = PosTypePosition;
            double price = dp->unitPrice().toDouble();

            if ( dp->type() == DocPositionBase::ExtraDiscount ) {
                typeStr = PosTypeExtraDiscount;
            }

            record.setValue( "docID",     QVariant(doc->docID().toInt()));
            record.setValue( "ordNumber", QVariant(ordNumber));
            record.setValue( "text",      QVariant(dp->text()));
            record.setValue( "postype",   QVariant(typeStr));
            record.setValue( "amount",    QVariant(dp->amount()));
            int unitId = dp->unit().id();
            record.setValue( "unit",      QVariant(unitId));
            record.setValue( "price",     QVariant(price));
            record.setValue( "taxType",   QVariant(dp->taxType()));

            ordNumber++; // FIXME

            if( doInsert ) {
                // qDebug () << "Inserting!" << endl;
                model.insertRecord(-1, record);
                model.submitAll();
                dp->setDbId( KraftDB::self()->getLastInsertID().toInt() );
            } else {
                // qDebug () << "Updating!" << endl;
                model.setRecord(0, record);
                model.submitAll();
            }
        } else {
            // qDebug () << "ERR: No record object found!" << endl;
        }

        dp->attributes().save( dp->dbId() );

        QSqlError err = model.lastError();
        if( err.type() != QSqlError::NoError ) {
            // qDebug () << "SQL-ERR: " << err.text() << " in " << model.tableName() << endl;
        }

    }
    model.submitAll();

    /*  remove the docpositions that were marked to be deleted */
    if( deleteIds.count() ) {
        QSqlQuery delQuery;
        delQuery.prepare( "DELETE FROM docposition WHERE positionID=:id" );
        foreach( int id, deleteIds ) {
            // kDebug() << "Deleting attribute id " << id;
            delQuery.bindValue( ":id", id );
            delQuery.exec();
        }
    }


}

void DocumentSaverDB::load( const QString& id, KraftDoc *doc )
{
    if( !id.isEmpty() ) {
        QSqlQuery q;
        q.prepare("SELECT ident, docType, clientID, clientAddress, salut, goodbye, date, lastModified, language, country, "
                  "pretext, posttext, docDescription, projectlabel, predecessor FROM document WHERE docID=:docID");
        q.bindValue(":docID", id);
        q.exec();
        // qDebug () << "Loading document id " << id << endl;

        if( q.next())
        {
            // qDebug () << "loading document with id " << id << endl;
            dbID dbid;
            dbid = id;
            doc->setDocID(dbid);

            doc->setIdent(      q.value( 0 ).toString() );
            doc->setDocType(    q.value( 1 ).toString() );
            doc->setAddressUid( q.value( 2 ).toString() );
            doc->setAddress(    q.value( 3 ).toString() );
            QString salut = q.value(4).toString();
            doc->setSalut(      salut );
            doc->setGoodbye(    q.value( 5 ).toString() );
            doc->setDate (      q.value( 6 ).toDate() );
            QDateTime dt = q.value(7).toDateTime();

            // Sqlite stores the timestamp as UTC in the database. Mysql does not.
            if (KraftDB::self()->isSqlite()) {
                dt.setTimeSpec(Qt::UTC);
                doc->setLastModified(dt.toLocalTime());
            } else {
                doc->setLastModified(dt);
            }

            // Removed, as with Kraft 0.80 there is no locale management on doc level any more
            // doc->setCountryLanguage( q.value( 8 ).toString(),
            //                         q.value( 9 ).toString());

            doc->setPreText(    KraftDB::self()->mysqlEuroDecode( q.value( 10  ).toString() ) );
            doc->setPostText(   KraftDB::self()->mysqlEuroDecode( q.value( 11 ).toString() ) );
            doc->setWhiteboard( KraftDB::self()->mysqlEuroDecode( q.value( 12 ).toString() ) );
            doc->setProjectLabel( q.value(13).toString() );
            doc->setPredecessor(  q.value(14).toString() );
        }
    }
    // load the dbID of the predecessor document from the database.
    const QString pIdent = doc->predecessor();
    if( ! pIdent.isEmpty() ) {
        QSqlQuery q1;
        q1.prepare("SELECT docID FROM document WHERE ident=:docID");
        q1.bindValue(":docID", pIdent);
        q1.exec();
        if( q1.next() ) {
            const QString pDbId = q1.value(0).toString();
            doc->setPredecessorDbId(pDbId);
        }
    }

    // finally load the item data.
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
    QSqlQuery q;
    q.prepare("SELECT positionID, postype, text, amount, unit, price, taxType FROM docposition WHERE docID=:docID ORDER BY ordNumber");
    q.bindValue(":docID", id);
    q.exec();

    // qDebug () << "* loading document positions for document id " << id << endl;
    while( q.next() ) {
        // qDebug () << " loading position id " << q.value( 0 ).toInt() << endl;

        DocPositionBase::PositionType type = DocPositionBase::Position;
        QString typeStr = q.value( 1 ).toString();
        if ( typeStr == PosTypeExtraDiscount ) {
          type = DocPositionBase::ExtraDiscount;
        } else if ( typeStr == PosTypePosition ) {
          // nice, default position type.
          type = DocPositionBase::Position;
        } else {
          // qDebug () << "ERROR: Strange type string loaded from db: " << typeStr << endl;
        }

        DocPosition *dp = doc->createPosition( type );
        dp->setDbId( q.value(0).toInt() );
        dp->setText( q.value(2).toString() );

        // Note: empty fields are treated as Positions which is intended because
        // the type col was added later and thus might be empty for older entries

        dp->setAmount( q.value(3).toDouble() );

        dp->setUnit( UnitManager::self()->getUnit( q.value(4).toInt() ) );
        dp->setUnitPrice( q.value(5).toDouble() );
        dp->setTaxType( q.value(6).toInt() );

        dp->loadAttributes();
    }
}

DocumentSaverDB::~DocumentSaverDB( )
{

}

/* END */

