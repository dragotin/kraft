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

#include <KTextTemplate/Template>
#include <KTextTemplate/MetaType>

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

    // The list owns its elements and deletes them in the destructor. As
    // ReportItem is a QObject it cannot be copied, so make the list move-only:
    // copying would shallow-copy the owned pointers and double-free them.
    ReportItemList(const ReportItemList&) = delete;
    ReportItemList& operator=(const ReportItemList&) = delete;

    ReportItemList(ReportItemList&& other) noexcept = default;
    ReportItemList& operator=(ReportItemList&& other) noexcept;
};

// Read-only introspection of ReportItemList object.
KTEXTTEMPLATE_BEGIN_LOOKUP(ReportItemList)
if ( property == "count" ) {
    return object.count();
} else {
    return QStringLiteral("undefined");
}
KTEXTTEMPLATE_END_LOOKUP

#endif
