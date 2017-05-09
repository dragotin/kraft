/***************************************************************************
        latestdocmodel  - the latest documents model
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
#include <QSortFilterProxyModel>
#include <QAbstractProxyModel>
#include <QtGlobal>
#include <QModelIndex>
#include <QVector>
#include <QObject>
#include <QVariant>
#include <QDate>
#include <QLocale>
#include <QDebug>

//Kraft includes
#include "documentmodel.h"
#include "defaultprovider.h"
#include "docdigest.h"
#include "datemodel.h"

#include "documentproxymodels.h"

DocumentFilterModel::DocumentFilterModel(int maxRows, QObject *parent)
        : QSortFilterProxyModel(parent),
          _enableTreeView(false)
{
    m_MaxRows = maxRows;
    _sourceModel.reset(new DateModel);
    _sourceModel->setColumnCount(6);
    _sourceModel->fromTable();
    this->setSourceModel( _sourceModel.data() );
    this->setFilterCaseSensitivity(Qt::CaseInsensitive);
}

void DocumentFilterModel::setMaxRows( int max )
{
  m_MaxRows = max;
  invalidateFilter(); // refreshes the model
}

void DocumentFilterModel::setEnableTreeview( bool treeview )
{
    _enableTreeView = treeview;
}

bool DocumentFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    bool isLeafItem = sourceParent.isValid() && sourceParent.parent().isValid();

    //The documentmodel is sorted by date, we only want to accept the last n items as they are the newest
    if(isLeafItem && m_MaxRows != -1) {
        if( sourceRow > m_MaxRows ) // sourceModel()->rowCount() - m_MaxRows)
            return false;
    }

    if( !_enableTreeView ) {
        // FIXME: Check for the type and filter out all year and month rows.
    }

    // filter works on the document ID, the client name and the document type.
    const QRegExp filter = filterRegExp();
    const QModelIndex index0 = sourceModel()->index(sourceRow, DocumentModel::Document_Ident, sourceParent);
    const QString idStr = sourceModel()->data(index0).toString();

    const QModelIndex index1 = sourceModel()->index(sourceRow, DocumentModel::Document_Type, sourceParent);
    const QString typeStr = sourceModel()->data(index1).toString();

    const QModelIndex index2 = sourceModel()->index(sourceRow, DocumentModel::Document_ClientName, sourceParent);
    const QString clientNameStr = sourceModel()->data(index2).toString();


    if( !( idStr.contains(filter) || typeStr.contains(filter) || clientNameStr.contains(filter)) ) {
        return false;
    }

    return true;
}

