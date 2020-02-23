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
#include <QDate>
#include <QSqlQuery>
#include <QLocale>
#include <QFont>
#include <QFontMetrics>
#include <QDebug>

//KDE includes
#include <klocalizedstring.h>

//Kraft includes
#include "documentmodel.h"
#include "docdigest.h"
#include "docbasemodel.h"
#include "defaultprovider.h"

DocumentModel::DocumentModel(QObject *parent)
       : DocBaseModel(parent)
{
}

DocumentModel::~DocumentModel()
{
}

int DocumentModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return _digests.count();
}

void DocumentModel::removeAllData()
{
    _digests.clear();
}

void DocumentModel::addData( const DocDigest& digest )
{
    _digests.append(digest);
}

QModelIndex DocumentModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
         return QModelIndex();

    return createIndex(row, column);
}

QModelIndex DocumentModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

bool DocumentModel::isDocument(const QModelIndex& indx) const
{
    Q_UNUSED(indx);
    return indx.isValid();
}

QVariant DocumentModel::data(const QModelIndex &idx, int role) const
{   
    if( !idx.isValid() )
        return QVariant();

    int row = idx.row();
    if( row < 0 || row >= _digests.count() ) {
        return QVariant();
    }
    const DocDigest digest = _digests.at(row);

    if(role == Qt::DisplayRole) {
        return columnValueFromDigest( digest, idx.column() );
    } else if( role == Qt::SizeHintRole ) {
        QFont f = data(idx, Qt::FontRole).value<QFont>();
        QFontMetrics fm(f);
        int h = fm.height();

        return QSize( 0, h + 4 );
    }
    return QVariant();
}



DocDigest DocumentModel::digest( const QModelIndex& index ) const
{
    int row = index.row();

    DocDigest digest;

    if( row > -1 && row < _digests.count() ) {
        digest = _digests.at(index.row());
    }
    return digest;
}
