/***************************************************************************
    contactsdirmodel.cpp  - A model of carddav contacts in a directory
                             -------------------
    begin                : Dec. 2024
    copyright            : (C) 2024 by Klaas Freitag
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

#include <QFileInfo>
#include <QIODevice>
#include <KContacts/Address>
#include <kcontacts/vcardconverter.h>

#include "models/contactsdirmodel.h"

QVariantList CTMItem::_headers{ "Name", "City", "UID" };

CTMItem::CTMItem(KContacts::Addressee& addressee, CTMItem* parentItem)
    :_parentItem(parentItem), _addressee(addressee)
{
    if (parentItem == nullptr) {

    }
}

void CTMItem::appendChild(std::unique_ptr<CTMItem> &child)
{
    _childItems.push_back(std::move(child));
}

CTMItem* CTMItem::child(int row)
{
    return row >= 0 && row < childCount() ? _childItems.at(row).get() : nullptr;
}

int CTMItem::childCount() const
{
    return int(_childItems.size());
}

int CTMItem::columnCount() const
{
    return int(_headers.count());
}

QVariant CTMItem::data(int column) const
{
    QVariant re;
    // qDebug() << "Checking for column" << column;
    if (column == 0) {
        // return name
        re = _addressee.formattedName();
    } else if (column == 1) {
        re = _addressee.address(KContacts::Address::Pref).locality();
    } else if (column == 2) {
        re = _addressee.uid();
    }

    return re;
}

int CTMItem::row() const
{
    if (_parentItem == nullptr) {
        return 0;
    }
    const auto it = std::find_if(_parentItem->_childItems.cbegin(), _parentItem->_childItems.cend(),
                                 [this](const std::unique_ptr<CTMItem> &CTMItem) {
                                     return CTMItem.get() == this;
                                 });

    if (it != _parentItem->_childItems.cend())
        return std::distance(_parentItem->_childItems.cbegin(), it);
    Q_ASSERT(false); // should not happen
    return -1;
}

CTMItem* CTMItem::parentItem()
{
    return _parentItem;
}

KContacts::Addressee CTMItem::getAddressee() const
{
    return _addressee;
}
// =============================================================================

ContactsDirModel::ContactsDirModel(const QString& baseDir, QObject *parent)
    : QAbstractItemModel(parent)
{

    QFileInfo fi(baseDir);
    if (!fi.exists()) {
        qDebug() << "Addressbook base dir does not exist.";
        return;
    }

    KContacts::Addressee rootAdr;
    rootAdr.setFormattedName(baseDir);
    _rootItem = std::make_unique<CTMItem>(rootAdr);

    for (const auto &dirEntry : QDirListing(baseDir, QDirListing::IteratorFlag::Recursive)) {
        if (dirEntry.fileName().endsWith(u".vcf")) {
            const QString file = dirEntry.fileInfo().canonicalFilePath();
            qDebug() << "reading vcard file" << file;
            QFile f(file);
            if( f.open( QIODevice::ReadOnly )) {
                const QByteArray data = f.readAll();
                KContacts::VCardConverter converter;
                KContacts::Addressee::List list = converter.parseVCards(data);

                if( list.count() > 0 ) {
                    auto contact = list.at(0);
                    auto item = std::make_unique<CTMItem>(contact, _rootItem.get());
                    _rootItem.get()->appendChild(item);
                    qDebug() << "Appending" << contact.formattedName();
                    // contact.insertCustom(CUSTOM_ADDRESS_MARKER, "manual");

                }
            }
        }
    }
}

ContactsDirModel::~ContactsDirModel() = default;

QVariant ContactsDirModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return {};

    const auto *item = static_cast<const CTMItem*>(index.internalPointer());
    return item->data(index.column());
}

Qt::ItemFlags ContactsDirModel::flags(const QModelIndex &index) const
{
    return index.isValid()
            ? QAbstractItemModel::flags(index) : Qt::ItemFlags(Qt::NoItemFlags);
}

QVariant ContactsDirModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return orientation == Qt::Horizontal && role == Qt::DisplayRole
        ? _rootItem->data(section) : QVariant{};
}

QModelIndex ContactsDirModel::index(int row, int column, const QModelIndex &parent) const
{
     if (!hasIndex(row, column, parent))
        return {};

    CTMItem *parentItem = parent.isValid()
        ? static_cast<CTMItem*>(parent.internalPointer())
        : _rootItem.get();

    if (auto *childItem = parentItem->child(row))
        return createIndex(row, column, childItem);
    return {};
}

QModelIndex ContactsDirModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return {};

    auto *childItem = static_cast<CTMItem*>(index.internalPointer());
    CTMItem *parentItem = childItem->parentItem();

    return parentItem != _rootItem.get()
        ? createIndex(parentItem->row(), 0, parentItem) : QModelIndex{};

}

int ContactsDirModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    CTMItem *parentItem;
    if (parent.isValid()) {
        parentItem = static_cast<CTMItem*>(parent.internalPointer());
    } else {
        parentItem = _rootItem.get();
    }
    int rc = parentItem->childCount();
    return rc;
}

int ContactsDirModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<CTMItem*>(parent.internalPointer())->columnCount();
    return _rootItem->columnCount();
}

