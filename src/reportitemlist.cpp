/***************************************************************************
                 reportitemlist.cpp - report item list
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
#include "reportitemlist.h"
#include "docposition.h"
#include "format.h"

ReportItemList::ReportItemList(const DocPositionList& positions)
    :QList<ReportItem*>()
{
    for (DocPositionBase *dp: positions) {
        ReportItem *ri = new ReportItem(dp);
        append(ri);
    }
}

ReportItemList::~ReportItemList()
{
    qDeleteAll(begin(), end());
}
