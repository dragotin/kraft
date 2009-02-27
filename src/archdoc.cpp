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

// include files for Qt
#include <qsqlcursor.h>

// include files for KDE
#include <kglobal.h>

#include <klocale.h>
#include <kdebug.h>

// application specific includes
#include "archdoc.h"
#include "documentman.h"
#include "kraftdb.h"
#include "defaultprovider.h"


ArchDoc::ArchDoc()
  :mLocale( "kraft" )
{

}

ArchDoc::ArchDoc( const dbID& id )
  :mLocale( "kraft" )
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
  return positions().taxSum( DocumentMan::self()->tax( date() ),
                             DocumentMan::self()->reducedTax( date() ) );
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
  QSqlCursor cur("archdoc");
  cur.setMode( QSqlCursor::ReadOnly );
  kdDebug() << "Loading document id " << id.toString() << endl;

  cur.select( "archDocID = " +  id.toString()  );

  if( cur.next()) {
    kdDebug() << "loading archived document with ident " << id.toString() << endl;
    mAddress   = cur.value( "clientAddress" ).toString();
    mClientUid = cur.value( "clientUid" ).toString();
    mPreText   = KraftDB::self()->mysqlEuroDecode( cur.value( "pretext" ).toString() );
    mPostText  = KraftDB::self()->mysqlEuroDecode( cur.value( "posttext" ).toString() );
    mDocType   = cur.value( "docType" ).toString();
    mSalut     = cur.value( "salut" ).toString();
    mGoodbye   = cur.value( "goodbye" ).toString();
    mIdent     = cur.value( "ident" ).toString();
    mProjectLabel = cur.value( "projectLabel" ).toString();

    mDate      = cur.value( "date" ).toDate();
    mPrintDate = cur.value( "printDate" ).toDateTime();
    mState     = cur.value( "state" ).toInt();
    QString country = cur.value( "country" ).toString();
    QString lang = cur.value( "language" ).toString();
    mLocale.setCountry( country );
    mLocale.setLanguage( lang );

    mTax = cur.value( "tax" ).toDouble();
    mReducedTax = cur.value( "reducedTax" ).toDouble();

    QString docID = cur.value( "archDocID" ).toString();
    loadPositions( docID );
    loadAttributes( docID );
  } else {
    kdDebug() << "ERR: Could not load archived doc with id " << id.toString() << endl;
  }
}

void ArchDoc::loadPositions( const QString& archDocId )
{
  mPositions.clear();

  QSqlCursor cur( "archdocpos" );
  cur.setMode( QSqlCursor::ReadOnly );

  if ( archDocId.isEmpty() /* || ! archDocId.isNum() */ ) {
    kdDebug() << "ArchDocId is not crappy: " << archDocId << endl;
    return;
  }

  cur.select( "archDocID="+archDocId, cur.index( "ordNumber" ) );

  while( cur.next() ) {
    ArchDocPosition pos;
    pos.mText = cur.value( "text" ).toString();
    pos.mPosNo = cur.value( "ordNumber" ).toString();
    pos.mUnit  = cur.value( "unit" ).toString();
    pos.mUnitPrice = Geld( cur.value( "price" ).toDouble() );
    pos.mAmount = cur.value( "amount" ).toDouble();
    int tt = cur.value( "taxType" ).toInt();
    if ( tt == 1 )
      pos.mTaxType = DocPositionBase::TaxNone;
    else if ( tt == 2 )
      pos.mTaxType = DocPositionBase::TaxReduced;
    else if ( tt == 3 )
      pos.mTaxType = DocPositionBase::TaxFull;

    pos.mOverallPrice = cur.value( "overallPrice" ).toDouble();
    pos.mKind = cur.value( "kind" ).toString();
    mPositions.append( pos );
  }
}

void ArchDoc::loadAttributes( const QString& archDocId )
{
  mAttribs.clear();

  QSqlCursor cur( "archPosAttribs" );
  cur.setMode( QSqlCursor::ReadOnly );

  if ( archDocId.isEmpty() ) {
    kdDebug() << "ArchDocId is Empty!" << endl;
    return;
  }

  cur.select( "archDocID=" + archDocId );

  while ( cur.next() ) {
    QString name  = cur.value( "name" ).toString();
    QString value = cur.value( "value" ).toString();

    if ( !name.isEmpty() ) {
      mAttribs[ name ] = value;
    } else {
      kdDebug() << "Empty attribute name in archive!"  << endl;
    }
  }
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
  return DefaultProvider::self()->locale()->formatDateTime( mPrintDate, true );
}

/* ###################################################################### */

ArchDocDigestList::ArchDocDigestList ()
  :QValueList<ArchDocDigest>()
{

}


