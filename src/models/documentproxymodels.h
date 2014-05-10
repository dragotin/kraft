/***************************************************************************
        documentproxymodels  - contains proxymodels to show the
                               documentmodel in different views
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

#ifndef DOCUMENTPROXYMODELS_H
#define DOCUMENTPROXYMODELS_H

#include <QSortFilterProxyModel>
#include <QVector>

class QModelIndex;
class QVariant;
class QObject;

class DocumentModel;

//Filters out the last 10 items  of the DocumentModel
class DocumentFilterModel : public QSortFilterProxyModel
{
    public:
        DocumentFilterModel(int maxRows = -1, QObject *parent = 0);
        void setMaxRows( int );
    protected:
        bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

    private:
        QScopedPointer<DocumentModel> mProxy;

        int m_MaxRows;
};

struct Helper
{
    QVector<int> modelRowCache;
    QVector<QDate> startDate;
};

struct Mapping
{
    Mapping *parent;
    QVector<Mapping *> childeren;
    int parentRow;
    int treeLevel;
};

/*
Creates a timelined view of the DocumentModel

Mapping helps us to keep track of where we are in the tree

m_yearsRowCache and it's helper struct is used to store at what row in the sourcemodel a given
year/month starts
*/

class TimelineModel : public QAbstractProxyModel
{
    Q_OBJECT

    public:
        TimelineModel(QObject *parent = 0);

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
        int columnCount(const QModelIndex &parent) const;
        int rowCount(const QModelIndex &parent = QModelIndex()) const;
        QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;
        QModelIndex mapToSource(const QModelIndex &proxyIndex) const;
        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
        QModelIndex parent(const QModelIndex &index= QModelIndex()) const;
        DocumentModel *baseModel();

        bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
        Qt::ItemFlags flags(const QModelIndex &index) const;
        bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

        void setSourceModel(QAbstractItemModel *sourceModel);

    private slots:
        void sourceReset();
        void sourceRowsInserted(const QModelIndex &parent, int start, int end);
        void sourceRowsRemoved(const QModelIndex &parent, int start, int end);

    private:
        QScopedPointer<DocumentModel> mProxy;
        int sourceDateRow(int yearRow, int monthRow) const;
        mutable QVector<Helper> m_yearsRowCache;
        mutable Mapping *m_rootMap;
};

#endif
