/***************************************************************************
                 reportitemlist.h  - report item list
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
#ifndef REPORTITEMLIST_H
#define REPORTITEMLIST_H

#include <grantlee/metatype.h>

#include "reportitem.h"
#include "docposition.h"

// include files for Qt
#include <QObject>
#include <QList>

/**
@author Klaas Freitag
*/
class QString;
class DocPosition;

class ReportItemList : public QList<ReportItem*>
{
public:
    ReportItemList() : QList<ReportItem*>() {}
    ReportItemList(const DocPositionList& positions);

    ~ReportItemList();
};

// Read-only introspection of ReportItemList object.

GRANTLEE_BEGIN_LOOKUP(ReportItemList)
if ( property == "count" ) {
    return object.count();
} else {
    return QStringLiteral("undefined");
}
GRANTLEE_END_LOOKUP

#endif
