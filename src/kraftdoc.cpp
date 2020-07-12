/***************************************************************************
                      KraftDoc.cpp  - Kraft document class
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
#include <QDir>
#include <QWidget>

#include <QDebug>

// application specific includes
#include "kraftsettings.h"
#include "kraftdoc.h"
#include "portal.h"
#include "kraftview.h"
#include "docposition.h"
#include "documentsaverdb.h"
#include "defaultprovider.h"
#include "documentman.h"
#include "doctype.h"
#include "documentman.h"

// FIXME: Make KraftDoc inheriting DocDigest!

KraftDoc::KraftDoc(QWidget *parent)
  : QObject(parent),
    _modified(false),
    mIsNew(true),
    mDocTypeChanged(false),
    mSaver(nullptr)
{
}

KraftDoc::~KraftDoc()
{
}

KraftDoc& KraftDoc::operator=( KraftDoc& origDoc )
{
  if ( this == &origDoc ) return *this;

  DocPositionListIterator it( origDoc.mPositions );

  while ( it.hasNext() ) {
    DocPosition *dp = static_cast<DocPosition*>( it.next() );

    DocPosition *newPos = new DocPosition();
    *newPos = *dp;
    newPos->setDbId( -1 );
    mPositions.append( newPos );
    // qDebug () << "Appending position " << dp->dbId().toString() << endl;
  }

  _modified = origDoc._modified;
  mIsNew = true;

  mAddressUid = origDoc.mAddressUid;
  mProjectLabel = origDoc.mProjectLabel;
  mPredecessor = origDoc.mPredecessor;
  mPredecessorDbId = origDoc.mPredecessorDbId;
  mAddress    = origDoc.mAddress;
  mPreText    = origDoc.mPreText;
  mPostText   = origDoc.mPostText;
  mDocType    = origDoc.mDocType;
  mDocTypeChanged = false;
  mSalut      = origDoc.mSalut;
  mGoodbye    = origDoc.mGoodbye;
  mIdent      = origDoc.mIdent;
  mWhiteboard = origDoc.mWhiteboard;

  // Two qualifiers for the locale settings.
  mCountry    = origDoc.mCountry;
  mLanguage   = origDoc.mLanguage;

  mDate = origDoc.mDate;
  mLastModified = origDoc.mLastModified;

  // setPositionList( origDoc.mPositions );
  mRemovePositions = origDoc.mRemovePositions;
  mSaver = nullptr;
  // mDocID = origDoc.mDocID;

  return *this;
}

void KraftDoc::closeDocument()
{
  deleteItems();
}

void KraftDoc::setPredecessor( const QString& w )
{
    mPredecessor = w;
}

bool KraftDoc::openDocument(const QString& id )
{
  DocumentSaverBase *loader = getSaver();
  loader->load( id, this );
  mDocTypeChanged = false;
  _modified=false;
  mIsNew = false;
  return true;
}

bool KraftDoc::reloadDocument()
{
  mPositions.clear();
  mRemovePositions.clear();

  return openDocument( mDocID.toString() );
}

bool KraftDoc::saveDocument( )
{
    bool result = false;

    DocumentSaverBase *saver = getSaver();
    if( saver ) {
        result = saver->saveDocument( this );
        if ( isNew() ) {
          setLastModified( QDateTime::currentDateTime() );
        }

        // We go through the whole document and remove the positions
        // that are to delete because they now were deleted in the
        // database.
        DocPositionListIterator it( mPositions );
        while( it.hasNext() ) {
          DocPositionBase *dp = it.next();
          if( dp->toDelete() ) {
            // qDebug () << "Removing pos " << dp->dbId().toString() << " from document object" << endl;
            mPositions.removeAll( dp );
          }
        }
        _modified = false;
    }
    return result;
}

QString KraftDoc::docIdentifier() const
{
  const QString id = ident();
  if( id.isEmpty() ) {
      return docType();
  }
  return i18nc("First argument is the doctype, like Invoice, followed by the ID",
               "%1 (Id %2)", docType(), id );
}

void KraftDoc::deleteItems()
{
    qDeleteAll(mPositions);
    mPositions.clear();
}

void KraftDoc::setDocType( const QString& s )
{
    if( s != mDocType ) {
        mDocType = s;
        mDocTypeChanged = true;
    }
}

void KraftDoc::setPositionList( DocPositionList newList, bool isNew)
{
  mPositions.clear();

  DocPositionListIterator it( newList );
  while ( it.hasNext() ) {
    DocPositionBase *dpb = it.next();
    DocPosition *dp = static_cast<DocPosition*>( dpb );
    DocPosition *newDp = createPosition( dp->type() );
    *newDp = *dp;
    if(isNew) {
        newDp->setDbId(-1);
    }
  }
}

DocPosition* KraftDoc::createPosition( DocPositionBase::PositionType t )
{
    DocPosition *dp = new DocPosition( t );
    mPositions.append( dp );
    return dp;
}

void KraftDoc::slotRemovePosition( int pos )
{
  // qDebug () << "Removing position " << pos << endl;

  foreach( DocPositionBase *dp, mPositions ) {
    // qDebug () << "Comparing " << pos << " with " << dp->dbId().toString() << endl;
    if( dp->dbId() == pos ) {
      if( ! mPositions.removeAll( dp ) ) {
        // qDebug () << "Could not remove!" << endl;
      } else {
        // qDebug () << "Successfully removed the position " << dp << endl;
        mRemovePositions.append( dp->dbId() ); // remember to delete
      }
    }
  }
}

void KraftDoc::slotMoveUpPosition( int dbid )
{
  // qDebug () << "Moving position " << dbid << " up" << endl;
  if( mPositions.count() < 1 ) return;
  int curPos = -1;

  // Search the one to move up
  for( int i = 0; curPos == -1 && i < mPositions.size(); i++ ) {
    if( (mPositions.at(i))->dbId() == dbid ) {
      curPos = i; // get out of the loop
    }
  }

  // qDebug () << "Found: "<< curPos << ", count: " << mPositions.count() << endl;
  if( curPos < mPositions.size()-1 ) {
    mPositions.swap( curPos, curPos+1 );
  }
}

void KraftDoc::slotMoveDownPosition( int dbid )
{
  // qDebug () << "Moving position " << dbid << " down" << endl;
  if( mPositions.count() < 1 ) return;
  int curPos = -1;

  // Search the one to move up
  for( int i = 0; curPos == -1 && i < mPositions.size(); i++ ) {
    if( (mPositions.at(i))->dbId() == dbid ) {
      curPos = i; // get out of the loop
    }
  }

  // qDebug () << "Found: "<< curPos << ", count: " << mPositions.count();
  if( curPos > 0 ) {
    mPositions.swap( curPos, curPos-1 );
  }
}

int KraftDoc::slotAppendPosition( const DocPosition& pos )
{
  DocPosition *dp = createPosition();
  *dp = pos; // FIXME: Proper assignment operator

  return mPositions.count();
}

DocumentSaverBase* KraftDoc::getSaver( const QString& )
{
    if( ! mSaver )
    {
        // qDebug () << "Create new Document DB-Saver" << endl;
        mSaver = new DocumentSaverDB();
    }
    return mSaver;
}

Geld KraftDoc::nettoSum() const
{
  return positions().nettoPrice();
}

Geld KraftDoc::bruttoSum() const
{
  Geld g = nettoSum();
  g += vatSum();
  return g;
}

Geld KraftDoc::fullTaxSum() const
{
    return positions().fullTaxSum(DocumentMan::self()->tax(date()));
}

Geld KraftDoc::reducedTaxSum() const
{
    return positions().reducedTaxSum(DocumentMan::self()->reducedTax(date()));
}

Geld KraftDoc::vatSum() const
{
  return positions().taxSum( DocumentMan::self()->tax( date() ),
                             DocumentMan::self()->reducedTax( date() ) );

  // return Geld( nettoSum() * DocumentMan::self()->vat()/100.0 );
}

QString KraftDoc::country() const
{
    QLocale *loc = DefaultProvider::self()->locale();
    return loc->countryToString(loc->country());
}

QString KraftDoc::language() const
{
    QLocale *loc = DefaultProvider::self()->locale();
    return loc->languageToString(loc->language());
}

 QString KraftDoc::partToString( Part p )
{
  if ( p == Header )
    return i18nc( "Document part header", "Header" );
  else if ( p == Footer )
    return i18nc( "Document part footer", "Footer" );
  else if ( p == Positions )
    return i18nc( "Document part containing the items", "Items" );

  return i18n( "Unknown document part" );
}
