/***************************************************************************
    addressprovider_local.cpp  - Addressprovider reading a directory
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

#include "addressproviderlocal.h"
#include "models/contactsdirmodel.h"
#include <kcontacts/contactgroup.h>

#include <QDebug>
#include <QFileInfo>
#include <QTimer>


AddressProviderLocal::AddressProviderLocal(const QString& baseDir, QObject *parent)
  :AddressProviderPrivate(parent), _baseDir(baseDir)
{

}

bool AddressProviderLocal::init()
{
    bool re{true};

    return re;
}

QString AddressProviderLocal::backendName() const
{
    return QStringLiteral("VCard Local Storage");
}


bool AddressProviderLocal::backendUp()
{
    QFileInfo fi{_baseDir};
    bool re{false};

    if (fi.exists() && fi.isDir()) {
        re =  true;
    }
    return re;
}

bool AddressProviderLocal::isSearchOngoing(const QString& uid)
{
    return mUidSearches.contains(uid);
}

bool AddressProviderLocal::lookupAddressee( const QString& uid )
{
    if( uid.isEmpty() ) {
        qDebug() << "Invalid: UID to lookup is empty.";
        return false;
    }

    if( mUidSearches.contains( uid ) ) {
        // search is already running
        // qDebug () << "Search already underways!";^
        return false;
    }

    // pretend to be be async here and return true.
    // send a signal with the result later...
    //         Q_EMIT lookupError(uid, errMsg );
    //         Q_EMIT addresseeFound(uid, contact);
    //         Q_EMIT addresseeNotFound(uid);
    mUidSearches.insert(uid);
    model(); // make sure the model exists.
    QTimer::singleShot(0, this,  [this, uid] () {this->searchResult(uid);});
    return true;
}

void AddressProviderLocal::searchResult(const QString& uid)
{
    KContacts::Addressee contact;

    // search
    auto *modl  = model();
    const QModelIndex rootIdx = QModelIndex();
    int rowCnt = modl->rowCount(rootIdx);
    qDebug() << "Amount of rows:" << rowCnt;

    for (int r = 0; r < rowCnt; r++) {
        QModelIndex idx = modl->index(r, 2, rootIdx); // 2 for Uid column
        const QVariant v = modl->data(idx, Qt::DisplayRole);
        if (v.toString() == uid) {
            CTMItem *item = static_cast<CTMItem*>(idx.internalPointer());
            if (item != nullptr) {
                contact = item->getAddressee();
                qDebug() << "Found contact" << contact.formattedName();
            }
            break;
        }
    }

    if (contact.isEmpty()) {
        Q_EMIT addresseeNotFound(uid);
    } else {
        Q_EMIT addresseeFound(uid, contact);
    }

    // cleanup
    if(!uid.isEmpty()) {
        mUidSearches.remove( uid );
    }
}

QAbstractItemModel* AddressProviderLocal::model()
{
    if( !_model ) {
        _model = std::make_unique<ContactsDirModel>(_baseDir);
    }
    return _model.get();
}

KContacts::Addressee AddressProviderLocal::getAddressee(const QModelIndex& indx)
{
    KContacts::Addressee contact;

    const auto *item = static_cast<const CTMItem*>(indx.internalPointer());
    if (item != nullptr)
        return item->getAddressee();

    return KContacts::Addressee();
}

KContacts::Addressee AddressProviderLocal::getAddressee(int row, const QModelIndex &parent)
{
    QModelIndex indx = _model->index(row, 0, parent);
    return getAddressee(indx);
}

