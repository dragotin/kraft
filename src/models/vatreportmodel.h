/***************************************************************************
     vatreportmodel.h - tree model quarter -> month -> invoice with VAT sums
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

#ifndef VATREPORTMODEL_H
#define VATREPORTMODEL_H

#include <QAbstractItemModel>

#include <memory>
#include <vector>

#include "vatreport.h"

/**
 * Read only tree model over the VAT of one year: the four quarters on the
 * top level, the three months of each quarter below and the invoices of
 * the month below that. Quarters and months carry the sums of their invoices.
 */
class VatReportModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Column {
        Label = 0,     // quarter, month or invoice number
        InvoiceDate,
        TaxDate,
        Client,
        NetFull,
        TaxFull,
        NetReduced,
        TaxReduced,
        NetNoTax,
        TaxSum,
        DocCount,
        ColumnCount
    };

    enum class NodeType { Invalid, Quarter, Month, Document };

    enum Roles {
        NodeTypeRole = Qt::UserRole + 1, // NodeType as int
        UuidRole,                        // uuid of the invoice, empty for periods
        SortRole                         // raw values: cents for money, QDate for dates
    };

    explicit VatReportModel(QObject *parent = nullptr);
    ~VatReportModel() override;

    // Builds the tree from the report. The report may be monthly or quarterly,
    // the documents are always regrouped into quarters and months of the year.
    void setReport(const VatReport &report, int year);
    void clear();

    int year() const { return _year; }

    NodeType nodeType(const QModelIndex &index) const;
    QString uuid(const QModelIndex &index) const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    struct Node {
        NodeType type{NodeType::Invalid};
        Node *parent{nullptr};
        int row{0};
        VatReport::PeriodSum sum;    // quarters and months
        VatReport::DocEntry entry;   // documents
        std::vector<std::unique_ptr<Node>> children;
    };

    Node *nodeFromIndex(const QModelIndex &index) const;
    QVariant displayData(const Node *node, int column) const;
    QVariant sortData(const Node *node, int column) const;

    std::vector<std::unique_ptr<Node>> _quarters;
    int _year{0};
};

#endif // VATREPORTMODEL_H
