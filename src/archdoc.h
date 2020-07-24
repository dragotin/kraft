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
#include <QObject>

#include "archdocposition.h"
#include "geld.h"
#include "dbids.h"

class QLocale;
class AttributeMap;

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

private:
  QDateTime mPrintDate;
  int       mState;
  dbID      mArchDocId;
  QString   mIdent;
};



class ArchDoc : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString docType READ docType)
    Q_PROPERTY(QString address READ address)
    Q_PROPERTY(QString clientUid READ clientUid)
    Q_PROPERTY(QString ident READ ident)
    Q_PROPERTY(QString salut READ salut)
    Q_PROPERTY(QString goodbye READ goodbye)
    Q_PROPERTY(QString preText READ preText)
    Q_PROPERTY(QString preTextHtml READ preTextHtml)
    Q_PROPERTY(QString postText READ postText)
    Q_PROPERTY(QString postTextHtml READ postTextHtml)
    Q_PROPERTY(QString projectLabel READ projectLabel)
    Q_PROPERTY(QString docIDStr READ docIdStr)
    Q_PROPERTY(QString docIdentifier READ docIdentifier)
    Q_PROPERTY(QString dateStr READ dateStr)

    Q_PROPERTY(QString nettoSumStr READ nettoSumStr)
    Q_PROPERTY(QString bruttoSumStr READ bruttoSumStr)
    Q_PROPERTY(QString taxSumStr READ taxSumStr)
    Q_PROPERTY(QString fullTaxSumStr READ fullTaxSumStr)
    Q_PROPERTY(QString reducedTaxSumStr READ reducedTaxSumStr)

    Q_PROPERTY(QString reducedTaxPercentStr READ reducedTaxPercentStr)
    Q_PROPERTY(QString fullTaxPercentStr READ fullTaxPercentStr)
    Q_PROPERTY(QString taxPercentStr READ taxPercentStr)

    Q_PROPERTY(QString taxMarkerFull READ taxMarkerFull)
    Q_PROPERTY(QString taxMarkerReduced READ taxMarkerReduced)

    Q_PROPERTY(QList<ArchDocPosition> items READ itemslist)
    Q_PROPERTY(bool hasIndividualTaxation READ hasIndividualTaxation)
public:

    const QString SentOutDateC {"SentOutDate"};

  /** Constructor for the fileclass of the application */
  ArchDoc();
  ArchDoc( const dbID& );
  /** Destructor for the fileclass of the application */
  ~ArchDoc();

  ArchDocPositionList positions() const { return mPositions; }
  QList<ArchDocPosition> itemslist() const;

  QDate date() const      { return mDate; }
  QString dateStr() const;

  QString docType() const { return mDocType; }

  QString address() const { return mAddress; }

  QString clientUid() const { return mClientUid; }

  QString ident() const { return mIdent;    }

  QString salut() const { return mSalut;    }

  QString goodbye() const { return mGoodbye;    }

  QString preText() const;
  QString preTextHtml() const;

  QString postText() const;
  QString postTextHtml() const;

  QString projectLabel() const { return mProjectLabel; }

  dbID docID() const { return mDocID; }
  QString docIdStr() const { return docID().toString(); }

  QString docIdentifier() const;

  Geld nettoSum() const;
  QString nettoSumStr() const { return nettoSum().toString(); }
  Geld bruttoSum() const;
  QString bruttoSumStr() const { return bruttoSum().toString(); }
  Geld taxSum() const;
  QString taxSumStr() const { return taxSum().toString(); }
  Geld fullTaxSum() const;
  QString fullTaxSumStr() const { return fullTaxSum().toString(); }
  Geld reducedTaxSum() const;
  QString reducedTaxSumStr() const { return reducedTaxSum().toString(); }

  QString fullTaxPercentStr() const;
  QString reducedTaxPercentStr() const;
  QString taxPercentStr() const;

  static QString taxMarkerNoTax()   { return QStringLiteral("1"); }
  static QString taxMarkerReduced() { return QStringLiteral("2"); }
  static QString taxMarkerFull()    { return QStringLiteral("");  }

  bool hasIndividualTaxation() const { return mPositions.hasIndividualTaxes(); }

  double tax() const;
  double reducedTax() const;

  ArchDocDigest toDigest();

  // when the document was sent to the customer.
  QDateTime sentOutDate();
  void setSentOutDate( const QDateTime& dt );

  void loadFromDb( dbID );

private:
  void loadItems( const QString& );

  dbID mArchDocID;
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

  ArchDocPositionList mPositions;
  dbID    mDocID;
  int     mState;
  AttributeMap mAttributes;
};

#endif // ARCHDOC_H
