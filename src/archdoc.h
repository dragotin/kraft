/***************************************************************************
                          archdoc.h  -
                             -------------------
    begin                : Sep 2006
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

#ifndef ARCHDOC_H
#define ARCHDOC_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// include files for QT
#include <qstring.h>
#include <qdatetime.h>
#include <qvaluelist.h>
#include <qmap.h>

// include files for KDE
#include "archdocposition.h"
#include "geld.h"
#include "dbids.h"

class KLocale;

class ArchDoc
{
public:

  /** Constructor for the fileclass of the application */
  ArchDoc();
  ArchDoc( const dbID& );
  /** Destructor for the fileclass of the application */
  ~ArchDoc();

  ArchDocPositionList positions() const { return mPositions; }

  QDate date() const      { return mDate; }

  QString docType() const { return mDocType; }

  QString address() const { return mAddress; }

  QString clientUid() const { return mClientUid; }

  QString ident() const { return mIdent;    }

  QString salut() const { return mSalut;    }

  QString goodbye() const { return mGoodbye;    }

  QString preText() const { return mPreText;  }

  QString postText() const { return mPostText; }

  QString projectLabel() const { return mProjectLabel; }

  dbID docID() const { return mDocID; }

  QString docIdentifier() const;

  KLocale* locale() { return &mLocale; }
  
  QMap<QString, QString> attributes() const {
    return mAttribs;
  }

  Geld nettoSum();
  Geld bruttoSum();
  double vat();
  Geld vatSum();

private:
  void loadPositions( const QString& );
  void loadAttributes( const QString& );
  void loadFromDb( dbID );

  QString mAddress;
  QString mClientUid;
  QString mPreText;
  QString mPostText;
  QString mDocType;
  QString mSalut;
  QString mGoodbye;
  QString mIdent;
  QString mProjectLabel;

  QDate     mDate;
  QDateTime mPrintDate;

  KLocale   mLocale;

  ArchDocPositionList mPositions;
  dbID    mDocID;
  int     mState;
  QMap<QString, QString> mAttribs;
};

class ArchDocDigest
{
public:

  /** Constructor for the fileclass of the application */
  ArchDocDigest();
  ArchDocDigest( QDateTime, int, const QString&, dbID );
  /** Destructor for the fileclass of the application */
  ~ArchDocDigest();

  QDateTime printDate() {
    return mPrintDate;
  }

  int archDocState() {
    return mState;
  }

  dbID archDocId() {
    return mArchDocId;
  }

  QString archDocIdent() const {
    return mIdent;
  }

  QString printDateString() const;

private:
  QDateTime mPrintDate;
  int       mState;
  dbID      mArchDocId;
  QString   mIdent;
};

class ArchDocDigestList : public QValueList<ArchDocDigest>
{
public:
  ArchDocDigestList();
};

#endif // ARCHDOC_H
