/***************************************************************************
     vatreporttreeview.cpp - tree view on the VAT of a year
                             -------------------
    begin                : Oct. 2026
    copyright            : (C) 2026 by Klaas Freitag
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

#include <QApplication>
#include <QHeaderView>

#include "vatreporttreeview.h"
#include "models/vatreportmodel.h"
#include "vatreport.h"

VatReportTreeView::VatReportTreeView(QWidget *parent)
    : QTreeView(parent)
    , _model(new VatReportModel(this))
{
    setModel(_model);
    setAlternatingRowColors(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setUniformRowHeights(true);
    header()->setStretchLastSection(false);

    connect(this, &QTreeView::doubleClicked, this, &VatReportTreeView::slotDoubleClicked);
}

QString VatReportTreeView::currentDocumentUuid() const
{
    return _model->uuid(currentIndex());
}

void VatReportTreeView::showYear(int year)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    VatReport report(VatReport::PeriodType::Month);
    report.loadYear(year);
    QApplication::restoreOverrideCursor();

    showReport(report, year);
}

void VatReportTreeView::showReport(const VatReport &report, int year)
{
    _model->setReport(report, year);

    // resizeColumnToContents only measures visible rows, so size the columns
    // with everything expanded and then show only the months of the quarters.
    expandAll();
    for (int col = 0; col < _model->columnCount(); col++) {
        resizeColumnToContents(col);
    }
    collapseAll();
    for (int row = 0; row < _model->rowCount(); row++) {
        expand(_model->index(row, 0));
    }
}

void VatReportTreeView::slotDoubleClicked(const QModelIndex &index)
{
    const QString uuid = _model->uuid(index);
    if (!uuid.isEmpty()) {
        Q_EMIT openDocument(uuid);
    }
}
