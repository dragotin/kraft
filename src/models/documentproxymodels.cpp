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

//KDE includes
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kcalendarsystem.h>

//Kraft includes
#include "documentmodel.h"
#include "defaultprovider.h"

#include "documentproxymodels.h"

DocumentFilterModel::DocumentFilterModel(int maxRows, QObject *parent)
        : QSortFilterProxyModel(parent)
{
    m_MaxRows = maxRows;
    mProxy.reset(new DocumentModel );
    this->setSourceModel( mProxy.data() );
    this->setSortRole(Qt::EditRole);
}

void DocumentFilterModel::setMaxRows( int max )
{
  m_MaxRows = max;
  invalidateFilter(); // refreshes the model
}

bool DocumentFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    //The documentmodel is sorted by date, we only want to accept the last n items as they are the newest
    if(!sourceParent.isValid() && m_MaxRows != -1)
        if( sourceRow > m_MaxRows ) // sourceModel()->rowCount() - m_MaxRows)
            return false;
    return true;
}

// ######################################################################################################

TimelineModel::TimelineModel(QObject *parent)
       : QAbstractProxyModel(parent)
{
    //First put the documentmodel in sortfilterproxymodel to sort the items by date
    mProxy.reset( new DocumentModel );
    mProxy->sort(DocumentModel::Document_CreationDate, Qt::AscendingOrder);

    m_rootMap = new Mapping;
    m_rootMap->parentRow = -1;
    m_rootMap->treeLevel = -1;
    m_rootMap->parent = 0;

    setSourceModel(mProxy.data());
    reset();
}

DocumentModel *TimelineModel::baseModel()
{
    return static_cast<DocumentModel*>( mProxy.data() );
}

QVariant TimelineModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return sourceModel()->headerData(section, orientation, role);
}

QVariant TimelineModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        Mapping *indexMap = static_cast<Mapping*>(index.internalPointer());

        if(index.column() == 0)
        {
            if(indexMap->treeLevel == 0)
                return m_yearsRowCache.at(index.row()).startDate.at(0).year();
            if(indexMap->treeLevel == 1)
            {
                QDate date = m_yearsRowCache.at(index.parent().row()).startDate.at(index.row());
                return DefaultProvider::self()->locale()->calendar()->monthName( date.month(), date.year() );
            }
        }
        return mapToSource(index).data(role);
    }

    return QAbstractProxyModel::data(index, role);
}

int TimelineModel::columnCount(const QModelIndex &parent) const
{
    return sourceModel()->columnCount(mapToSource(parent));
}

int TimelineModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0 || !sourceModel())
        return 0;

    // Row count of years
    if (!parent.isValid()) {
        if (!m_yearsRowCache.isEmpty())
            return m_yearsRowCache.count();

        //If we didn't cache this yet, we'll run over the model to see what years and months are in there
        //We assume that de data in the model is sorted by date (DESC)
        int rows = 0;
        int currentYear = 0;
        int currentMonth = 0;
        int totalRows = sourceModel()->rowCount();

        for (int i = 0; i < totalRows; ++i) {
            QDate rowDate = sourceModel()->index(i, 6 /* Creation Date */ ).data(Qt::EditRole).toDate();
            if(rowDate.year() != currentYear)
            {
                Helper helper;
                m_yearsRowCache.append(helper);
                currentYear = rowDate.year();
                currentMonth = 0;
                ++rows;
            }
            if(rowDate.month() != currentMonth) {
                currentMonth = rowDate.month();
                m_yearsRowCache.last().modelRowCache.append(i);
                m_yearsRowCache.last().startDate.append(rowDate);
            }
        }

        /*
        //Print tree;
        for(int i=0; i< m_yearsRowCache.count(); ++i)
        {
            kDebug() << " Year " << i;
            for(int j=0; j < m_yearsRowCache.at(i).modelRowCache.count(); ++j)
                kDebug() << " Month " << j << " rowID" << m_yearsRowCache.at(i).modelRowCache.at(j);
        }
        */
        Q_ASSERT(m_yearsRowCache.count() == rows);
        m_rootMap->childeren.resize(rows);
        return rows;
    }

    Mapping *parentMap = static_cast<Mapping*>(parent.internalPointer());

    //Number of months in a given year
    if(parentMap->treeLevel == 0)
    {
        int count = m_yearsRowCache[parent.row()].modelRowCache.count();
        parentMap->childeren.resize(count);
        //kDebug() << "Year row: " << parent.row() << " month count: " << count;
        return count;
    }

     //Number of documents in a given month
     if(parentMap->treeLevel == 1)
     {
        int start = sourceDateRow(parent.parent().row(), parent.row());
        int end = sourceDateRow(parent.parent().row(), parent.row()+1);
        //kDebug() << "Year row: " << parent.parent().row() << " Month row: " << parent.row() << " Doc count: " << end << " - " << start;
        parentMap->childeren.resize(end - start);
        return (end - start);
    }

    int count = sourceModel()->rowCount(mapToSource(parent));
    parentMap->childeren.resize(count);
    return count;
}

// Translate the top level date row into the offset where that date starts
int TimelineModel::sourceDateRow(int yearRow, int monthRow) const
{
    if (yearRow < 0 || monthRow < 0)
    return 0;

    //Build the cache first if that didn't happen yet
    if (m_yearsRowCache.isEmpty())
        rowCount(QModelIndex());

    //If the row we want to reach is out of range of this month we return the first row of the next month.
    //If there is no next month, we'll return rowCount() as we're at the end
    if(yearRow >= m_yearsRowCache.count())
        return sourceModel()->rowCount();
    if(monthRow >= m_yearsRowCache.at(yearRow).modelRowCache.count())
    {
        ++yearRow;
        if(yearRow >= m_yearsRowCache.count())
            return sourceModel()->rowCount();

        return m_yearsRowCache.at(yearRow).modelRowCache.at(0);
    }

    return m_yearsRowCache[yearRow].modelRowCache.at(monthRow);
}

QModelIndex TimelineModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if(proxyIndex.isValid())
    {
        Mapping *indexMap = static_cast<Mapping*>(proxyIndex.internalPointer());
        if(indexMap->treeLevel == 2)
        {
            return sourceModel()->index(m_yearsRowCache.at(indexMap->parent->parentRow).modelRowCache.at(indexMap->parentRow) + proxyIndex.row(), proxyIndex.column());
        }

        if(indexMap->treeLevel > 2)
        {
            QModelIndex parent = mapToSource(proxyIndex.parent());
            return sourceModel()->index(proxyIndex.row(), proxyIndex.column(), parent);
        }
    }

    return QModelIndex();
}

QModelIndex TimelineModel::index(int row, int column, const QModelIndex &parent) const
{
  if (row < 0 || column < 0 || column >= columnCount(parent) || parent.column() > 0)
    return QModelIndex();

  if (!parent.isValid())
  {
    if( m_rootMap->childeren.count() == 0 ) {
      // No entries at all
      return QModelIndex();
    }
    Mapping *mapping = static_cast<Mapping*>( m_rootMap->childeren.at(row) );

    if(!mapping)
    {
      mapping = new Mapping;
      mapping->parent = m_rootMap;
      mapping->parentRow = -1;
      mapping->treeLevel = 0;
      m_rootMap->childeren.replace(row, mapping);
    }

    return createIndex(row, column, mapping);
  }


  Mapping *parentmap = static_cast<Mapping*>(parent.internalPointer());
  Mapping *mapping = static_cast<Mapping*>(parentmap->childeren.at(row));

  if(!mapping)
  {
    mapping = new Mapping;
    mapping->parent = parentmap;
    mapping->parentRow = parent.row();
    mapping->treeLevel = parentmap->treeLevel + 1;

    kDebug() << "Index created " << row << " parent row " << parent.row() << " treelevel " << mapping->treeLevel;

    parentmap->childeren.replace(row, mapping);
  }

  //We're creating a month item
  return createIndex(row, column, mapping);
}

QModelIndex TimelineModel::parent(const QModelIndex &index) const
{
  if ( !index.isValid() )
    return QModelIndex();

  Mapping *indexmap = static_cast<Mapping*>(index.internalPointer());

  if( indexmap && indexmap->parent == m_rootMap )
    return QModelIndex();

  return createIndex(indexmap->parentRow, 0, indexmap->parent);
}

bool TimelineModel::hasChildren(const QModelIndex &parent) const
{
    if(!parent.isValid())
        return true;

    Mapping *parentMap = static_cast<Mapping*>(parent.internalPointer());
    if(parentMap->treeLevel <= 1)
        return true;

    return sourceModel()->hasChildren(mapToSource(parent));
}

Qt::ItemFlags TimelineModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;
}

bool TimelineModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(row);
    Q_UNUSED(count);
    Q_UNUSED(parent);
    //Not needed as we'll remove rows directly in the sourcemodel

    return false;
}

void TimelineModel::setSourceModel(QAbstractItemModel *newSourceModel)
{
    if (sourceModel()) {
        disconnect(sourceModel(), SIGNAL(modelReset()), this, SLOT(sourceReset()));
        disconnect(sourceModel(), SIGNAL(layoutChanged()), this, SLOT(sourceReset()));
        disconnect(sourceModel(), SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                this, SLOT(sourceRowsInserted(const QModelIndex &, int, int)));
        disconnect(sourceModel(), SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                this, SLOT(sourceRowsRemoved(const QModelIndex &, int, int)));
    }

    QAbstractProxyModel::setSourceModel(newSourceModel);

    if (newSourceModel) {
        connect(sourceModel(), SIGNAL(modelReset()), this, SLOT(sourceReset()));
        connect(sourceModel(), SIGNAL(layoutChanged()), this, SLOT(sourceReset()));
        connect(sourceModel(), SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                this, SLOT(sourceRowsInserted(const QModelIndex &, int, int)));
        connect(sourceModel(), SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                this, SLOT(sourceRowsRemoved(const QModelIndex &, int, int)));
    }

    reset();
}

void TimelineModel::sourceReset()
{
    m_yearsRowCache.clear();
    reset();
}

void TimelineModel::sourceRowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    //Fixme: Implementation needed
}

QModelIndex TimelineModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    Q_UNUSED(sourceIndex);
    //This one is not needed as long as we don't edit in our timelined view
    return QModelIndex();
}

void TimelineModel::sourceRowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    //Fixme: Implementation needed
}
