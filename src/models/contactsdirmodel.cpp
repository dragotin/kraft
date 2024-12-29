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

#include <KLocalizedString>
#include <QFileInfo>
#include <KContacts/Address>
#include <kcontacts/vcardconverter.h>

#include "models/contactsdirmodel.h"

QVariantList TreeItem::_headers{ "Name", "City"};

TreeItem::TreeItem(KContacts::Addressee& addressee, TreeItem* parentItem)
    :_parentItem(parentItem), _addressee(addressee)
{
    if (parentItem == nullptr) {

    }
}

void TreeItem::appendChild(std::unique_ptr<TreeItem> &child)
{
    _childItems.push_back(std::move(child));
}

TreeItem* TreeItem::child(int row)
{
    return row >= 0 && row < childCount() ? _childItems.at(row).get() : nullptr;
}

int TreeItem::childCount() const
{
    return int(_childItems.size());
}

int TreeItem::columnCount() const
{
    return int(_headers.count());
}

QVariant TreeItem::data(int column) const
{
    QVariant re;
    if (column == 0) {
        // return name
        re = _addressee.formattedName();
    } else if (column == 1) {
        re = _addressee.address(KContacts::Address::Pref).locality();
    }

    return re;
}

int TreeItem::row() const
{
    if (_parentItem == nullptr)
        return 0;
    const auto it = std::find_if(_parentItem->_childItems.cbegin(), _parentItem->_childItems.cend(),
                                 [this](const std::unique_ptr<TreeItem> &treeItem) {
                                     return treeItem.get() == this;
                                 });

    if (it != _parentItem->_childItems.cend())
        return std::distance(_parentItem->_childItems.cbegin(), it);
    Q_ASSERT(false); // should not happen
    return -1;
}

TreeItem* TreeItem::parentItem()
{
    return _parentItem;
}

// =============================================================================

ContactsDirModel::ContactsDirModel(QObject *parent, const QString& baseDir)
    : QAbstractItemModel(parent)
{

    QFileInfo fi(baseDir);
    if (!fi.exists()) {
        qDebug() << "Addressbook base dir does not exist.";
        return;
    }

    KContacts::Addressee rootAdr;
    rootAdr.setFormattedName(baseDir);
    auto rootItem = std::make_unique<TreeItem>(rootAdr);

    for (const auto &dirEntry : QDirListing(baseDir, QDirListing::IteratorFlag::Recursive)) {
         if (dirEntry.fileName().endsWith(u".vcf")) {
             const QString file = dirEntry.fileName();
             qDebug() << "looking up my identity in vcard file"<< file;
             QFile f(file);
             if( f.exists() ) {
                 if( f.open( QIODevice::ReadOnly )) {
                     const QByteArray data = f.readAll();
                     KContacts::VCardConverter converter;
                     KContacts::Addressee::List list = converter.parseVCards( data );

                     if( list.count() > 0 ) {
                        auto contact = list.at(0);
                        auto item = std::make_unique<TreeItem>(contact, rootItem.get());
                        rootItem.get()->appendChild(item);
                        // contact.insertCustom(CUSTOM_ADDRESS_MARKER, "manual");

                     }
                 }
             } else {
                 qDebug() << "VCard file does not exist!";
             }

         }
    }

}

ContactsDirModel::~ContactsDirModel() = default;

QVariant ContactsDirModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return {};

    const auto *item = static_cast<const TreeItem*>(index.internalPointer());
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

    TreeItem *parentItem = parent.isValid()
        ? static_cast<TreeItem*>(parent.internalPointer())
        : _rootItem.get();

    if (auto *childItem = parentItem->child(row))
        return createIndex(row, column, childItem);
    return {};
}

QModelIndex ContactsDirModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return {};

    auto *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parentItem();

    return parentItem != _rootItem.get()
        ? createIndex(parentItem->row(), 0, parentItem) : QModelIndex{};

}

int ContactsDirModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    const TreeItem *parentItem = parent.isValid()
        ? static_cast<const TreeItem*>(parent.internalPointer())
        : _rootItem.get();

    return parentItem->childCount();

}

int ContactsDirModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
    return _rootItem->columnCount();
}

