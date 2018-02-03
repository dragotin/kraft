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

#include "docbasemodel.h"
#include <QSqlTableModel>


class DocDigest;
class AddressProvider;

class DocumentModel : public DocBaseModel
{
  Q_OBJECT
public:

  DocumentModel(QObject *parent = 0);
  ~DocumentModel();

  QVariant data(const QModelIndex &idx, int role) const;
 // QVariant headerData( int, Qt::Orientation, int role = Qt::DisplayRole ) const;
  QModelIndex index(int row, int column, const QModelIndex &parent) const;
  QModelIndex parent(const QModelIndex &index) const;
  int rowCount(const QModelIndex &parent) const;

  // int columnCount(const QModelIndex &parent = QModelIndex()) const;
  DocDigest digest( const QModelIndex& ) const;
  void setQueryAgain();

  void removeAllData();
  void addData( const DocDigest& );

  bool isDocument(const QModelIndex& indx) const;


// protected slots:
//  void slotAddresseeFound( const QString&, const KContacts::Addressee& );

protected:


private:
  DocDigestList _digests;

};

#endif
