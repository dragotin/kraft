/***************************************************************************
                 reportitem.h  - report item
                             -------------------
    begin                : August 2023
    copyright            : (C) 2023 by Klaas Freitag
    email                : kraft@freisturz.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef REPORTITEM_H
#define REPORTITEM_H

// include files for Qt
#include <QObject>
#include <QString>
#include <QList>

/**
@author Klaas Freitag
*/

class DocPositionBase;

class ReportItem: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString kind READ getKind)
    Q_PROPERTY(QString itemNumber READ itemNumber)
    Q_PROPERTY(QString text READ getText)
    Q_PROPERTY(QString htmlText READ htmlText)
    Q_PROPERTY(QString amount READ amount)
    Q_PROPERTY(QString unit READ unit)
    Q_PROPERTY(QString unitPrice READ unitPrice)
    Q_PROPERTY(QString nettoPrice READ nettoPrice)
    Q_PROPERTY(QString taxMarker READ taxMarker)

public:
    ReportItem() : QObject() {}
    ReportItem(DocPositionBase *dpb);

    QString getText() { return _text; }
    QString itemNumber() { return QString::number(_itemNo); }
    QString getKind() { return _kind; }
    QString htmlText();
    QString amount() { return _amount; }
    QString unit() { return _unit; }
    QString nettoPrice() { return _nettoPrice; }
    QString unitPrice() { return _unitPrice; }
    QString taxMarker() { return _taxMarker; }

private:
    int     _itemNo;
    QString _uuid;
    QString _kind;
    QString _text;
    QString _amount;
    QString _unit;
    QString _unitPrice;
    QString _nettoPrice;
    QString _taxMarker;
};

#endif
