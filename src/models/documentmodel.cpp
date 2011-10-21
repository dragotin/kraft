/***************************************************************************
        documentmodel  - the database model for documents
                             -------------------
    begin                : 2010-01-11
    copyright            : (C) 2010 by Thomas Richard, 2011 by Klaas Freitag
    email                : thomas.richard@proan.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//QT includes
#include <QSqlTableModel>
#include <QDate>
#include <QSqlQuery>

//KDE includes
#include <kglobal.h>
#include <kdebug.h>
#include <klocale.h>
#include <kabc/addressbook.h>
#include <kabc/addressee.h>
#include <kabc/stdaddressbook.h>

//Kraft includes
#include "documentmodel.h"
#include "docdigest.h"
#include "addressprovider.h"

DocumentModel::DocumentModel()
       : QSqlQueryModel()
{
  //    mysql> describe document;
  //    +------------------+--------------+------+-----+-------------------+-----------------------------+
  //    | Field            | Type         | Null | Key | Default           | Extra                       |
  //    +------------------+--------------+------+-----+-------------------+-----------------------------+
  //    |0  docID          | int(11)      | NO   | PRI | NULL              | auto_increment              |
  //    |1  ident          | varchar(32)  | YES  | MUL | NULL              |                             |
  //    |2  docType        | varchar(255) | YES  |     | NULL              |                             |
  //    |3  docDescription | text         | YES  |     | NULL              |                             |
  //    |4  clientID       | varchar(32)  | YES  | MUL | NULL              |                             |
  //    |5  clientAddress  | text         | YES  |     | NULL              |                             |
  //    |6  salut          | varchar(255) | YES  |     | NULL              |                             |
  //    |7  goodbye        | varchar(128) | YES  |     | NULL              |                             |
  //    |8  lastModified   | timestamp    | NO   |     | CURRENT_TIMESTAMP | on update CURRENT_TIMESTAMP |
  //    |9  date           | date         | YES  |     | NULL              |                             |
  //    |10  pretext       | text         | YES  |     | NULL              |                             |
  //    |11  posttext      | text         | YES  |     | NULL              |                             |
  //    |12  country       | varchar(32)  | YES  |     | NULL              |                             |
  //    |13  language      | varchar(32)  | YES  |     | NULL              |                             |
  //    |14  projectLabel  | varchar(255) | YES  |     | NULL              |                             |
  //    +------------------+--------------+------+-----+-------------------+-----------------------------+
  //    15 rows in set (0.00 sec)
  mHeaders.resize(10);

  mHeaders[ Document_Id ]            = i18n("Id");
  mHeaders[ Document_Ident ]         = i18n("Doc. Number");
  mHeaders[ Document_Type ]          = i18n( "Doc. Type");
  mHeaders[ Document_Whiteboard ]    = i18n( "Whiteboard" );
  mHeaders[ Document_ClientId ]      = i18n( "Client Id" );
  mHeaders[ Document_LastModified]   = i18n( "Last modified" );
  mHeaders[ Document_CreationDate]   = i18n( "Creation date" );
  mHeaders[ Document_ProjectLabel]   = i18n( "Project" );
  mHeaders[ Document_ClientAddress ] = i18n( "Client Address" );
  mHeaders[ Document_ClientName ]    = i18n( "Client" );

  mAddressProvider = new AddressProvider( this );
  connect( mAddressProvider, SIGNAL( addresseeFound( const QString&, const KABC::Addressee& )),
          this, SLOT( slotAddresseeFound( const QString&, const KABC::Addressee& )));
  setQueryAgain();
}

DocumentModel::~DocumentModel()
{
  delete mAddressProvider;
}

void DocumentModel::setQueryAgain()
{
   setQuery("SELECT docID, ident, docType, docDescription, clientID, lastModified, date, projectLabel, clientAddress "
            "FROM document ORDER BY date DESC");
}

void DocumentModel::slotAddresseeFound( const QString& uid, const KABC::Addressee & addressee )
{
  if( addressee.isEmpty() ) {
    kDebug() << "No address found for uid " << uid;
    mAddresses[uid] = KABC::Addressee();
  } else {
    mAddresses[uid] = addressee;
  }
}

QVariant DocumentModel::headerData( int section, Qt::Orientation /* orientation */, int role ) const
{
  if( role == Qt::DisplayRole && section >= 0 && section < 10 ) {
    return mHeaders.value( section );
  }
  return QVariant();
}

QVariant DocumentModel::data(const QModelIndex &idx, int role) const
{   
  if(role == Qt::DisplayRole)
  {
    if(idx.column() == Document_LastModified ) {
      KLocale *locale = KGlobal::locale();
      QDateTime date = QSqlQueryModel::data(idx, role).toDateTime();
      return locale->formatDateTime( date, KLocale::ShortDate );
    } else if( idx.column() == Document_CreationDate ) {
      KLocale *locale = KGlobal::locale();
      QDate date = QSqlQueryModel::data( idx, role ).toDate();
      return locale->formatDate( date, KLocale::ShortDate );
    } else if(idx.column() == Document_ClientId ) {
      const QString uid = QSqlQueryModel::data( idx, role ).toString();
      return uid;
    } else if( idx.column() == Document_ClientName ) {
      QModelIndex uidIdx = idx.sibling( idx.row(), Document_ClientId );
      const QString uid = QSqlQueryModel::data( uidIdx, role ).toString();

      // kDebug() << "Checking for UID " << uid;
      if( uid.isEmpty() ) return "";

      if( mAddresses.contains( uid ) ) {
        if( mAddresses.value(uid).isEmpty() ) {
          // empty address means that there is no valid entry in this addressbook
          return i18n("not found");
        }
        const QString realName = mAddresses.value(uid).realName();

        // kDebug() << "returning " << realName;
        return realName;
      } else {
        mAddressProvider->getAddressee( uid );
      }
      return i18n("retrieving...");
    }
  } else if( role == RawTypes ) {
    if(idx.column() == Document_LastModified ) {
      return QSqlQueryModel::data( idx, Qt::DisplayRole ).toDateTime();
    } else if( idx.column() == Document_CreationDate ) {
      return QSqlQueryModel::data( idx, Qt::DisplayRole ).toDate();
    }
  } else if( role == Qt::SizeHintRole ) {
    QFont f = QSqlQueryModel::data(idx, Qt::FontRole ).value<QFont>();
    QFontMetrics fm(f);
    int h = fm.height();

    return QSize( 0, h + 4 );
  }
  return QSqlQueryModel::data(idx, role);
}

DocDigest DocumentModel::digest( const QModelIndex& index ) const
{
  DocDigest digest( dbID( data(index.sibling(index.row(), Document_Id ),   Qt::DisplayRole).toInt() ),
                    data( index.sibling( index.row(), Document_Type ),     Qt::DisplayRole).toString(),
                    data( index.sibling( index.row(), Document_ClientId ), Qt::DisplayRole).toString() );

  digest.setDate( data( index.sibling( index.row(), Document_CreationDate ), RawTypes ).toDate() );
  digest.setLastModified( data( index.sibling( index.row(), Document_LastModified), RawTypes ).toDateTime() );

  const QString clientAdr = data( index.sibling( index.row(), Document_ClientAddress), Qt::DisplayRole).toString();
  digest.setClientAddress( clientAdr );

  QString ident = data( index.sibling( index.row(), Document_Ident), Qt::DisplayRole ).toString();
  digest.setIdent( ident );
  digest.setWhiteboard( data( index.sibling( index.row(), Document_Whiteboard), Qt::DisplayRole).toString() );
  digest.setProjectLabel( data( index.sibling( index.row(), Document_ProjectLabel), Qt::DisplayRole).toString() );

  const QString clientId = data( index.sibling( index.row(), Document_ClientId), Qt::DisplayRole).toString();
  digest.setClientId( clientId );
  if( mAddresses.contains( clientId )) {
    digest.setAddressee( mAddresses.value( clientId ));
  }

  kDebug() << "Querying archdocs for document ident " << ident;
  QSqlQuery query("SELECT archDocID, ident, printDate, state FROM archdoc WHERE ident='" + ident +"' ORDER BY printDate DESC" );
  query.exec();

  while(query.next())
  {
    int archDocID = query.value(0).toInt();
    const QString dbIdent = query.value(1).toString();
    QDateTime printDateTime = query.value(2).toDateTime();
    int state = query.value(3).toInt();

    digest.appendArchDocDigest( ArchDocDigest( printDateTime, state, dbIdent, dbID(archDocID) ) );
  }
  return digest;
}

int DocumentModel::columnCount(const QModelIndex &) const
{
  return 10;
}
