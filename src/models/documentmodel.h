/***************************************************************************
        documentmodel  - the database model for documents
                             -------------------
    begin                : 2010-01-11
    copyright            : Copyright 2010 by Thomas Richard
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

#ifndef DOCUMENTMODEL_H
#define DOCUMENTMODEL_H

#include <QSqlTableModel>
#include <QVector>

#include <kabc/addressbook.h>

class DocDigest;

class DocumentModel : public QSqlQueryModel
{
public:

  enum Columns {
    Document_Id = 0,
    Document_Ident = 1,
    Document_Type = 2,
    Document_Whiteboard = 3,
    Document_ClientId = 4,
    Document_LastModified = 5,
    Document_CreationDate = 6,
    Document_ProjectLabel = 7
  };

  enum Roles
  {
    DataType = Qt::UserRole + 1,
    RawTypes = Qt::UserRole + 2
  };

  enum DataTypes
  {
    DocumentType = 0,
    ArchivedType = 1
  };

  static DocumentModel *self();
  QVariant data(const QModelIndex &idx, int rol) const;
  bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex &index= QModelIndex()) const;
  QModelIndex sibling ( int row, int column, const QModelIndex & index ) const;
  bool canFetchMore(const QModelIndex &parent) const;
  DocDigest digest( const QModelIndex& ) const;

protected:
  DocumentModel();

  static DocumentModel *mSelf;
  QVector<int> archiveCountCache;
  QVector<QDateTime> mArchiveDocCache;
  mutable QHash<QString, KABC::Addressee> mAddressNameCache;

  mutable QVector<QSqlTableModel*> archiveModelCache;

  KABC::AddressBook *mAdrBook;
};

#endif
