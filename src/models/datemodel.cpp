/***************************************************************************
                            datemodel.cpp
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

#include "datemodel.h"

#include <QStringList>
#include <QColor>
#include <QSqlQuery>

class TreeItem
{
public:
    TreeItem(AbstractIndx *indx, TreeItem *parent = 0);
    ~TreeItem();

    void appendChild(TreeItem *child);

    TreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    TreeItem *parent();

    QList<TreeItem*> children() { return childItems; }
    AbstractIndx *payload() { return dataPtr; }
private:
    QList<TreeItem*> childItems;
    AbstractIndx *dataPtr;
    TreeItem *parentItem;
};


TreeItem::TreeItem(AbstractIndx *indx, TreeItem *parent)
    :dataPtr(indx), parentItem(parent)
{
    // make sure the parents have the children registered
    if( parent ) {
        parent->appendChild(this);
    }
}

int TreeItem::columnCount() const
{
    if( dataPtr ) {
        return dataPtr->columnCount();
    }
    return 0;
}

TreeItem* TreeItem::child(int row)
{
    return childItems.value(row);
}

TreeItem* TreeItem::parent()
{
    return parentItem;
}

int TreeItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

    return 0;
}

int TreeItem::childCount() const
{
    return childItems.count();
}

void TreeItem::appendChild(TreeItem *child)
{
    childItems.append(child);
}

/* ================================================================== */

QVariant AbstractIndx::data(int column) const
{
    if( _data.count() > column ) {
        return _data.at(column);
    }
    return QVariant();
}

int AbstractIndx::columnCount()
{
    return _data.count();
}

AbstractIndx::IndxType AbstractIndx::type()
{
    return _type;
}

void AbstractIndx::setData( const QVariantList& list )
{
    _data = list;
}

int AbstractIndx::year()
{
    return _date.year();
}

int AbstractIndx::month()
{
    return _date.month();
}

/* ================================================================== */

class YearIndx : public AbstractIndx
{
public:
    explicit YearIndx(int year)
        :AbstractIndx(IndxType::YearType, QDate(year, 1, 1)) {
        _data << QString::number(year);
    }
};

/* ================================================================== */

class MonthIndx : public AbstractIndx
{
public:
    explicit MonthIndx(int year, int month)
        :AbstractIndx(IndxType::MonthType, QDate(year, month, 1)) {

        _data << QDate::longMonthName(month);
    }
};

/* ================================================================== */

DateModel::DateModel(QObject *parent)
    :QAbstractItemModel(parent)
{
    rootItem = new TreeItem(NULL);

}

int DateModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return _columnCount;
}

void DateModel::setHeaderStrings( const QStringList& headers)
{
    _headers = headers;
}

QVariant DateModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role == Qt::DisplayRole && orientation == Qt::Horizontal && section < _headers.count() ) {
        return _headers.at(section);
    }
    return QVariant();
}

QVariant DateModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    AbstractIndx *indx = item->payload();

    if( role == Qt::BackgroundColorRole ) {
        if( indx->type() == AbstractIndx::YearType ) {
            return QColor(0x80, 0xC8, 0xFE);
        } else if( indx->type() == AbstractIndx::MonthType ) {
            return QColor(0xBF, 0xE3, 0xFE);
        }
        return QVariant();
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    if( indx->type() == AbstractIndx::YearType ) {
        int col = index.column();
        QList<TreeItem*> monthItems = item->children();
        if( _yearExtra[col] == Sum ) {
            float sum = 0.0;
            foreach( TreeItem *month, monthItems) {
                foreach( TreeItem *docItem, month->children()) {
                    sum += docItem->payload()->data(col).toFloat();
                }
            }
            return sum;
        } else if( _monthExtra[col] == Count ) {
            int cnt = 0;
            foreach( TreeItem *month, monthItems) {
                cnt += month->children().count();
            }
            return cnt;
        } else {
            return item->payload()->data(col);
        }
    }

    if( indx->type() == AbstractIndx::MonthType ) {
        // there might be a special column type
        int col = index.column();
        QList<TreeItem*> childitems = item->children();
        if( _monthExtra[col] == Sum ) {
            float sum = 0.0;
            foreach( TreeItem *child, childitems) {
                sum += child->payload()->data(col).toFloat();
            }
            return sum;
        } else if( _monthExtra[col] == Count ) {
            return childitems.count();
        } else {
            return item->payload()->data(col);
        }
    }

    if( indx->type() == AbstractIndx::DocumentType ) {
        return item->payload()->data(index.column());
    }
    return QVariant();
}

Qt::ItemFlags DateModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}


QModelIndex DateModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
         return QModelIndex();

     TreeItem *parentItem;

     if (!parent.isValid())
         parentItem = rootItem;
     else
         parentItem = static_cast<TreeItem*>(parent.internalPointer());

     TreeItem *childItem = parentItem->child(row);
     if (childItem)
         return createIndex(row, column, childItem);
     else
         return QModelIndex();
}

QModelIndex DateModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int DateModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem;
     if (parent.column() > 0)
         return 0;

     if (!parent.isValid())
         parentItem = rootItem;
     else
         parentItem = static_cast<TreeItem*>(parent.internalPointer());

     return parentItem->childCount();
}

void DateModel::setColumnCount(int columns)
{
    if( columns > 0 ) {
        _columnCount = columns;
        _monthExtra.resize(columns);
        _yearExtra.resize(columns);
    }
}

void DateModel::setMonthSumColumn( int column )
{
    if(column < _columnCount) {
        _monthExtra[column] = Sum;
    }
}

void DateModel::setMonthCountColumn( int column )
{
    if( column < _columnCount ) {
        _monthExtra[column] = Count;
    }
}

void DateModel::setYearSumColumn( int column )
{
    if(column < _columnCount) {
        _yearExtra[column] = Sum;
    }
}

void DateModel::setYearCountColumn( int column )
{
    if( column < _columnCount ) {
        _yearExtra[column] = Count;
    }
}

void DateModel::setYearCountColumn( int column );

TreeItem *DateModel::findYearItem(int year)
{
    TreeItem *yearItem = NULL;

    QList<TreeItem*> yearitems = rootItem->children();
    foreach( TreeItem *item, yearitems ) {
        AbstractIndx *indx = item->payload();
        if( indx->year() == year ) {
            yearItem = item;
            break;
        }
    }
    return yearItem;
}

TreeItem *DateModel::findMonthItem(int year, int month)
{
    TreeItem *monthItem = NULL;
    TreeItem *yearItem = findYearItem( year );

    if( yearItem ) {
        QList<TreeItem*> monthItems = yearItem->children();
        foreach( TreeItem *item, monthItems ) {
            AbstractIndx *indx = static_cast<AbstractIndx*>(item->payload());
            if( indx->month() == month ) {
                monthItem = item;
                break;
            }
        }
    }
    return monthItem;
}

void DateModel::addData( DocumentIndx doc )
{
    int month = doc.month();
    int year = doc.year();

    TreeItem *yearItem = NULL;
    TreeItem *monthItem = NULL;

    yearItem = findYearItem( year );

    if( !yearItem ) {
        AbstractIndx *newIndx = new YearIndx(year);
        yearItem = new TreeItem( newIndx, rootItem );
    }

    // ====
    monthItem = findMonthItem( year, month );

    if( !monthItem ) {
        AbstractIndx *newIndx = new MonthIndx(year, month);
        monthItem = new TreeItem( newIndx, yearItem);
    }

    DocumentIndx *itemIndx = new DocumentIndx(doc);
    TreeItem *newItem = new TreeItem( itemIndx, monthItem );

    Q_UNUSED(newItem);

}

int DateModel::fromTable()
{
    int cnt = 0;

    QSqlQuery query;
    query.prepare("SELECT ident, docType, docDescription, clientID, "
                    "lastModified, date, projectLabel, clientAddress "
                    "FROM document ORDER BY date DESC");
    query.exec();

    while (query.next()) {
        const QDate date(query.value(5).toDate());
        if( date.isValid()) {
            DocumentIndx doc(date);
            QVariantList vl;
            vl.insert(0, QLatin1String(""));
            vl.insert(1, query.value(0).toString()); // docID
            vl.insert(2, query.value(1).toString()); //column 2
            vl.insert(3, query.value(2).toString());
            vl.insert(4, query.value(3).toString());
            vl.insert(5, query.value(4).toString());
            vl.insert(6, query.value(7).toString());

            doc.setData(vl);
            this->addData( doc );
            cnt++;
        }
    }
    return cnt;
}
