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

#include "docbasemodel.h"
#include "docdigest.h"


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

    explicit AbstractIndx();
    virtual ~AbstractIndx() { }

    explicit AbstractIndx(IndxType t);

    explicit AbstractIndx(IndxType t, DocDigest(digest));

    virtual IndxType type();

    DocDigest digest() const;

    int year();
    int month();

protected:
    DocDigest    _docDigest;

private:
    IndxType     _type;
};

/* ================================================================== */

class DocumentIndx : public AbstractIndx
{
public:
    DocumentIndx(const DocDigest& digest)
        :AbstractIndx(IndxType::DocumentType, digest) {
    }
};

/* ================================================================== */

class DateModel : public DocBaseModel
{
public:
    DateModel(QObject *parent = 0);
    enum CalcType {
        Zero = 0,
        Sum,
        Count
    };

    TreeItem* findYearItem(int year);
    TreeItem* findMonthItem(int year, int month);

    void setMonthSumColumn( int column );
    void setMonthCountColumn( int column );
    void setYearSumColumn( int column );
    void setYearCountColumn( int column );

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &index) const;
    DocDigest digest(const QModelIndex& indx) const;

    int rowCount(const QModelIndex &parent) const;

    void removeAllData();
    void addData(const DocDigest& digest);

    bool isDocument(const QModelIndex& indx) const;

private:
    TreeItem          *rootItem;
    QVector<CalcType> _monthExtra;
    QVector<CalcType> _yearExtra;
};

#endif // DATEMODEL_H
