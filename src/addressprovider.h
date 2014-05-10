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

#include <kdeversion.h>

#include <kabc/addressee.h>
#include <akonadi/job.h>

#include <kjob.h>

class AddressProvider : public QObject
{
  Q_OBJECT
public:
  AddressProvider( QObject* parent = 0 );

  void getAddressee( const QString& uid );
  QString formattedAddress( const KABC::Addressee& ) const;

protected slots:
  void searchResult( KJob* );
signals:
  //
  void addresseeFound( const QString&, const KABC::Addressee& );

  // emitted when the search is finished, even if there was no result.
  void finished( int );

private:
  KJob *searchAddressGID( const QString& );

  QMap<KJob*, QString> mUidSearchJobs;
  QSet<QString>        mUidSearches;
};

#endif // ADDRESSPROVIDER_H
