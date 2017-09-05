/***************************************************************************
                            datemodel.h
                          -------------------
    copyright            : (C) 2017 by Klaas Freitag
    email                : klaas@volle-kraft-voraus.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DOCBASEMODEL_H
#define DOCBASEMODEL_H

#include <kcontacts/addressee.h>

#include "addressprovider.h"
#include "docdigest.h"

#include <QAbstractItemModel>
#include <QDate>
#include <QList>
#include <QVector>
#include <QDebug>
#include <QStringList>


class DocBaseModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    DocBaseModel(QObject *parent = 0);
    virtual ~DocBaseModel() {}

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
        Document_Id_Raw = 10,
        Document_CreationDateRaw = 11,
        Treestruct_Year = 12,
        Treestruct_Month = 13,
        Treestruct_Type  = 14,
        Max_Column_Marker = 15   // leave this as last enum
    };

    int columnCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const = 0;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    virtual int rowCount(const QModelIndex &parent) const = 0;
    virtual void removeAllData() = 0;
    virtual void addData(const DocDigest& digest) = 0;

    virtual DocDigest digest(const QModelIndex& indx) const = 0;
    virtual bool isDocument(const QModelIndex& indx) const = 0;

    int loadFromTable();

    void resetData();

protected:
    QVariant columnValueFromDigest( const DocDigest& digest, int col ) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

protected slots:
    void slotAddresseeFound(const QString &uid, const KContacts::Addressee &contact);

private:
    QString firstLineOf( const QString& str) const;

    QVector<QString> _headers;

    AddressProvider   *mAddressProvider;
};

#endif // DATEMODEL_H
