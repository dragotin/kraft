/***************************************************************************
        vatreportwindow.h - window that shows the VAT per tax period
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

#ifndef VATREPORTWINDOW_H
#define VATREPORTWINDOW_H

#include "htmlwindow.h"
#include "vatreport.h"

class QTabWidget;
class VatReportTreeView;

/**
 * Non modal window that shows the VatReport of one year in two tabs: a html
 * page rendered with the views/vatreport.gtmpl template and a tree view with
 * quarters, months and invoices. Links in the page switch the year, the
 * period type and open documents. The tree always shows the same year.
 */
class VatReportWindow : public HtmlWindow
{
    Q_OBJECT
public:
    explicit VatReportWindow(QWidget *parent = nullptr);

    // renders the report with the views/vatreport.gtmpl template
    static QString reportHtml(const VatReport &report, int year);

public Q_SLOTS:
    void showYear(int year);

Q_SIGNALS:
    void openDocument(const QString &uuid);

private Q_SLOTS:
    void slotUrlClicked(const QUrl &url);

private:
    void render();

    int _year;
    VatReport::PeriodType _type;
    QTabWidget *_tabs;
    VatReportTreeView *_treeView;
};

#endif // VATREPORTWINDOW_H
