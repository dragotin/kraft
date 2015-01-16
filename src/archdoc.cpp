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
const char *ArchDocStateC = "ArchDocStates";

ArchDocAttributer::ArchDocAttributer()
    :mAttributes( QLatin1String("ArchDoc"))
{

}

ArchDocAttributer::ArchDocAttributer(const dbID& id)
    :mAttributes( QLatin1String("ArchDoc")),
      mArchDocID(id)
{
    mAttributes.load(id);
}

QString ArchDocAttributer::stateString( State state )
{
    if( state == NoState ) {
        return QLatin1String("NoState");
    }
    if( state == Sent ) {
        return QLatin1String("Sent");
    }
    if( state == Payed ) {
        return QLatin1String("Payed");
    }
    return QString();
}

bool ArchDocAttributer::hasDocState( ArchDocAttributer::State state )
{
    if( state == Sent ) {
        // check if there is a sent out date set
        return mAttributes.hasAttribute(SentOutDateC);
    } else {
        if( mAttributes.hasAttribute(ArchDocStateC)) {
            QStringList states = mAttributes[ArchDocStateC].value().toStringList();
            if( states.contains( stateString(state))) {
                return true;
            }
        }
    }
    return false;
}

void ArchDocAttributer::setDocState( ArchDocAttributer::State state )
{
    if( state == Sent ) {
        if( ! mAttributes.hasAttribute(ArchDocStateC) ) {
            setSentOutDate(QDateTime::currentDateTime());
        }
        return;
    }
    if( hasDocState(state )) {
        return;
    }

    Attribute att("ArchDocState");
    if( mAttributes.hasAttribute(ArchDocStateC)) {
        att = mAttributes[ArchDocStateC];
    }
    const QString stateStr = stateString(state);
    att.setValue(QVariant(stateStr));
    att.setPersistant(true);
    mAttributes[ArchDocStateC] = att;
    mAttributes.save(mArchDocID);

}

QDateTime ArchDocAttributer::sentOutDate()
{
    QDateTime re;

    if ( mAttributes.hasAttribute( SentOutDateC ) ) {
        re = mAttributes[SentOutDateC].value().toDateTime();
    }
    return re;
}

void ArchDocAttributer::setSentOutDate( const QDateTime& dt )
{
    if( dt.isValid() ) {
        Attribute att(SentOutDateC);
        att.setPersistant(true);
        att.setValue(dt);
        mAttributes[SentOutDateC] = att;
    } else {
        mAttributes.markDelete(SentOutDateC);
    }
    mAttributes.save(mArchDocID);
}

dbID ArchDocAttributer::archDocId() const {
    return mArchDocID;
}

/* ###################################################################### */

ArchDoc::ArchDoc()
    :ArchDocAttributer(),
      mLocale( "kraft" )
{

}

ArchDoc::ArchDoc( const dbID& id )
    :ArchDocAttributer(id),
      mLocale( "kraft" )
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
  return positions().sumPrice();
}

Geld ArchDoc::bruttoSum()
{
  Geld g = nettoSum();
  g += taxSum();
  return g;
}

Geld ArchDoc::taxSum()
{
  return positions().taxSum( tax(), reducedTax() ); // DocumentMan::self()->tax( date() ),
  // DocumentMan::self()->reducedTax( date() ) );
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

ArchDocDigest ArchDoc::toDigest()
{
    return ArchDocDigest(mPrintDate, mState, mIdent, mArchDocID);
}

/* ###################################################################### */

ArchDocDigest::ArchDocDigest()
    :ArchDocAttributer()
{

}

ArchDocDigest::ArchDocDigest( QDateTime dt,  int s, const QString& ident, dbID id )
    :ArchDocAttributer(id),
      mPrintDate( dt ),
      mState( s ),
      mIdent( ident )
{
    mArchDocID = id;
}

ArchDocDigest::~ArchDocDigest()
{

}

QString ArchDocDigest::printDateString() const
{
  return DefaultProvider::self()->locale()->formatDateTime( mPrintDate, KLocale::ShortDate );
}

/* ###################################################################### */


