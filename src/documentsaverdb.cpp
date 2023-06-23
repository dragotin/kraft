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

DocumentSaverDB::DocumentSaverDB( ) : DocumentSaverBase(),
                                      PosTypePosition( QString::fromLatin1( "Position" ) ),
                                      PosTypeExtraDiscount( QString::fromLatin1( "ExtraDiscount" ) ),
                                      PosTypeHeader( QString::fromLatin1( "Header" ) )
{

}

bool DocumentSaverDB::saveDocument( KraftDoc* )
{
    return false;
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
    // All docs from the DB are in unknown state. In old Kraft's there were no
    // way to indicate that a document was sent out to the customer really.
    doc->setState(KraftDoc::State::Converted);

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
        DocDigest digest(query.value(DocBaseModel::Columns::Document_Type).toString(),
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

        const QString id = query.value(DocBaseModel::Columns::Document_Id).toString();
        digest.setUuid(id);
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

