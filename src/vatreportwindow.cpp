/***************************************************************************
       vatreportwindow.cpp - window that shows the VAT per tax period
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
#include <QDebug>
#include <QLayout>
#include <QTabWidget>
#include <QUrlQuery>

#include <klocalizedstring.h>

#include "vatreportwindow.h"
#include "vatreporttreeview.h"
#include "htmlview.h"
#include "defaultprovider.h"
#include "format.h"
#include "grantleetemplate.h"
#include "kraftsettings.h"

namespace {

QString moneyStr(const Geld &g)
{
    return g.toLocaleString();
}

QString dateStr(const QDate &d)
{
    return Format::toDateString(d, KraftSettings::self()->dateFormat());
}

QString rateStr(double rate)
{
    return i18nc("tax rate in percent", "%1 %", QLocale().toString(rate, 'g', 4));
}

void setSumProperties(QObject *obj, const VatReport::PeriodSum &sum)
{
    obj->setProperty("netFull", moneyStr(sum.netFull));
    obj->setProperty("taxFull", moneyStr(sum.taxFull));
    obj->setProperty("netReduced", moneyStr(sum.netReduced));
    obj->setProperty("taxReduced", moneyStr(sum.taxReduced));
    obj->setProperty("netNoTax", moneyStr(sum.netNoTax));
    obj->setProperty("taxSum", moneyStr(sum.taxSum()));
    obj->setProperty("docCount", QString::number(sum.docs.size()));
    obj->setProperty("hasDocs", !sum.docs.isEmpty());
}

QObject *docObject(const VatReport::DocEntry &e, QObject *parent)
{
    QObject *obj = new QObject(parent);
    obj->setProperty("uuid", e.uuid);
    obj->setProperty("ident", e.ident);
    obj->setProperty("client", e.clientAddress.section(QLatin1Char('\n'), 0, 0).trimmed());
    obj->setProperty("invoiceDate", dateStr(e.invoiceDate));
    obj->setProperty("taxDate", dateStr(e.taxDate));
    obj->setProperty("dateFromInvoice", e.dateFromInvoice);
    obj->setProperty("retracted", e.state == KraftDocState::State::Retracted);
    obj->setProperty("netFull", moneyStr(e.netFull));
    obj->setProperty("taxFull", moneyStr(e.taxFull));
    obj->setProperty("netReduced", moneyStr(e.netReduced));
    obj->setProperty("taxReduced", moneyStr(e.taxReduced));
    obj->setProperty("netNoTax", moneyStr(e.netNoTax));
    obj->setProperty("hasUnknownTax", e.netUnknownTax.toLong() != 0);
    obj->setProperty("netUnknownTax", moneyStr(e.netUnknownTax));
    return obj;
}

} // namespace

VatReportWindow::VatReportWindow(QWidget *parent)
    : HtmlWindow(i18n("VAT Overview"), QStringLiteral("vatreportwindow"), parent)
    , _year(QDate::currentDate().year())
    , _type(KraftSettings::self()->vatReportQuarterly() ? VatReport::PeriodType::Quarter : VatReport::PeriodType::Month)
{
    const QByteArray geo = QByteArray::fromBase64(KraftSettings::self()->vatReportWinGeometry().toLatin1());
    if (!geo.isEmpty()) {
        restoreGeometry(geo);
    } else {
        resize(900, 700);
    }
    connect(this, &HtmlWindow::closing, this, [this]() {
        KraftSettings::self()->setVatReportWinGeometry(QString::fromLatin1(saveGeometry().toBase64()));
        KraftSettings::self()->save();
    });
    connect(this, &HtmlWindow::kraftUrlClicked, this, &VatReportWindow::slotUrlClicked);

    setStylesheetFile(QStringLiteral("vatreport.css"));

    // Put the html view of the base class into the first tab, the tree into the second
    _tabs = new QTabWidget(this);
    layout()->replaceWidget(htmlView(), _tabs);
    _tabs->addTab(htmlView(), i18n("Report"));

    _treeView = new VatReportTreeView(_tabs);
    _tabs->addTab(_treeView, i18n("Quarters and Months"));
    connect(_treeView, &VatReportTreeView::openDocument, this, &VatReportWindow::openDocument);
}

void VatReportWindow::showYear(int year)
{
    _year = year;
    render();
}

void VatReportWindow::slotUrlClicked(const QUrl &url)
{
    const QUrlQuery q(url);
    const QString path = url.path();

    if (path == QLatin1String("year")) {
        bool ok;
        const int y = q.queryItemValue(QStringLiteral("y")).toInt(&ok);
        if (ok) {
            showYear(y);
        }
    } else if (path == QLatin1String("period")) {
        const bool quarterly = q.queryItemValue(QStringLiteral("type")) == QLatin1String("quarter");
        _type = quarterly ? VatReport::PeriodType::Quarter : VatReport::PeriodType::Month;
        KraftSettings::self()->setVatReportQuarterly(quarterly);
        KraftSettings::self()->save();
        render();
    } else if (path == QLatin1String("document")) {
        const QString uuid = q.queryItemValue(QStringLiteral("uuid"));
        if (!uuid.isEmpty()) {
            Q_EMIT openDocument(uuid);
        }
    } else {
        qDebug() << "Unknown link in VAT report:" << url;
    }
}

void VatReportWindow::render()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    VatReport report(_type);
    report.loadYear(_year);
    QApplication::restoreOverrideCursor();

    setHtml(reportHtml(report, _year));
    _treeView->showReport(report, _year);
}

QString VatReportWindow::reportHtml(const VatReport &report, int year)
{
    const QString tmplFile = DefaultProvider::self()->locateFile(QStringLiteral("views/vatreport.gtmpl"));
    GrantleeFileTemplate tmpl(tmplFile);
    if (!tmpl.isOk()) {
        return i18n("Template file views/vatreport.gtmpl could not be found.");
    }

    // owns all objects created for the template, deleted when leaving
    QObject holder;

    const VatReport::PeriodSum total = report.total();

    // The rates are taken from the documents. Name them in the header only if
    // they are the same for all invoices of the year.
    QSet<double> fullRates, reducedRates;
    int fromInvoiceCnt{0};
    int retractedCnt{0};
    Geld unknownTax;
    for (const auto &e : total.docs) {
        if (e.netFull.toLong() != 0)
            fullRates.insert(e.fullTaxRate);
        if (e.netReduced.toLong() != 0)
            reducedRates.insert(e.reducedTaxRate);
        if (e.dateFromInvoice)
            fromInvoiceCnt++;
        if (e.state == KraftDocState::State::Retracted)
            retractedCnt++;
        unknownTax += e.netUnknownTax;
    }

    QList<QObject *> periodList;
    const auto periods = report.periodsOfYear(year);
    for (const auto &p : periods) {
        QObject *pObj = new QObject(&holder);
        pObj->setProperty("label", p.label());
        pObj->setProperty("anchor", QStringLiteral("p%1").arg(p.period));
        pObj->setProperty("range", i18nc("date range of a tax period", "%1 – %2", dateStr(p.firstDay()), dateStr(p.lastDay())));
        setSumProperties(pObj, p);

        QList<QObject *> docList;
        for (const auto &e : p.docs) {
            docList.append(docObject(e, &holder));
        }
        pObj->setProperty("docs", QVariant::fromValue(docList));
        periodList.append(pObj);
    }

    const bool quarterly = (report.periodType() == VatReport::PeriodType::Quarter);
    const int currentYear = QDate::currentDate().year();

    QObject kraft;
    kraft.setProperty("year", QString::number(year));
    kraft.setProperty("prevYear", QString::number(year - 1));
    kraft.setProperty("nextYear", QString::number(year + 1));
    kraft.setProperty("hasNextYear", year < currentYear);
    kraft.setProperty("quarterly", quarterly);
    kraft.setProperty("periods", QVariant::fromValue(periodList));
    kraft.setProperty("hasDocs", !total.docs.isEmpty());
    kraft.setProperty("fromInvoiceCnt", fromInvoiceCnt);
    kraft.setProperty("retractedCnt", retractedCnt);
    kraft.setProperty("hasUnknownTax", unknownTax.toLong() != 0);
    kraft.setProperty("unknownTax", moneyStr(unknownTax));

    QObject totalObj;
    setSumProperties(&totalObj, total);

    QObject labels;
    labels.setProperty("headline", i18n("VAT from Invoices %1", QString::number(year)));
    labels.setProperty("intro", i18n("VAT contained in the final invoices of the year, assigned to the tax period by the end of the time of supply "
                                     "(taxation on agreed remuneration, Sollversteuerung)."));
    labels.setProperty("noInputTax", i18n("Input tax is not taken into account. The figures are not the tax to be paid and not a tax advice."));
    labels.setProperty("monthly", i18n("Monthly"));
    labels.setProperty("quarterly", i18n("Quarterly"));
    labels.setProperty("period", quarterly ? i18n("Quarter") : i18n("Month"));
    labels.setProperty("netFull", fullRates.size() == 1 ? i18n("Net %1", rateStr(*fullRates.cbegin())) : i18n("Net full rate"));
    labels.setProperty("taxFull", fullRates.size() == 1 ? i18n("VAT %1", rateStr(*fullRates.cbegin())) : i18n("VAT full rate"));
    labels.setProperty("netReduced", reducedRates.size() == 1 ? i18n("Net %1", rateStr(*reducedRates.cbegin())) : i18n("Net reduced rate"));
    labels.setProperty("taxReduced", reducedRates.size() == 1 ? i18n("VAT %1", rateStr(*reducedRates.cbegin())) : i18n("VAT reduced rate"));
    labels.setProperty("netNoTax", i18n("Net without VAT"));
    labels.setProperty("taxSum", i18n("VAT total"));
    labels.setProperty("invoices", i18n("Invoices"));
    labels.setProperty("total", i18n("Year %1", QString::number(year)));
    labels.setProperty("details", i18n("Invoices per period"));
    labels.setProperty("ident", i18n("Invoice"));
    labels.setProperty("invoiceDate", i18n("Date"));
    labels.setProperty("taxDate", i18n("Supply"));
    labels.setProperty("client", i18n("Client"));
    labels.setProperty("noDocs", i18n("There are no final invoices for this year."));
    labels.setProperty("noDocsPeriod", i18n("No invoices in this period."));
    labels.setProperty("fromInvoiceMarker", i18nc("marker for invoices without time of supply", "*"));
    labels.setProperty("fromInvoice", i18np("One invoice has no time of supply, the invoice date was used instead (marked with *).",
                                            "%1 invoices have no time of supply, the invoice date was used instead (marked with *).",
                                            fromInvoiceCnt));
    labels.setProperty("retracted", i18np("One retracted invoice is included, as the VAT shown on it is owed until it is corrected (§ 14c UStG).",
                                          "%1 retracted invoices are included, as the VAT shown on them is owed until they are corrected (§ 14c UStG).",
                                          retractedCnt));
    labels.setProperty("retractedMarker", i18n("retracted"));
    labels.setProperty("unknownTax", i18n("Items with an undefined tax type are not included in the sums:"));

    tmpl.addToObjMapping(QStringLiteral("kraft"), &kraft);
    tmpl.addToObjMapping(QStringLiteral("total"), &totalObj);
    tmpl.addToObjMapping(QStringLiteral("label"), &labels);

    bool ok;
    return tmpl.render(ok);
}
