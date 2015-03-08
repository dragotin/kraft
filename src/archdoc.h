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

// include files for QT
#include <QString>
#include <QDateTime>
#include <QMap>

// include files for KDE
#include "archdocposition.h"
#include "geld.h"
#include "dbids.h"

class KLocale;
class AttributeMap;

#define ARCHDOC_STATE_SENT 1

class ArchDocAttributer
{
public:
    enum State { NoState, Sent, Payed };

    ArchDocAttributer();
    ArchDocAttributer(const dbID& id);

    QString stateString( State state );
    bool hasDocState( State );

    QDateTime sentOutDate();
    void setSentOutDate( const QDateTime& dt );

    dbID archDocId() const;

    void setDocState( ArchDocAttributer::State state );

protected:
    AttributeMap mAttributes;
    dbID mArchDocID;
};

class ArchDocDigest : public ArchDocAttributer
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

  QString archDocIdent() const {
    return mIdent;
  }

  QString printDateString() const;

  QDate paymentDate();

  Geld payment();

private:
  QDateTime mPrintDate;
  int       mState;
  QString   mIdent;

};

class ArchDoc : public ArchDocAttributer
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

  QString docIdentifier() const;

  KLocale* locale() { return &mLocale; }

  Geld nettoSum();
  Geld bruttoSum();
  Geld taxSum();
  Geld fullTaxSum( );
  Geld reducedTaxSum();

  double tax();
  double reducedTax();

  ArchDocDigest toDigest();

  void setPayment( Geld g );

private:
  void loadPositions( const QString& );
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
  double  mTax;
  double  mReducedTax;

  QDate     mDate;
  QDateTime mPrintDate;

  KLocale   mLocale;

  ArchDocPositionList mPositions;
  int     mState;
  AttributeMap mAttributes;

};

#endif // ARCHDOC_H
