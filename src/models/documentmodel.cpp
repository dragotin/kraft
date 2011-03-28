/***************************************************************************
        documentmodel  - the database model for documents
                             -------------------
    begin                : 2010-01-11
    copyright            : (C) 2010 by Thomas Richard
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

    setHeaderData( 0 /* Document_Id */, Qt::Horizontal, i18n("Id"));
    setHeaderData( 1 /* Document_Ident */, Qt::Horizontal, i18n("Doc. number"));
    setHeaderData( 2 /* Document_Type */, Qt::Horizontal, i18n("Doc. type"));
    setHeaderData( 3 /* Document_Whiteboard */,   Qt::Horizontal, i18n("Whiteboard"));
    setHeaderData( 4 /* Document_ClientId */  ,   Qt::Horizontal, i18n("Client ID"));
    setHeaderData( 5 /* Document_LastModified */, Qt::Horizontal, i18n("Last modified"));
    setHeaderData( 6 /* Document_CreationDate */, Qt::Horizontal, i18n("Creation date"));
    setHeaderData( 7 /* Document_ProjectLabel */, Qt::Horizontal, i18n("Project label"));
    setHeaderData( 8 /* Document_ClientAddress */,   Qt::Horizontal, i18n("Client Address"));
    setHeaderData( 9 /* Document_ClientName */,   Qt::Horizontal, i18n("Client"));
    mAddressProvider = new AddressProvider( this );
    connect( mAddressProvider, SIGNAL( addresseeFound( const QString&, const KABC::Addressee& )),
             this, SLOT( slotAddresseeFound( const QString&, const KABC::Addressee& )));
    setQueryAgain();
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
  }

  mAddresses[addressee.uid()] = addressee;
}

#if 0
DocumentModel * DocumentModel::self()
{
    K_GLOBAL_STATIC(DocumentModel, mSelf);
    return mSelf;
}
#endif

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

      kDebug() << "Checking for UID " << uid;
      if( uid.isEmpty() ) return "";

      if( mAddresses.contains( uid ) ) {
        if( mAddresses.value(uid).isEmpty() ) {
          // empty address means that there is no valid entry in this addressbook
          return i18n("not found");
        }
        return mAddresses.value(uid).realName();
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

bool DocumentModel::hasChildren(const QModelIndex &parent) const
{
    if(!parent.isValid())
        return true;

    return false;
}
#if 0
int DocumentModel::columnCount(const QModelIndex &parent) const
{
    if(!parent.isValid() || !parent.parent().isValid())
        return 1+QSqlQueryModel::columnCount(QModelIndex());

    return 0;
}
#endif
QModelIndex DocumentModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || column >= columnCount() || parent.column() > 0)
        return QModelIndex();

    if(parent.isValid())
    {
        return createIndex(row, column, parent.row()+1);
    }

    return createIndex(row, column, 0);
}

QModelIndex DocumentModel::parent(const QModelIndex &index) const
{
    if(!index.isValid())
        return QModelIndex();
    if(index.internalId() != 0)
        return createIndex(index.internalId()-1, 0, 0);

    return QModelIndex();
}

QModelIndex DocumentModel::sibling ( int row, int column, const QModelIndex & index ) const
{
    if(!index.isValid())
        return QModelIndex();

    return createIndex(row, column, (int) index.internalId());
}

bool DocumentModel::canFetchMore(const QModelIndex &parent) const
{
    if(parent.isValid())
        return true;

    return QSqlQueryModel::canFetchMore(parent);
}
