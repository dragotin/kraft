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
#include "doctype.h"
#include "defaultprovider.h"
#include "docdigest.h"
#include "models/docbasemodel.h"


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
        checkAndSet(changes, buf, "pretext", KraftDB::self()->mysqlEuroEncode( doc->preTextRaw()));
        checkAndSet(changes, buf, "posttext", KraftDB::self()->mysqlEuroEncode( doc->postTextRaw()));

        // The locale can be reconstructed from the name of the locale.
        checkAndSet(changes, buf, "country", DefaultProvider::self()->locale()->name());
        // ...while the language setting is not really needed, but for beauty written to db.
        checkAndSet(changes, buf, "language", QLocale::languageToString(DefaultProvider::self()->locale()->language()));
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
    if( ! doc ) return false;

    QSqlTableModel model;
    model.setTable("document");

    QSqlRecord record;

    // qDebug () << "############### Document Save ################";

    if( doc->isNew() ) {
        record = model.record();
    } else {
      model.setFilter("docID=" + doc->docID().toString());
      model.select();
      if ( model.rowCount() > 0 ) {
        record = model.record(0);
      } else {
        qCritical() << "Could not select document record";
        return false;
      }
       // The document was already saved.
    }
    QString ident;
    if( doc->isNew() || doc->docTypeChanged() ) {
        // an existing doc has a new document type. Fix the doc number cycle and pick a new ident
        DocType dt( doc->docType() );
        int id = dt.nextIdentId(true);
        int dayCnt = dt.nextDayCounter(QDate::currentDate());
        ident = dt.generateDocumentIdent(doc->date(), doc->addressUid(), id, dayCnt);
        doc->setIdent( ident );
    }

    bool hasChanges = fillDocumentBuffer( record, doc );

    if( doc->isNew() ) {
        // qDebug () << "Doc is new, inserting";
        if( !model.insertRecord(-1, record)) {
            QSqlError err = model.lastError();
            // qDebug () << "################# SQL Error: " << err.text();
        }
        model.submitAll();

        dbID id = KraftDB::self()->getLastInsertID();
        doc->setDocID( id );
        model.setFilter("docID=" + id.toString());
        model.select();
        if ( model.rowCount() > 0 ) {
            model.setData(model.index(0, 1), ident);
            model.submitAll();
        }

    } else {
        // qDebug () << "Doc is not new, updating #" << doc->docID().intID();
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

    // qDebug () << "Saved document no " << doc->docID().toString();

    return true;
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
        int posDbID = dp->dbId().toInt();

        if( dp->toDelete() ) {
            qDebug () << "Delete doc item id" << posDbID;
            // delete all existing attributes
            // dp->attributes().dbDeleteAll( dp->dbId() );
            deleteIds.append(posDbID);
            continue;
        }

        QSqlRecord record ;
        bool doInsert = true;

        if( posDbID > -1 ) {
            const QString selStr = QString("docID=%1 AND positionID=%2").arg( doc->docID().toInt() ).arg( posDbID );
            // qDebug() << "Selecting with " << selStr;
            model.setFilter(selStr);
            model.select();
            if ( model.rowCount() > 0 ) {
                record = model.record(0);
                doInsert = false;
            } else {
                qCritical() << "ERR: Could not select document position record";
                return;
            }
        } else {
            // The record is new
            record = model.record();
        }

        // qDebug() << "Updating position " << dp->position() << " is " << dp->text();
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
            // qDebug () << "Inserting!";
            model.insertRecord(-1, record);
            model.submitAll();
            dp->setDbId( KraftDB::self()->getLastInsertID().toInt() );
        } else {
            // qDebug () << "Updating!";
            model.setRecord(0, record);
            model.submitAll();
        }

        QSqlError err = model.lastError();
        if( err.type() != QSqlError::NoError ) {
            qDebug () << "SQL-ERR: " << err.text() << " in " << model.tableName();
        }

        // dp->attributes().save( dp->dbId() );
    }

    model.submitAll();

    /*  remove the docpositions that were marked to be deleted */
    if( deleteIds.count() ) {
        QSqlQuery delQuery;
        delQuery.prepare( "DELETE FROM docposition WHERE positionID=:id and docID=:docId" );
        int docId = doc->docID().toInt();

        for( int id : deleteIds ) {
            // kDebug() << "Deleting attribute id " << id;
            delQuery.bindValue(":id", id );
            delQuery.bindValue(":docId", docId);
            delQuery.exec();
        }
    }
}

bool DocumentSaverDB::loadByIdent( const QString& ident, KraftDoc *doc )
{
    int id{-1}; // the database id

    if( !ident.isEmpty() ) {
        QSqlQuery q;
        q.prepare("SELECT docID, docType, clientID, clientAddress, salut, goodbye, date, lastModified, language, country, "
                  "pretext, posttext, docDescription, projectlabel, predecessor FROM document WHERE ident=:ident");
        q.bindValue(":ident", ident);
        q.exec();
        // qDebug () << "Loading document id " << id;

        if( q.next())
        {
            // qDebug () << "loading document with id " << id << endl;
            id = q.value( 0 ).toInt();
            dbID dbid(id);
            doc->setDocID(dbid);

            doc->setIdent(ident);
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
            // Later, the locale can be read from here again.
            // doc->setCountryLanguage( q.value( 8 ).toString(),
            //                         q.value( 9 ).toString());

            doc->setPreTextRaw( KraftDB::self()->mysqlEuroDecode( q.value( 10  ).toString() ) );
            doc->setPostTextRaw(   KraftDB::self()->mysqlEuroDecode( q.value( 11 ).toString() ) );
            doc->setWhiteboard( KraftDB::self()->mysqlEuroDecode( q.value( 12 ).toString() ) );
            doc->setProjectLabel( q.value(13).toString() );
            doc->setPredecessor(  q.value(14).toString() );
        }
    }
    // load the dbID of the predecessor document from the database.
    const QString pIdent = doc->predecessor();
    if( ! pIdent.isEmpty() ) {
        QSqlQuery q1;
        q1.prepare("SELECT ident FROM document WHERE ident=:docID");
        q1.bindValue(":docID", pIdent);
        q1.exec();
        if( q1.next() ) {
            const QString pIdent = q1.value(0).toString();
            doc->setPredecessorDbId(pIdent);
        }
    }

    // finally load the item data.
    if (id > -1) {
        loadPositions( QString::number(id), doc );
    }
    // All docs from the DB are in state Draft. In old Kraft's there were no
    // way to indicate that a document was sent out to the customer really.
    doc->setState(KraftDoc::State::Draft); // FIXME: Check if this is correct.

    return true;
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

    // qDebug () << "* loading document positions for document id " << id;
    while( q.next() ) {
        // qDebug () << " loading position id " << q.value( 0 ).toInt();

        DocPositionBase::PositionType type = DocPositionBase::Position;
        QString typeStr = q.value( 1 ).toString();
        if ( typeStr == PosTypeExtraDiscount ) {
          type = DocPositionBase::ExtraDiscount;
        } else if ( typeStr == PosTypePosition ) {
          // nice, default position type.
          type = DocPositionBase::Position;
        } else {
          // qDebug () << "ERROR: Strange type string loaded from db: " << typeStr;
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

        // dp->loadAttributes();
    }
}

int DocumentSaverDB::addDigestsToModel(DocBaseModel* model)
{
    if (model == nullptr) {
        return -1;
    }

    int cnt = 0;

    QSqlQuery query;

    query.prepare("SELECT docID, ident, docType, docDescription, clientID, lastModified,"
                  "date, projectLabel, clientAddress "
                  "FROM document ORDER BY date DESC");
    query.exec();

    while (query.next()) {
        DocDigest digest(query.value(DocBaseModel::Columns::Document_Id).toInt(),
                         query.value(DocBaseModel::Columns::Document_Type).toString(),
                         query.value(DocBaseModel::Columns::Document_ClientId).toString());

        digest.setDate( query.value(DocBaseModel::Columns::Document_CreationDate ).toDate() );
        QDateTime dt = query.value(DocBaseModel::Columns::Document_LastModified).toDateTime();
        if (KraftDB::self()->isSqlite()) {
            // The timestamps in Sqlite are in UTC
            dt.setTimeSpec(Qt::UTC);
            digest.setLastModified(dt.toLocalTime());
        } else {
            digest.setLastModified(dt);
        }

        const QString clientAdr = query.value(DocBaseModel::Columns::Document_ClientAddress).toString();
        digest.setClientAddress( clientAdr );

        QString ident = query.value(DocBaseModel::Columns::Document_Ident).toString();
        digest.setIdent( ident );
        digest.setWhiteboard( query.value(DocBaseModel::Columns::Document_Whiteboard).toString() );
        digest.setProjectLabel( query.value(DocBaseModel::Columns::Document_ProjectLabel).toString() );

        const QString clientId = query.value(DocBaseModel::Columns::Document_ClientId).toString();
        digest.setClientId( clientId );

        model->addData( digest );
    }
    return cnt;
}

DocumentSaverDB::~DocumentSaverDB( )
{

}

/* END */

