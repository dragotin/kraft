/***************************************************************************
                          docdigest.h  -
                             -------------------
    begin                : Wed Mar 15 2006
    copyright            : (C) 2006 by Klaas Freitag
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
#ifndef DOCDIGEST_H
#define DOCDIGEST_H

#include <kcontacts/addressee.h>
#include <QLocale>
#include <QList>

#include "archdoc.h"
#include "kraftobj.h"

class QString;
class QDate;

typedef QList<ArchDocDigest> ArchDocDigestList;

class DocDigest : public KraftObj
{

public:
  DocDigest(const QString& type, const QString& clientID);
  DocDigest();

  QString clientId() const { return mClientId; }
  void setClientId( const QString& id ) { mClientId = id; }

  QString clientAddress() const { return mClientAddress; }
  void setClientAddress( const QString& address ) { mClientAddress = address; }

  KContacts::Addressee addressee() const;
  void setAddressee( const KContacts::Addressee& );

  QString type() const { return mType; }
  void setType( const QString& t ) { mType = t; }

  QString date() const;
  void setDate( const QDate& date ) { mDate = date; }
  QDate rawDate() const;

  QString ident() const   { return mIdent; }
  void setIdent( const QString& ident ) { mIdent = ident; }

  QString whiteboard() const  { return mWhiteboard; }
  void setWhiteboard( const QString& white ) { mWhiteboard = white; }

  void setProjectLabel( const QString& prjLabel ) { mProjectLabel = prjLabel; }
  QString projectLabel() const { return mProjectLabel; }

  ArchDocDigestList archDocDigestList() const;

protected:

  QString mType;
  QString mClientId;
  QString mIdent;
  QString mWhiteboard;
  QString mProjectLabel;
  QString mClientAddress ;

  QDate       mDate;
  QLocale     mLocale;

private:
  KContacts::Addressee mContact;
};

typedef QList<DocDigest> DocDigestList;

class DocDigestsTimeline
{
public:
  DocDigestsTimeline();
  DocDigestsTimeline( int,  int );

  int month() { return mMonth; }
  void setMonth( int m ) { mMonth = m; }
  int year()  { return mYear;  }
  void setYear( int y ) { mYear = y; }

  DocDigestList digests() { return mDigests; }
  void setDigestList( const DocDigestList& );
  void clearDigestList() { mDigests.clear (); }
private:
  int mMonth, mYear;
  DocDigestList mDigests;
};

typedef QList<DocDigestsTimeline> DocDigestsTimelineList;

#endif
