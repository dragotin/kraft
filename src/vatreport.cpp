/***************************************************************************
         vatreport.cpp - Sum of the VAT of invoices per tax period
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

#include <QDebug>
#include <QLocale>

#include <KLocalizedString>

#include "vatreport.h"
#include "docdigest.h"
#include "docposition.h"
#include "documentman.h"
#include "unitmanager.h"
#include "xmlindexdb.h"

void VatReport::PeriodSum::add(const DocEntry &e)
{
    netFull += e.netFull;
    taxFull += e.taxFull;
    netReduced += e.netReduced;
    taxReduced += e.taxReduced;
    netNoTax += e.netNoTax;
    netUnknownTax += e.netUnknownTax;
}

Geld VatReport::PeriodSum::taxSum() const
{
    Geld g{taxFull};
    g += taxReduced;
    return g;
}

QDate VatReport::PeriodSum::firstDay() const
{
    if (type == PeriodType::Quarter) {
        return QDate(year, 3 * (period - 1) + 1, 1);
    }
    return QDate(year, period, 1);
}

QDate VatReport::PeriodSum::lastDay() const
{
    const int months = (type == PeriodType::Quarter) ? 3 : 1;
    return firstDay().addMonths(months).addDays(-1);
}

QString VatReport::PeriodSum::label() const
{
    if (type == PeriodType::Quarter) {
        return i18nc("Tax period label, %1 is the quarter, %2 the year", "Q%1/%2", period, QString::number(year));
    }
    return QStringLiteral("%1 %2").arg(QLocale().standaloneMonthName(period), QString::number(year));
}

// ##############################################################

VatReport::VatReport(PeriodType type)
    : _type(type)
{
}

bool VatReport::countsForVat(KraftDocState::State state)
{
    // Retracted invoices were sent out to the customer. The VAT shown on them is
    // owed until the invoice is corrected (§ 14c UStG), so they count.
    return state == KraftDocState::State::Final || state == KraftDocState::State::Retracted;
}

bool VatReport::countsForVat(const DocDigest &digest)
{
    return digest.isInvoice() && countsForVat(digest.state().state());
}

QDate VatReport::taxDate(KraftDoc *doc, bool *fromInvoiceDate)
{
    QDate d;
    if (doc) {
        if (doc->timeOfSupplyEnd().isValid()) {
            d = doc->timeOfSupplyEnd().date();
        } else if (doc->timeOfSupplyStart().isValid()) {
            d = doc->timeOfSupplyStart().date();
        }
    }
    if (fromInvoiceDate) {
        *fromInvoiceDate = !d.isValid();
    }
    if (!d.isValid() && doc) {
        d = doc->date();
    }
    return d;
}

VatReport::DocEntry VatReport::entryFromDoc(KraftDoc *doc)
{
    DocEntry e;
    if (!doc)
        return e;

    e.uuid = doc->uuid();
    e.ident = doc->ident();
    e.clientAddress = doc->address();
    e.invoiceDate = doc->date();
    e.taxDate = taxDate(doc, &e.dateFromInvoice);
    e.state = doc->state().state();

    // Use the tax rates stored in the document, as they were printed on the invoice.
    e.fullTaxRate = doc->fullTax() < 0 ? UnitManager::self()->tax(doc->date()) : doc->fullTax();
    e.reducedTaxRate = doc->reducedTax() < 0 ? UnitManager::self()->reducedTax(doc->date()) : doc->reducedTax();

    const DocPositionList positions = doc->positions();
    for (DocPosition *dp : positions) {
        if (dp->toDelete())
            continue;
        const Geld price = dp->overallPrice();
        switch (dp->taxType()) {
        case DocPosition::Tax::Full:
            e.netFull += price;
            break;
        case DocPosition::Tax::Reduced:
            e.netReduced += price;
            break;
        case DocPosition::Tax::None:
            e.netNoTax += price;
            break;
        default:
            e.netUnknownTax += price;
            break;
        }
    }

    // Same rounding as on the invoice: sum up the net per tax rate first, then
    // calculate the tax. Other than DocPositionList::fullTaxSum, negative sums
    // (cancellations, credit notes) result in negative tax here.
    e.taxFull = e.netFull.percent(e.fullTaxRate);
    e.taxReduced = e.netReduced.percent(e.reducedTaxRate);

    return e;
}

int VatReport::periodKey(const QDate &date) const
{
    int period = date.month();
    if (_type == PeriodType::Quarter) {
        period = (date.month() - 1) / 3 + 1;
    }
    return date.year() * 100 + period;
}

void VatReport::addDocument(KraftDoc *doc)
{
    const DocEntry e = entryFromDoc(doc);
    if (!e.taxDate.isValid()) {
        qDebug() << "Document without valid date can not be added to the VAT report:" << e.ident;
        return;
    }

    const int key = periodKey(e.taxDate);
    PeriodSum &sum = _periods[key];
    if (sum.year == 0) {
        sum.year = key / 100;
        sum.period = key % 100;
        sum.type = _type;
    }
    sum.add(e);
    sum.docs.append(e);
}

int VatReport::loadYear(int year)
{
    // An invoice can be issued up to six months after the service (§ 14 Abs. 2 UStG),
    // so invoices dated early next year may still belong to this year.
    const QDate from(year, 1, 1);
    const QDate to(year + 1, 6, 30);

    XmlIndexDb indx;
    int cnt{0};
    const QList<DocDigest> digests = indx.allDigests();
    for (const DocDigest &digest : digests) {
        const QDate d = digest.rawDate();
        // countsForVat checks if it is an invoice
        if (d < from || d > to || !countsForVat(digest))
            continue;

        KraftDoc *doc = DocumentMan::self()->openDocumentByUuid(digest.uuid());
        if (!doc) {
            qDebug() << "Unable to load document for VAT report:" << digest.ident();
            continue;
        }
        if (taxDate(doc).year() == year) {
            addDocument(doc);
            cnt++;
        }
        delete doc;
    }
    return cnt;
}

QList<VatReport::PeriodSum> VatReport::periods() const
{
    // QMap is sorted by key, which is year * 100 + period
    return _periods.values();
}

QList<VatReport::PeriodSum> VatReport::periodsOfYear(int year) const
{
    QList<PeriodSum> re;
    const int cnt = (_type == PeriodType::Quarter) ? 4 : 12;
    for (int period = 1; period <= cnt; period++) {
        const int key = year * 100 + period;
        if (_periods.contains(key)) {
            re.append(_periods.value(key));
        } else {
            PeriodSum sum;
            sum.year = year;
            sum.period = period;
            sum.type = _type;
            re.append(sum);
        }
    }
    return re;
}

VatReport::PeriodSum VatReport::total() const
{
    PeriodSum t;
    t.type = _type;
    for (const PeriodSum &p : _periods) {
        for (const DocEntry &e : p.docs) {
            t.add(e);
            t.docs.append(e);
        }
    }
    return t;
}
