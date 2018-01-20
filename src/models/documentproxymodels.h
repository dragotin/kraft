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

class DateModel;
class DocumentModel;

//Filters out the last 10 items  of the DocumentModel
class DocumentFilterModel : public QSortFilterProxyModel
{
    public:
        DocumentFilterModel(int maxRows = -1, QObject *parent = 0);
        void setMaxRows( int );
        void setEnableTreeview( bool treeview );
    protected:
        bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
        bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;

    private:
        int m_MaxRows;
        bool _enableTreeView;

        QScopedPointer<DateModel> _treeModel;
        QScopedPointer<DocumentModel> _tableModel;
};

#endif
