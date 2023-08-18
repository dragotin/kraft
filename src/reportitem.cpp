/***************************************************************************
                 reportitem.cpp - report item
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
#include "reportitem.h"
#include "docposition.h"
#include "format.h"


ReportItem::ReportItem(DocPositionBase *dpb)
    : QObject()
{
//    int     _itemNo;
//    QString _kind;
//    QString _text;
//    QString _amount;
//    QString _unit;
//    QString _unitPrice;
//    QString _nettoPrice;
//    QString _taxMarker;

    DocPosition *dp = static_cast<DocPosition*>(dpb);
    _kind = dp->typeStr();
    _text = dp->text();
    _itemNo = dp->positionNumber();
    _amount = Format::localeDoubleToString(dp->amount());
    _unitPrice = dp->unitPrice().toLocaleString();
    _nettoPrice = dp->overallPrice().toLocaleString();

    QString re;
    DocPositionBase::TaxType tt = dp->taxType();
    if ( tt == DocPositionBase::TaxReduced ) {
        re = QStringLiteral("1");
    } else if ( tt == DocPositionBase::TaxNone) {
        re = QStringLiteral("");
    } else if (tt == DocPositionBase::TaxFull) {
        re = QStringLiteral("2");
    }

    _unit = dp->unit().einheit(_amount > 1 ? 2 : 1);
    _taxMarker = re;
    _uuid = dp->uuid();
}

QString ReportItem::htmlText()
{
    const QStringList li = _text.toHtmlEscaped().split( "\n", QString::KeepEmptyParts );
    QString re = li.join("<br/>");

    return re;
}
