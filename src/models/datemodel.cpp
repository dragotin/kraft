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
#include "docdigest.h"

#include <QStringList>
#include <QColor>
#include <QSqlQuery>
#include <QFont>

class TreeItem
{
public:
    TreeItem(AbstractIndx *indx, TreeItem *parent = 0);
    ~TreeItem();

    void appendChild(TreeItem *child);

    TreeItem *child(int row);
    int childCount() const;
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

TreeItem::~TreeItem()
{
    foreach( TreeItem *i, childItems ) {
        delete i;
    }
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
AbstractIndx::AbstractIndx()
    :_type(Invalid)
{

}

AbstractIndx::AbstractIndx(IndxType t)
    :_type(t)
{

}

AbstractIndx::AbstractIndx(IndxType t, DocDigest(digest))
    :_docDigest(digest), _type(t)
{

}


AbstractIndx::IndxType AbstractIndx::type()
{
    return _type;
}

DocDigest AbstractIndx::digest() const
{
    return _docDigest;
}

int AbstractIndx::year()
{
    return _docDigest.rawDate().year();
}

int AbstractIndx::month()
{
    return _docDigest.rawDate().month();
}

/* ================================================================== */

class YearIndx : public AbstractIndx
{
public:
    explicit YearIndx(int year)
        :AbstractIndx(IndxType::YearType) {
         QDate d (year, 1, 1);
         _docDigest.setDate(d);
    }
};

/* ================================================================== */

class MonthIndx : public AbstractIndx
{
public:
    explicit MonthIndx(int year, int month)
        :AbstractIndx(IndxType::MonthType) {
        QDate d(year, month, 1);
        _docDigest.setDate(d);
    }
};

/* ================================================================== */

DateModel::DateModel(QObject *parent)
    :DocBaseModel(parent)
{
    rootItem = new TreeItem(0);
}

QVariant DateModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    AbstractIndx *indx = item->payload();
#if 0
    if( role == Qt::BackgroundColorRole ) {
        if( indx->type() == AbstractIndx::YearType ) {
            return QColor(0x80, 0xC8, 0xFE);
        } else if( indx->type() == AbstractIndx::MonthType ) {
            return QColor(0xBF, 0xE3, 0xFE);
        }
        return QVariant();
    }
#endif
    if( role == Qt::FontRole ) {
        QFont f;
        if( indx->type() == AbstractIndx::YearType ) {
            f.setPointSize(22);
            return f;
        } else if( indx->type() == AbstractIndx::MonthType ) {
            f.setPointSize(16);
            return f;
        }
    }

    if (role != Qt::DisplayRole)
        return QVariant();
    int col = index.column();

    if( indx->type() == AbstractIndx::YearType ) {
        if( col == 0 ) {
            return item->payload()->year();
        } else if(col == Treestruct_Year) {
            return item->payload()->year();
        } else if(col == Treestruct_Type) {
            return AbstractIndx::YearType;
        }

#if 0
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
#endif
    }

    if( indx->type() == AbstractIndx::MonthType ) {
        // there might be a special column type
        if( col == 0 ) {
            return QDate::shortMonthName(item->payload()->month());
        } else if(col == Treestruct_Month) {
            return item->payload()->month();
        } else if(col == Treestruct_Year) {
            return item->payload()->year();
        } else if(col == Treestruct_Type) {
            return AbstractIndx::MonthType;
        }
#if 0
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
        }
#endif
    }

    if( indx->type() == AbstractIndx::DocumentType ) {
        DocDigest digest = item->payload()->digest();
        if( index.column() == Document_Id ) {
            // In column zero the day has to be displayed here
            const QDate date = digest.rawDate();
            return date.day();
        } else {
            return columnValueFromDigest( digest, index.column() );
        }
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

void DateModel::setMonthSumColumn( int column )
{
    if(column < columnCount( QModelIndex() )) {
         _monthExtra[column] = Sum;
    }
}

void DateModel::setMonthCountColumn( int column )
{
    if( column < columnCount(QModelIndex()) ) {
        _monthExtra[column] = Count;
    }
}

void DateModel::setYearSumColumn( int column )
{
    if(column < columnCount(QModelIndex())) {
        _yearExtra[column] = Sum;
    }
}

void DateModel::setYearCountColumn( int column )
{
    if( column < columnCount(QModelIndex()) ) {
        _yearExtra[column] = Count;
    }
}

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

bool DateModel::isDocument(const QModelIndex& indx) const
{
    bool re = false;
    if( indx.isValid() ) {
        TreeItem *item = static_cast<TreeItem*>(indx.internalPointer());
        if( item ) {
            AbstractIndx *abstractindx = item->payload();

            re = !(abstractindx->type() == AbstractIndx::YearType ||
                    abstractindx->type() == AbstractIndx::MonthType );
        }
    }
    return re;
}

DocDigest DateModel::digest(const QModelIndex& indx) const
{
    DocDigest dig;

    TreeItem *item = static_cast<TreeItem*>(indx.internalPointer());

    AbstractIndx *abstractindx = item->payload();

    if( abstractindx->type() == AbstractIndx::YearType ||
            abstractindx->type() == AbstractIndx::MonthType ) {
        // there is no digest
    } else {
        dig = abstractindx->digest();
    }
    return dig;
}

void DateModel::removeAllData()
{
    // the destructor of the TreeItem removes the entire tree recursivly
    delete rootItem;
    rootItem = new TreeItem(0);
}

void DateModel::addData( const DocDigest& digest ) // DocumentIndx doc )
{
    int month = digest.rawDate().month();
    int year = digest.rawDate().year();

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

    DocumentIndx *itemIndx = new DocumentIndx(digest);
    TreeItem *newItem = new TreeItem( itemIndx, monthItem );

    Q_UNUSED(newItem);

}

