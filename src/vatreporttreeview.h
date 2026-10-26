/***************************************************************************
      vatreporttreeview.h - tree view on the VAT of a year
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

#ifndef VATREPORTTREEVIEW_H
#define VATREPORTTREEVIEW_H

#include <QTreeView>

class VatReport;
class VatReportModel;

/**
 * Shows the quarters, months and invoices of one year with their VAT sums.
 * A double click on an invoice emits openDocument().
 */
class VatReportTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit VatReportTreeView(QWidget *parent = nullptr);

    VatReportModel *vatModel() const { return _model; }

    // uuid of the currently selected invoice, empty if a period is selected
    QString currentDocumentUuid() const;

public Q_SLOTS:
    // loads the invoices of the year from the document index
    void showYear(int year);
    // shows an already loaded report, ie. for testing
    void showReport(const VatReport &report, int year);

Q_SIGNALS:
    void openDocument(const QString &uuid);

private Q_SLOTS:
    void slotDoubleClicked(const QModelIndex &index);

private:
    VatReportModel *_model;
};

#endif // VATREPORTTREEVIEW_H
