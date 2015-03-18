/***************************************************************************
                  ArchDoc.cpp  - an archived document.
                             -------------------
    begin                : Mit Dez 31 19:24:05 CET 2003
    copyright            : (C) 2003 by Klaas Freitag
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

#include <QString>
#include <QSqlQuery>
#include <QDateTime>

// include files for KDE
#include <kglobal.h>

#include <klocale.h>
#include <kdebug.h>

// application specific includes
#include "archdoc.h"
#include "documentman.h"
#include "kraftdb.h"
#include "defaultprovider.h"

const char *SentOutDateC = "SentOutDate";

ArchDoc::ArchDoc()
    :mLocale( "kraft" ),
    mAttributes( QLatin1String("ArchDoc"))
{

}

ArchDoc::ArchDoc( const dbID& id )
    :mLocale( "kraft" ),
      mAttributes( QLatin1String("ArchDoc"))
{
    /* load archive from database */
    loadFromDb( id );
}

ArchDoc::~ArchDoc()
{
}


QString ArchDoc::docIdentifier() const
{
  QString re = docType();

  return i18n("%1 for %2 (Id %3)").arg( docType() ).arg( ident() );
}

Geld ArchDoc::nettoSum()
{
    const Geld g = positions().sumPrice();
    return g;
}

Geld ArchDoc::bruttoSum()
{
    Geld g = nettoSum();
    const Geld ts = taxSum();
    g += ts;
    return g;
}

Geld ArchDoc::taxSum()
{
    const Geld g = positions().taxSum( tax(), reducedTax() );
    return  g;
}

Geld ArchDoc::fullTaxSum()
{
    return positions().fullTaxSum( tax() );
}

Geld ArchDoc::reducedTaxSum()
{
    return positions().reducedTaxSum( reducedTax() );
}

double ArchDoc::tax()
{
    return mTax;
}

double ArchDoc::reducedTax()
{
    return mReducedTax;
}

void ArchDoc::loadFromDb( dbID id )
{
  mArchDocID = id;

  QSqlQuery q;
  q.prepare("SELECT archDocID, ident, docType, clientAddress, clientUid, " // pos 0..4
            "salut, goodbye, printDate, date, pretext, posttext, country, language, " // pos 5..12
            "projectLabel,tax, reducedTax, state from archdoc WHERE archDocID=:id" ); // pos 13..16
  q.bindValue(":id", id.toInt());
  q.exec();

  kDebug() << "Loading document id " << id.toString() << endl;

  if( q.next()) {
    QString docID;
    QString country;
    QString lang;
    docID         = q.value( 0 ).toString();
    mIdent        = q.value( 1 ).toString();
    mDocType      = q.value( 2 ).toString();
    mAddress      = q.value( 3 ).toString();
    mClientUid    = q.value( 4 ).toString();
    mSalut        = q.value( 5 ).toString();
    mGoodbye      = q.value( 6 ).toString();
    QVariant v    = q.value( 7 );
    mPrintDate    = v.toDateTime();
    mDate         = q.value( 8 ).toDate();
    mPreText      = KraftDB::self()->mysqlEuroDecode( q.value( 9 ).toString() );
    mPostText     = KraftDB::self()->mysqlEuroDecode( q.value( 10 ).toString() );
    country       = q.value( 11 ).toString();
    lang          = q.value( 12 ).toString();
    mProjectLabel = q.value( 13 ).toString();
    mTax          = q.value( 14 ).toDouble();
    mReducedTax   = q.value( 15 ).toDouble();
    mState        = q.value( 16 ).toInt();

    KConfig *cfg = KGlobal::config().data();
    mLocale.setCountry( country, cfg );
    mLocale.setLanguage( lang , cfg );

    loadPositions( docID );

    mAttributes.load(id);
  } else {
    kDebug() << "ERR: Could not load archived doc with id " << id.toString() << endl;
  }
}

void ArchDoc::loadPositions( const QString& archDocId )
{
  mPositions.clear();

  if ( archDocId.isEmpty() /* || ! archDocId.isNum() */ ) {
    kDebug() << "ArchDocId is not crappy: " << archDocId << endl;
    return;
  }

  QSqlQuery q;
  q.prepare("SELECT archPosID, archDocID, ordNumber, kind, postype, text, amount, " // pos 0..6
            "unit, price, overallPrice, taxType FROM archdocpos WHERE archDocID=:id ORDER BY ordNumber"); // pos 7..10
  q.bindValue("id", archDocId);
  q.exec();

  while( q.next() ) {
    ArchDocPosition pos;
    pos.mPosNo = q.value( 2 ).toString();
    pos.mKind = q.value( 3 ).toString();
    pos.mText = q.value( 5 ).toString();
    pos.mAmount = q.value( 6 ).toDouble();
    pos.mUnit  = q.value( 7 ).toString();
    pos.mUnitPrice = Geld( q.value( 8 ).toDouble() );
    pos.mOverallPrice = q.value( 9 ).toDouble();

    int tt = q.value( 10 ).toInt();
    if ( tt == 1 )
      pos.mTaxType = DocPositionBase::TaxNone;
    else if ( tt == 2 )
      pos.mTaxType = DocPositionBase::TaxReduced;
    else if ( tt == 3 )
      pos.mTaxType = DocPositionBase::TaxFull;

    mPositions.append( pos );
  }
}

QDateTime ArchDoc::sentOutDate()
{
    QDateTime re;

    if ( mAttributes.hasAttribute( SentOutDateC ) ) {
        re = mAttributes["sentOutDate"].value().toDateTime();
    }
    return re;
}

void ArchDoc::setSentOutDate( const QDateTime& dt )
{
    QString attName(SentOutDateC);
    if( dt.isValid() ) {
        Attribute att(attName);
        att.setPersistant(true);
        att.setValue(dt);
        mAttributes[attName] = att;
    } else {
        mAttributes.markDelete(attName);
    }
    mAttributes.save(mArchDocID);
}

ArchDocDigest ArchDoc::toDigest()
{
    return ArchDocDigest(mPrintDate, mState, mIdent, mArchDocID);
}

/* ###################################################################### */

ArchDocDigest::ArchDocDigest()
{

}

ArchDocDigest::ArchDocDigest( QDateTime dt,  int s, const QString& ident, dbID id )
  : mPrintDate( dt ),
    mState( s ),
    mArchDocId( id ),
    mIdent( ident )
{

}

ArchDocDigest::~ArchDocDigest()
{

}

QString ArchDocDigest::printDateString() const
{
  return DefaultProvider::self()->locale()->formatDateTime( mPrintDate, KLocale::ShortDate );
}

/* ###################################################################### */


