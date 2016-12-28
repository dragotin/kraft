/***************************************************************************
                          addressprovider.h
                             -------------------
    begin                : Fri Mar 4 2011
    copyright            : (C) 2011 by Klaas Freitag
    email                : freitag@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADDRESSPROVIDER_H
#define ADDRESSPROVIDER_H

#include <QSet>
#include <QAbstractItemModel>

#include <kcontacts/addressee.h>

#include <kjob.h>

class AddressProviderPrivate;


class AddressProvider : public QObject
{
  Q_OBJECT
public:
  AddressProvider( QObject* parent = 0 );

  void lookupAddressee( const QString& uid );
  QString formattedAddress( const KContacts::Addressee& ) const;

  QAbstractItemModel *model();
  KContacts::Addressee getAddressee( int row, const QModelIndex &parent);

protected slots:
  void searchResult( KJob* );

signals:
  //
  void addresseeFound( const QString&, const KContacts::Addressee& );
  void formattedAddressFound( const QString& uid, const QString& addressString );

  // emitted when the search is finished, even if there was no result.
  void finished( int );

private:
  AddressProviderPrivate *_d;
};

#endif // ADDRESSPROVIDER_H
