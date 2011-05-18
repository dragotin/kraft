/***************************************************************************
        documentmodel  - the database model for documents
                             -------------------
    begin                : 2010-01-11
    copyright            : Copyright 2010 by Thomas Richard,
                           2011 by Klaas Freitag <freitag@kde.org>
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
#include <kabc/addressee.h>

class DocDigest;
class AddressProvider;

class DocumentModel : public QSqlQueryModel
{
  Q_OBJECT
public:

  DocumentModel();
  ~DocumentModel();

  enum Columns {
    Document_Id = 0,
    Document_Ident = 1,
    Document_Type = 2,
    Document_Whiteboard = 3,
    Document_ClientId = 4,
    Document_LastModified = 5,
    Document_CreationDate = 6,
    Document_ProjectLabel = 7,
    Document_ClientAddress = 8,
    Document_ClientName = 9,

  };

  enum Roles
  {
    RawTypes = Qt::UserRole + 1
  };

  QVariant data(const QModelIndex &idx, int rol) const;
  QVariant headerData( int, Qt::Orientation, int role = Qt::DisplayRole ) const;

  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  DocDigest digest( const QModelIndex& ) const;
  void setQueryAgain();

protected slots:
  void slotAddresseeFound( const QString&, const KABC::Addressee& );

protected:
  AddressProvider   *mAddressProvider;
  QHash<QString, KABC::Addressee> mAddresses;

private:
  QVector<QString> mHeaders;
};

#endif
