/***************************************************************************
                            datemodel.cpp
                          -------------------
    copyright            : (C) 2017 by Klaas Freitag
    email                : klaas@volle-kraft-voraus.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "models/docbasemodel.h"

#include <QStringList>
#include <QColor>

#include <klocalizedstring.h>


DocBaseModel::DocBaseModel(QObject *parent)
    :QAbstractItemModel(parent)
{
    _headers.resize(Max_Column_Marker);
    _headers[ Document_Id ]            = i18n("Date"); // this is only displayed by the date model
    _headers[ Document_Ident ]         = i18n("Doc. Number");
    _headers[ Document_Type ]          = i18n( "Doc. Type");
    _headers[ Document_Whiteboard ]    = i18n( "Whiteboard" );
    _headers[ Document_ClientId ]      = i18n( "Client Id" );
    _headers[ Document_LastModified]   = i18n( "Last modified" );
    _headers[ Document_CreationDateRaw]   = i18n( "Creation date" );
    _headers[ Document_ProjectLabel]   = i18n( "Project" );
    _headers[ Document_ClientAddress ] = i18n( "Client Address" );
    _headers[ Document_ClientName ]    = i18n( "Client" );
    _headers[ Document_StateStr ]      = i18n( "State" );

    mAddressProvider = new AddressProvider( this );
    connect( mAddressProvider, SIGNAL(lookupResult(QString,KContacts::Addressee)),
             this, SLOT(slotAddresseeFound(QString, KContacts::Addressee)));

}

QString DocBaseModel::firstLineOf( const QString& str) const
{
    QString var;
    if( !str.isEmpty() ) {
        QStringList li = str.split(QChar('\n'));
        var = QString( "> %1").arg(li[0]);
    }
    return var;
}

QVariant DocBaseModel::columnValueFromDigest( const DocDigest& digest, int col ) const
{
    if( col < 0 || col >= Max_Column_Marker ) return QVariant();
    QVariant var;
    QStringList li;
    QString help;

    switch(col) {
    case Document_Id:
        [[fallthrough]];
    case Document_Uuid:
        [[fallthrough]];
    case Document_Id_Raw:
        var = digest.uuid();
        break;
    case Document_Ident:
        var = digest.ident();
        break;
    case Document_Type:
        var = digest.type();
        break;
    case Document_Whiteboard:
        help = digest.whiteboard();
        li = help.split(QChar('\n'));
        var = li[0];
        break;
    case Document_ClientId:
        var = digest.clientId();
        break;
    case Document_LastModified:
        var = digest.lastModified();
        break;
    case Document_CreationDate:
        var = digest.date();
        break;
    case Document_CreationDateRaw:
        var = digest.rawDate();
        break;
    case Document_ProjectLabel:
        var = digest.projectLabel();
        break;
    case Document_ClientAddress: {
        var = firstLineOf( digest.clientAddress());
        break;
    }
    case Document_StateStr: {
        var = digest.stateStr();
        break;
    }
    case Document_ClientName: {
        help = digest.clientId();
        AddressProvider::LookupState state = mAddressProvider->lookupAddressee(help);
        if( state == AddressProvider::LookupFromCache ) {
            KContacts::Addressee addressee = mAddressProvider->getAddresseeFromCache(help);
            var = addressee.assembledName();
            // qDebug() << "Address from Cache: " << var.toString();
        } else if( state == AddressProvider::LookupOngoing ) {
            var = i18n("Looking up address");
        } else if( state == AddressProvider::LookupStarted ) {
            var = i18n("Lookup started");
        } else if( state == AddressProvider::LookupNotFound ||
                   state == AddressProvider::BackendError   ||
                   state == AddressProvider::ItemError ) {
            var = firstLineOf(digest.clientAddress());
        }
        break;
    }
    default:
        break;
    }
    return var;
}

int DocBaseModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return Max_Column_Marker;
}

QVariant DocBaseModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( section >= 0 && role == Qt::DisplayRole && orientation == Qt::Horizontal && section < _headers.count() ) {
        return _headers.at(section);
    }
    return QVariant();
}

Qt::ItemFlags DocBaseModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemFlags();

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void DocBaseModel::resetData()
{
    beginResetModel();
    removeAllData();
    loadDigests();
    endResetModel();

}
void DocBaseModel::slotAddresseeFound( const QString& uid, const KContacts::Addressee& contact)
{
    // FIXME: Update the data in the model and update the view accordingly.
    // Given that the view is updated so often, it does not seem to be neccessary
    // to do at all. Maybe later...
    Q_UNUSED(uid);
    Q_UNUSED(contact);
}

int DocBaseModel::loadDigests()
{
    return DefaultProvider::self()->documentPersister().addDigestsToModel(this);
}
