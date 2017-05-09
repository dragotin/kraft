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

#ifndef DATEMODEL_H
#define DATEMODEL_H

#include <QAbstractItemModel>
#include <QDate>
#include <QList>
#include <QVector>
#include <QDebug>
#include <QStringList>


class TreeItem;

class AbstractIndx
{
public:
    enum IndxType {
        Invalid  = 0,
        DocumentType,
        YearType,
        MonthType
    };

    AbstractIndx()
        :_type(Invalid) { }

    explicit AbstractIndx(IndxType t, QDate d)
        :_type(t), _date(d) { }

    virtual QVariant data(int column) const;

    virtual IndxType type();

    virtual int columnCount();

    void setData( const QVariantList& list );

    int year();
    int month();

protected:
    QVariantList _data;

private:
    IndxType     _type;
    QDate        _date;
};

/* ================================================================== */

class DocumentIndx : public AbstractIndx
{
public:
    DocumentIndx( QDate d)
        :AbstractIndx(IndxType::DocumentType, d) {
    }
};

/* ================================================================== */

class DateModel : public QAbstractItemModel
{
public:
    DateModel(QObject *parent = 0);
    enum CalcType {
        Zero = 0,
        Sum,
        Count
    };

    void setColumnCount(int columns);
    void setHeaderStrings( const QStringList& headers);

    TreeItem* findYearItem(int year);
    TreeItem* findMonthItem(int year, int month);

    void setMonthSumColumn( int column );
    void setMonthCountColumn( int column );
    void setYearSumColumn( int column );
    void setYearCountColumn( int column );

    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;

    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent) const;

    void addData(DocumentIndx doc);

    int fromTable();
protected:
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    TreeItem          *rootItem;
    int               _columnCount;
    QVector<CalcType> _monthExtra;
    QVector<CalcType> _yearExtra;
    QStringList       _headers;

};

#endif // DATEMODEL_H
