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

// use define CUSTOM_ADDRESS_MARKER to mark the origin of addresses with .insertCustom
#define CUSTOM_ADDRESS_MARKER "kraft", "identity_source"

class AddressProviderPrivate;


class AddressProvider : public QObject
{
  Q_OBJECT
public:
  AddressProvider( QObject* parent = 0 );

  enum LookupState { LookupOngoing, LookupStarted, LookupFromCache, LookupNotFound, ItemError, BackendError };

  bool backendUp();
  QString backendName() const;

  /**
   * @brief lookupAddressee - look up an addressee by it's uid.
   * @param uid - A unique string identifying the contact
   *
   * This is asynchron an asynchronous method that returns immediately.
   * Connect to the signal lookupResult() for the result.
   *
   * Make sure to always use a non empty uid.
   */
  LookupState lookupAddressee( const QString& uid );
  QString formattedAddress( const KContacts::Addressee& ) const;

  /**
    * @brief return an address from cache
    * @param uid - the unique address uid
    *
    * Use this method to get the address if the lookupAddressee method
    * returned LookupFromCache. In this case, the address is already
    * known and can be fetched synchronously
    */
  KContacts::Addressee getAddresseeFromCache(const QString& uid);

  /**
   * @brief model - returns an Qt model for a tree view.
   * @return a QAbstractItemModel
   */
  QAbstractItemModel *model();

  /**
   * @brief getAddressee - get the contact from an index
   * @param indx
   *
   * Depending on the underlying model in the private implementation
   * advantage can be taken from this functions.
   * Used by the addressselectorwidget
   * @return the found addressee
   */
  KContacts::Addressee getAddressee(const QModelIndex& indx);
  KContacts::Addressee getAddressee( int row, const QModelIndex &parent = QModelIndex());

  /**
   * @brief errorMsg - returns the error string for retrieval by uid
   * @param uid - the UID of the lookup job
   *
   * If the lookup job failed for whatever reason, this returns an error
   * message why, if that was available from the private implementation.
   * @return QString error message
   */
  QString errorMsg( const QString& uid );

public slots:
  void slotResetNotFoundCache();

protected slots:
  void slotErrorMsg(const QString& uid, const QString& msg);
  void slotAddresseeFound( const QString& uid, const KContacts::Addressee contact);
  void slotAddresseeNotFound( const QString& uid );

signals:
  /**
   * @brief lookupResult - deliver lookup result
   * @param uid - the uid of the lookup, and the contact
   *
   * If the contact is empty, it was simply not found. It can be checked
   * if there was an error using the errorMsg method for the uid. If there
   * was no error (errorMsg returns empty string) the addressbook just
   * did not contain the address.
   */
  void lookupResult( const QString&, const KContacts::Addressee& );

private:
  QHash<QString, KContacts::Addressee> _addressCache;

  AddressProviderPrivate *_d;
  QHash<QString, QString> _errMessages;
  QSet<QString> _notFoundUids;
};

#endif // ADDRESSPROVIDER_H
