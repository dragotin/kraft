/***************************************************************************
         vatreport.h - Sum of the VAT of invoices per tax period
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

#ifndef VATREPORT_H
#define VATREPORT_H

#include <QDate>
#include <QList>
#include <QMap>
#include <QString>

#include "geld.h"
#include "kraftdoc.h"

class DocDigest;

/**
 * Sums up the VAT contained in outgoing invoices per tax period (month or quarter).
 *
 * The invoices are assigned to the period in which the service was performed
 * (Sollversteuerung, § 13 Abs. 1 Nr. 1a UStG), ie. the end of the time of supply.
 * If an invoice has no time of supply, the invoice date is used instead and the
 * entry is flagged with dateFromInvoice.
 *
 * Input tax (Vorsteuer) is not known to Kraft, so this is NOT the tax to be paid,
 * but only the VAT from outgoing invoices.
 */
class VatReport
{
public:
    enum class PeriodType { Month, Quarter };

    // the VAT relevant data of one invoice
    struct DocEntry {
        QString uuid;
        QString ident;
        QString clientAddress;
        QDate invoiceDate;
        QDate taxDate;                // date that determines the tax period
        bool dateFromInvoice{false};  // true if there was no time of supply
        KraftDocState::State state{KraftDocState::State::Undefined};

        double fullTaxRate{0.0};
        double reducedTaxRate{0.0};
        Geld netFull;
        Geld taxFull;
        Geld netReduced;
        Geld taxReduced;
        Geld netNoTax;
        Geld netUnknownTax;           // positions with tax type Invalid or Individual
    };

    struct PeriodSum {
        int year{0};
        int period{0};                // month 1..12 or quarter 1..4
        PeriodType type{PeriodType::Month};

        Geld netFull;
        Geld taxFull;
        Geld netReduced;
        Geld taxReduced;
        Geld netNoTax;
        Geld netUnknownTax;

        QList<DocEntry> docs;

        // adds the sums of the entry, but does not append it to docs
        void add(const DocEntry &e);

        Geld taxSum() const;
        QDate firstDay() const;
        QDate lastDay() const;
        QString label() const;
    };

    explicit VatReport(PeriodType type = PeriodType::Month);

    PeriodType periodType() const { return _type; }

    // Loads all relevant invoices from the XML document index that fall into the
    // given year by their tax date. Returns the number of invoices added.
    int loadYear(int year);

    // Adds a single invoice to the report. The caller is responsible to only
    // pass invoices, see countsForVat()
    void addDocument(KraftDoc *doc);

    // all periods that contain at least one document, sorted ascending
    QList<PeriodSum> periods() const;

    // all periods of the year, also the ones without documents, sorted ascending
    QList<PeriodSum> periodsOfYear(int year) const;

    // sum over all periods
    PeriodSum total() const;

    // true if a document of this type and state carries VAT that is owed
    static bool countsForVat(const DocDigest &digest);
    static bool countsForVat(KraftDocState::State state);

    // the date that determines the tax period of the doc
    static QDate taxDate(KraftDoc *doc, bool *fromInvoiceDate = nullptr);

    static DocEntry entryFromDoc(KraftDoc *doc);

private:
    int periodKey(const QDate &date) const;

    PeriodType _type;
    QMap<int, PeriodSum> _periods; // key: year * 100 + period
};

#endif // VATREPORT_H
