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

DocumentModel::DocumentModel()
       : QSqlQueryModel()
{
    setQuery("SELECT docID, ident, docType, docDescription, clientID, lastModified, date, projectLabel FROM document ORDER BY date DESC");

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

//    Document_Id = 0,
//    Document_Ident = 1,
//    Document_Type = 2,
//    Document_Whiteboard = 3,
//    Document_ClientId = 4,
//    Document_Client = 5,
//    Document_Salut = 6,
//    Document_Goodbye = 7,
//    Document_LastModified = 8,
//    Document_CreationDate = 9,
//    Document_Pretext = 10,
//    Document_Posttext = 11,
//    Document_Country = 12,
//    Document_Language = 13,
//    Document_ProjectLabel = 14

    // this->setSort(Document_Id, Qt::AscendingOrder);
    setHeaderData( 0 /* Document_Id */, Qt::Horizontal, i18n("Id"));
    setHeaderData( 1 /* Document_Ident */, Qt::Horizontal, i18n("Doc. number"));
    setHeaderData( 2 /* Document_Type */, Qt::Horizontal, i18n("Doc. type"));
    setHeaderData( 3 /* Document_Whiteboard */,   Qt::Horizontal, i18n("Whiteboard"));
    setHeaderData( 4 /* Document_ClientId */  ,   Qt::Horizontal, i18n("Client ID"));
    setHeaderData( 5 /* Document_LastModified */, Qt::Horizontal, i18n("Last modified"));
    setHeaderData( 6 /* Document_CreationDate */, Qt::Horizontal, i18n("Creation date"));
    setHeaderData( 7 /* Document_ProjectLabel */, Qt::Horizontal, i18n("Project label"));
    setHeaderData( 8 /* Document_ClientName */,   Qt::Horizontal, i18n("Client"));
    // Cache the count of archived documents per document
    // QSqlQuery query("SELECT docID, count(arch.archDocID) FROM document, archdoc arch WHERE document.ident = arch.ident group by docID");
    // query.exec();

    mAdrBook = KABC::StdAddressBook::self();
}

DocumentModel * DocumentModel::self()
{
    K_GLOBAL_STATIC(DocumentModel, mSelf);
    return mSelf;
}

QVariant DocumentModel::data(const QModelIndex &idx, int role) const
{   
  if(idx.parent().isValid()) {  // clicked on an archive item
    if(role == DocumentModel::DataType)
    {
      return DocumentModel::ArchivedType;
    }
    if(role == Qt::DisplayRole)
    {
      //We're showing an archived document
      if(idx.column() == Document_Type)
        return QString(i18n("Archived"));
      if(idx.column() == Document_LastModified)
      {
        QSqlTableModel *arcmodel = archiveModelCache[idx.parent().row()];
        if(!arcmodel)
        {
          arcmodel = new QSqlTableModel;
          arcmodel->setTable("archdoc");
          QString filter = "ident='" + index(idx.parent().row(), Document_Ident).data(Qt::DisplayRole).toString()+"'";
          arcmodel->setFilter(filter);
          arcmodel->select();
          archiveModelCache.replace(idx.parent().row(), arcmodel);
        }

        KLocale *locale = KGlobal::locale();
        QDateTime dt;
        dt.setTime_t(arcmodel->data(arcmodel->index(idx.row(), 7), Qt::DisplayRole).toInt());
        return locale->formatDateTime(dt, KLocale::FancyLongDate);
      }
      return QString();
    }

    return QSqlQueryModel::data(idx, role);
  }

  if(role == DocumentModel::DataType)
  {
    return DocumentModel::DocumentType;
  }

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
    } else if(idx.column() == Document_ClientId || idx.column() == Document_ClientName ) {
      QString uid = QSqlQueryModel::data( idx, role ).toString();

      if( idx.column() == Document_ClientId )
        return uid;

      QModelIndex uidIdx = idx.sibling( idx.row(), Document_ClientId );
      uid = QSqlQueryModel::data( uidIdx, role ).toString();
      // kDebug() << "Searching address for UID " << uid;
      if( uid.isEmpty()) return "";

      KABC::Addressee contact;
      if( ! mAddressNameCache.contains( uid )) {
         contact = mAdrBook->findByUid( uid );
         mAddressNameCache[uid] = contact;
      } else {
        contact = mAddressNameCache[uid];
      }
      QString name = uid;
      if( ! contact.isEmpty() ) {
          name = contact.realName();
      }
      return name;
    }

  } else if( role == RawTypes ) {
    if(idx.column() == Document_LastModified ) {
      return QSqlQueryModel::data( idx, Qt::DisplayRole ).toDateTime();
    } else if( idx.column() == Document_CreationDate ) {
      return QSqlQueryModel::data( idx, Qt::DisplayRole ).toDate();
    }
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

  QString ident = data( index.sibling( index.row(), Document_Ident), Qt::DisplayRole ).toString();
  digest.setIdent( ident );
  digest.setWhiteboard( data( index.sibling( index.row(), Document_Whiteboard), Qt::DisplayRole).toString() );
  digest.setProjectLabel( data( index.sibling( index.row(), Document_ProjectLabel), Qt::DisplayRole).toString() );

  const QString clientId = data( index.sibling( index.row(), Document_ClientId), Qt::DisplayRole).toString();
  digest.setClientId( clientId );
  KABC::Addressee contact = mAddressNameCache[clientId];
  if( ! contact.isEmpty() ) {
    digest.setAddressee( contact );
  }
  // get the arch doc information

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
    if(archiveCountCache[parent.row()] > 0 && !parent.parent().isValid())
        return true;

    return false;
}

int DocumentModel::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid() && !parent.parent().isValid())
        return archiveCountCache[parent.row()];

    return QSqlQueryModel::rowCount(parent);
}

int DocumentModel::columnCount(const QModelIndex &parent) const
{
    if(!parent.isValid() || !parent.parent().isValid())
        return 1+QSqlQueryModel::columnCount(QModelIndex());

    return 0;
}

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
