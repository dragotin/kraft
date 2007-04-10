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


ArchDoc::ArchDoc()
{

}

ArchDoc::ArchDoc( const dbID& id )
{
  /* load archive from database */
  loadFromDb( id );
}

ArchDoc::~ArchDoc()
{
}


QString ArchDoc::docIdentifier()
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
  g += vatSum();
  return g;
}

Geld ArchDoc::vatSum()
{
  return Geld( nettoSum() * vat()/100.0 );
}

double ArchDoc::vat()
{
  return DocumentMan::self()->vat();
}

void ArchDoc::loadFromDb( dbID id )
{
  QSqlCursor cur("archdoc");
  kdDebug() << "Loading document id " << id.toString() << endl;

  cur.select( "archDocID = " +  id.toString()  );

  if( cur.next()) {
    kdDebug() << "loading archived document with ident " << id.toString() << endl;
    mAddress   = cur.value( "clientAddress" ).toString();
    mPreText   = cur.value( "pretext" ).toString();
    mPostText  = cur.value( "posttext" ).toString();
    mDocType   = cur.value( "docType" ).toString();
    mSalut     = cur.value( "salut" ).toString();
    mGoodbye   = cur.value( "goodbye" ).toString();
    mIdent     = cur.value( "ident" ).toString();
    mDate      = cur.value( "date" ).toDate();
    mPrintDate = cur.value( "printDate" ).toDateTime();
    mState     = cur.value( "state" ).toInt();

    QString docID = cur.value( "archDocID" ).toString();
    loadPositions( docID );
  } else {
    kdDebug() << "ERR: Could not load archived doc with id " << id.toString() << endl;
  }
}

void ArchDoc::loadPositions( const QString& archDocId )
{
  mPositions.clear();

  QSqlCursor cur( "archdocpos" );
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
    pos.mVat = cur.value( "vat" ).toDouble();

    mPositions.append( pos );
  }
}


/* ###################################################################### */

ArchDocDigest::ArchDocDigest()
{

}

ArchDocDigest::ArchDocDigest( QDateTime dt,  int s, dbID id )
  : mPrintDate( dt ),
    mState( s ),
    mArchDocId( id )
{

}

ArchDocDigest::~ArchDocDigest()
{

}

QString ArchDocDigest::printDateString() const
{
  return KGlobal().locale()->formatDateTime( mPrintDate, true );
}

/* ###################################################################### */

ArchDocDigestList::ArchDocDigestList ()
  :QValueList<ArchDocDigest>()
{

}


