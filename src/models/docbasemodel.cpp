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

#include "docbasemodel.h"

#include <QStringList>
#include <QColor>
#include <QSqlQuery>

#include <klocalizedstring.h>


DocBaseModel::DocBaseModel(QObject *parent)
    :QAbstractItemModel(parent)
{
    _headers.resize(10);

    _headers[ Document_Id ]            = i18n("Date"); // this is only displayed by the date model
    _headers[ Document_Ident ]         = i18n("Doc. Number");
    _headers[ Document_Type ]          = i18n( "Doc. Type");
    _headers[ Document_Whiteboard ]    = i18n( "Whiteboard" );
    _headers[ Document_ClientId ]      = i18n( "Client Id" );
    _headers[ Document_LastModified]   = i18n( "Last modified" );
    _headers[ Document_CreationDate]   = i18n( "Creation date" );
    _headers[ Document_ProjectLabel]   = i18n( "Project" );
    _headers[ Document_ClientAddress ] = i18n( "Client Address" );
    _headers[ Document_ClientName ]    = i18n( "Client" );

    mAddressProvider = new AddressProvider( this );
    connect( mAddressProvider, SIGNAL(lookupResult(QString,KContacts::Addressee)),
            this, SLOT(slotAddresseeFound(QString, KContacts::Addressee)));

}

QString DocBaseModel::firstLineOf( const QString& str) const
{
    QString var;
    if( !str.isEmpty() ) {
        QStringList li = str.split(QChar('\n'));
        var = li[0];
    }
    return var;
}

QVariant DocBaseModel::columnValueFromDigest( const DocDigest& digest, int col ) const
{
    if( col < 0 || col > 10 ) return QVariant();
    QVariant var;
    QStringList li;
    QString help;

    switch(col) {
    case Document_Id:
    case Document_Id_Raw:
        var = digest.id();
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
    case Document_ProjectLabel:
        var = digest.projectLabel();
        break;
    case Document_ClientAddress: {
        help = firstLineOf( digest.clientAddress());
        break;
    }
    case Document_ClientName: {
        help = digest.clientId();
        AddressProvider::LookupState state = mAddressProvider->lookupAddressee(help);
        if( state == AddressProvider::LookupFromCache ) {
            KContacts::Addressee addressee = mAddressProvider->getAddresseeFromCache(help);
            var = addressee.assembledName();
            qDebug() << "Address from Cache: " << var.toString();
        } else if( state == AddressProvider::LookupOngoing ) {
            var = tr("Looking up address");
        } else if( state == AddressProvider::LookupStarted ) {
            var = tr("Lookup started");
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
    return 11;
}

QVariant DocBaseModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role == Qt::DisplayRole && orientation == Qt::Horizontal && section < _headers.count() ) {
        return _headers.at(section);
    }
    return QVariant();
}

Qt::ItemFlags DocBaseModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int DocBaseModel::loadFromTable()
{
    int cnt = 0;

    QSqlQuery query;

    query.prepare("SELECT docID, ident, docType, docDescription, clientID, lastModified,"
                  "date, projectLabel, clientAddress "
                  "FROM document ORDER BY date DESC");
    query.exec();

/*    enum Columns {
      Document_Id = 0,
      Document_Ident = 1,
      Document_Type = 2,
      Document_Whiteboard = 3,
      Document_ClientId = 4,
      Document_LastModified = 5,
      Document_CreationDate = 6,
      Document_ProjectLabel = 7,
      Document_ClientAddress = 8,
      Document_ClientName = 9,

    };
   */
    while (query.next()) {
        DocDigest digest(query.value(Document_Id).toInt(),
                         query.value(Document_Type).toString(),
                         query.value(Document_ClientId).toString());

        digest.setDate( query.value( Document_CreationDate ).toDate() );
        digest.setLastModified( query.value(Document_LastModified).toDateTime() );

        const QString clientAdr = query.value(Document_ClientAddress).toString();
        digest.setClientAddress( clientAdr );

        QString ident = query.value(Document_Ident).toString();
        digest.setIdent( ident );
        digest.setWhiteboard( query.value(Document_Whiteboard).toString() );
        digest.setProjectLabel( query.value(Document_ProjectLabel).toString() );

        const QString clientId = query.value(Document_ClientId).toString();
        digest.setClientId( clientId );
        if( mAddresses.contains( clientId )) {
            digest.setAddressee( mAddresses.value( clientId ));
        }

        this->addData( digest );
    }
    return cnt;
}
