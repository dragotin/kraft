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

#ifndef ADDRESSPROVIDER_AKONADI_H
#define ADDRESSPROVIDER_AKONADI_H

#include <QSet>

#include <kcontacts/addressee.h>

#include <kjob.h>

class AddressProviderPrivate : public QObject
{
  Q_OBJECT
public:
  AddressProviderPrivate( QObject* parent = 0 );

  void getAddressee( const QString& uid );
  QString formattedAddress( const KContacts::Addressee& ) const;

public slots:
  void searchResult( KJob* );

signals:
  //
  void addresseeFound( const QString&, const KContacts::Addressee& );
  void formattedAddressFound( const QString& uid, const QString& addressString );

  // emitted when the search is finished, even if there was no result.
  void finished( int );

private:
  QMap<KJob*, QString> mUidSearchJobs;
  QSet<QString>        mUidSearches;
};

#endif // ADDRESSPROVIDER_H
