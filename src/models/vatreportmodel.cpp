/***************************************************************************
    vatreportmodel.cpp - tree model quarter -> month -> invoice with VAT sums
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

#include <QFont>
#include <QPalette>
#include <QGuiApplication>

#include <klocalizedstring.h>

#include <algorithm>

#include "vatreportmodel.h"
#include "format.h"
#include "kraftsettings.h"

namespace {

QString dateStr(const QDate &d)
{
    return Format::toDateString(d, KraftSettings::self()->dateFormat());
}

bool isMoneyColumn(int column)
{
    return column >= VatReportModel::NetFull && column <= VatReportModel::TaxSum;
}

Geld moneyValue(const VatReport::PeriodSum &s, int column)
{
    switch (column) {
    case VatReportModel::NetFull:
        return s.netFull;
    case VatReportModel::TaxFull:
        return s.taxFull;
    case VatReportModel::NetReduced:
        return s.netReduced;
    case VatReportModel::TaxReduced:
        return s.taxReduced;
    case VatReportModel::NetNoTax:
        return s.netNoTax;
    case VatReportModel::TaxSum:
        return s.taxSum();
    default:
        return Geld();
    }
}

Geld moneyValue(const VatReport::DocEntry &e, int column)
{
    switch (column) {
    case VatReportModel::NetFull:
        return e.netFull;
    case VatReportModel::TaxFull:
        return e.taxFull;
    case VatReportModel::NetReduced:
        return e.netReduced;
    case VatReportModel::TaxReduced:
        return e.taxReduced;
    case VatReportModel::NetNoTax:
        return e.netNoTax;
    case VatReportModel::TaxSum: {
        Geld g{e.taxFull};
        g += e.taxReduced;
        return g;
    }
    default:
        return Geld();
    }
}

} // namespace

VatReportModel::VatReportModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

VatReportModel::~VatReportModel() = default;

void VatReportModel::clear()
{
    beginResetModel();
    _quarters.clear();
    _year = 0;
    endResetModel();
}

void VatReportModel::setReport(const VatReport &report, int year)
{
    beginResetModel();
    _quarters.clear();
    _year = year;

    for (int q = 1; q <= 4; q++) {
        auto quarter = std::make_unique<Node>();
        quarter->type = NodeType::Quarter;
        quarter->row = q - 1;
        quarter->sum.year = year;
        quarter->sum.period = q;
        quarter->sum.type = VatReport::PeriodType::Quarter;

        for (int m = 1; m <= 3; m++) {
            auto month = std::make_unique<Node>();
            month->type = NodeType::Month;
            month->parent = quarter.get();
            month->row = m - 1;
            month->sum.year = year;
            month->sum.period = 3 * (q - 1) + m;
            month->sum.type = VatReport::PeriodType::Month;
            quarter->children.push_back(std::move(month));
        }
        _quarters.push_back(std::move(quarter));
    }

    // regroup all documents of the year into the months
    QList<VatReport::DocEntry> entries = report.total().docs;
    std::stable_sort(entries.begin(), entries.end(), [](const VatReport::DocEntry &a, const VatReport::DocEntry &b) {
        return a.taxDate < b.taxDate;
    });

    for (const auto &e : std::as_const(entries)) {
        if (e.taxDate.year() != year)
            continue;
        const int monthIdx = e.taxDate.month() - 1;
        Node *quarter = _quarters.at(monthIdx / 3).get();
        Node *month = quarter->children.at(monthIdx % 3).get();

        auto doc = std::make_unique<Node>();
        doc->type = NodeType::Document;
        doc->parent = month;
        doc->row = static_cast<int>(month->children.size());
        doc->entry = e;
        month->children.push_back(std::move(doc));

        month->sum.add(e);
        month->sum.docs.append(e);
        quarter->sum.add(e);
        quarter->sum.docs.append(e);
    }

    endResetModel();
}

VatReportModel::Node *VatReportModel::nodeFromIndex(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;
    return static_cast<Node *>(index.internalPointer());
}

VatReportModel::NodeType VatReportModel::nodeType(const QModelIndex &index) const
{
    const Node *node = nodeFromIndex(index);
    return node ? node->type : NodeType::Invalid;
}

QString VatReportModel::uuid(const QModelIndex &index) const
{
    const Node *node = nodeFromIndex(index);
    if (node && node->type == NodeType::Document)
        return node->entry.uuid;
    return QString();
}

QModelIndex VatReportModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid()) {
        return createIndex(row, column, _quarters.at(row).get());
    }
    const Node *parentNode = nodeFromIndex(parent);
    return createIndex(row, column, parentNode->children.at(row).get());
}

QModelIndex VatReportModel::parent(const QModelIndex &index) const
{
    const Node *node = nodeFromIndex(index);
    if (!node || !node->parent)
        return QModelIndex();
    return createIndex(node->parent->row, 0, node->parent);
}

int VatReportModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return static_cast<int>(_quarters.size());
    if (parent.column() > 0)
        return 0;
    const Node *node = nodeFromIndex(parent);
    return static_cast<int>(node->children.size());
}

int VatReportModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return ColumnCount;
}

QVariant VatReportModel::displayData(const Node *node, int column) const
{
    if (node->type == NodeType::Document) {
        const VatReport::DocEntry &e = node->entry;
        switch (column) {
        case Label:
            return e.ident;
        case InvoiceDate:
            return dateStr(e.invoiceDate);
        case TaxDate:
            return e.dateFromInvoice ? dateStr(e.taxDate) + QStringLiteral(" *") : dateStr(e.taxDate);
        case Client:
            return e.clientAddress.section(QLatin1Char('\n'), 0, 0).trimmed();
        case DocCount:
            return QVariant();
        default:
            return moneyValue(e, column).toLocaleString();
        }
    }

    const VatReport::PeriodSum &s = node->sum;
    switch (column) {
    case Label:
        if (node->type == NodeType::Quarter) {
            return i18nc("Quarter label in VAT tree, %1 is the quarter number", "Q%1", s.period);
        }
        return QLocale().standaloneMonthName(s.period);
    case InvoiceDate:
    case TaxDate:
    case Client:
        return QVariant();
    case DocCount:
        return QString::number(s.docs.size());
    default:
        if (s.docs.isEmpty())
            return QStringLiteral("–");
        return moneyValue(s, column).toLocaleString();
    }
}

QVariant VatReportModel::sortData(const Node *node, int column) const
{
    if (isMoneyColumn(column)) {
        const Geld g = node->type == NodeType::Document ? moneyValue(node->entry, column) : moneyValue(node->sum, column);
        return QVariant::fromValue<qlonglong>(g.toLong());
    }
    if (node->type == NodeType::Document) {
        switch (column) {
        case Label:
            return node->entry.ident;
        case InvoiceDate:
            return node->entry.invoiceDate;
        case TaxDate:
            return node->entry.taxDate;
        case Client:
            return displayData(node, column);
        default:
            return QVariant();
        }
    }
    if (column == Label)
        return node->sum.period;
    if (column == DocCount)
        return static_cast<int>(node->sum.docs.size());
    return QVariant();
}

QVariant VatReportModel::data(const QModelIndex &index, int role) const
{
    const Node *node = nodeFromIndex(index);
    if (!node)
        return QVariant();

    const int column = index.column();

    switch (role) {
    case Qt::DisplayRole:
        return displayData(node, column);
    case SortRole:
        return sortData(node, column);
    case NodeTypeRole:
        return static_cast<int>(node->type);
    case UuidRole:
        return node->type == NodeType::Document ? node->entry.uuid : QString();
    case Qt::TextAlignmentRole:
        if (isMoneyColumn(column) || column == DocCount)
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    case Qt::FontRole:
        if (node->type == NodeType::Quarter) {
            QFont f;
            f.setBold(true);
            return f;
        }
        return QVariant();
    case Qt::ForegroundRole:
        if (node->type != NodeType::Document && node->sum.docs.isEmpty()) {
            return QGuiApplication::palette().brush(QPalette::Disabled, QPalette::Text);
        }
        return QVariant();
    case Qt::ToolTipRole:
        if (node->type == NodeType::Document) {
            QStringList tips;
            if (node->entry.dateFromInvoice)
                tips << i18n("The invoice has no time of supply, the invoice date is used.");
            if (node->entry.state == KraftDocState::State::Retracted)
                tips << i18n("Retracted invoice: The VAT shown on it is owed until it is corrected (§ 14c UStG).");
            if (node->entry.netUnknownTax.toLong() != 0)
                tips << i18n("Items with an undefined tax type are not included: %1", node->entry.netUnknownTax.toLocaleString());
            if (!tips.isEmpty())
                return tips.join(QLatin1Char('\n'));
        } else {
            return i18nc("date range of a tax period", "%1 – %2", dateStr(node->sum.firstDay()), dateStr(node->sum.lastDay()));
        }
        return QVariant();
    default:
        return QVariant();
    }
}

QVariant VatReportModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();

    if (role == Qt::TextAlignmentRole) {
        if (isMoneyColumn(section) || section == DocCount)
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    }
    if (role != Qt::DisplayRole)
        return QVariant();

    switch (section) {
    case Label:
        return i18n("Period / Invoice");
    case InvoiceDate:
        return i18n("Date");
    case TaxDate:
        return i18n("Supply");
    case Client:
        return i18n("Client");
    case NetFull:
        return i18n("Net full rate");
    case TaxFull:
        return i18n("VAT full rate");
    case NetReduced:
        return i18n("Net reduced rate");
    case TaxReduced:
        return i18n("VAT reduced rate");
    case NetNoTax:
        return i18n("Net without VAT");
    case TaxSum:
        return i18n("VAT total");
    case DocCount:
        return i18n("Invoices");
    default:
        return QVariant();
    }
}
