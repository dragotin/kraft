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

// include files for KDE
#include <klocale.h>
#include <kmessagebox.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kabc/addressbook.h>
#include <kabc/stdaddressbook.h>
#include <kabc/addresseedialog.h>
#include <kabc/addressee.h>

// application specific includes
#include "kraftsettings.h"
#include "kraftdoc.h"
#include "portal.h"
#include "kraftview.h"
#include "docposition.h"
#include "documentsaverdb.h"
#include "defaultprovider.h"
#include "documentman.h"

// FIXME: Make KraftDoc inheriting DocDigest!

KraftDoc::KraftDoc(QWidget *parent)
  : QObject(parent),
    mIsNew(true),

    mDocTypeChanged(false),
    mLocale(0),
    mSaver(0)

{
  mLocale = new KLocale( "kraft" );
  mPositions.setLocale( mLocale );
}

KraftDoc::~KraftDoc()
{
  delete mLocale;
}

KraftDoc& KraftDoc::operator=( KraftDoc& origDoc )
{
  if ( this == &origDoc ) return *this;

  mLocale = new KLocale( "kraft" );

  DocPositionListIterator it( origDoc.mPositions );

  while ( it.hasNext() ) {
    DocPosition *dp = static_cast<DocPosition*>( it.next() );

    DocPosition *newPos = new DocPosition();
    *newPos = *dp;
    newPos->setDbId( -1 );
    mPositions.append( newPos );
    kDebug() << "Appending position " << dp->dbId().toString() << endl;
  }

  mPositions.setLocale( mLocale );

  modified = origDoc.modified;
  mIsNew = true;

  mAddressUid = origDoc.mAddressUid;
  mProjectLabel = origDoc.mProjectLabel;
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
  mSaver = 0;
  // mDocID = origDoc.mDocID;

  return *this;
}

void KraftDoc::closeDocument()
{
  deleteContents();
}

bool KraftDoc::newDocument( const QString& docType )
{
  modified=false;

  /* initialise data */
  mDate = QDate::currentDate();
  mIdent = QString();

  mIsNew = true;
  mAddress = QString::null;
  mAddressUid = QString::null;

  if( docType.isEmpty() ) {
    mDocType = DefaultProvider::self()->docType();
  } else {
    mDocType = docType;
  }
  mPreText = DefaultProvider::self()->defaultText( mDocType, KraftDoc::Header );
  mPostText = DefaultProvider::self()->defaultText( mDocType, KraftDoc::Footer );

  mCountry  = DefaultProvider::self()->locale()->country();
  mLanguage = DefaultProvider::self()->locale()->language();

  mSalut = KraftSettings::salut();
  mGoodbye = KraftSettings::greeting();
  mDocTypeChanged = false;
  return true;
}

bool KraftDoc::openDocument(const QString& id )
{
  DocumentSaverBase *loader = getSaver();
  loader->load( id, this );
  mDocTypeChanged = false;
  modified=false;
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
          setLastModified( QDate::currentDate() );
        }

        // We go through the whole document and remove the positions
        // that are to delete because they now were deleted in the
        // database.
        DocPositionListIterator it( mPositions );
        while( it.hasNext() ) {
          DocPositionBase *dp = it.next();
          if( dp->toDelete() ) {
            kDebug() << "Removing pos " << dp->dbId().toString() << " from document object" << endl;
            mPositions.removeAll( dp );
          }
        }
        modified = false;
    }
    return result;
}

QString KraftDoc::docIdentifier()
{
  QString re = docType();

  const QString realName = ""; // FIXME: get Realname out of Akonadi
  return i18n("%1 for %2 (Id %3)").arg( docType() ).arg( realName ).arg( ident() );

}

void KraftDoc::deleteContents()
{
    int pos = mPositions.size();
    for( int i=0; i < pos; i++) {
        DocPositionBase *pb = mPositions.takeFirst();
        delete pb;
    }
}

void KraftDoc::setDocType( const QString& s )
{
    if( s != mDocType ) {
        mDocType = s;
        mDocTypeChanged = true;
    }
}

void KraftDoc::setPositionList( DocPositionList newList )
{
  mPositions.clear();

  DocPositionListIterator it( newList );
  while ( it.hasNext() ) {
    DocPositionBase *dpb = it.next();
    DocPosition *dp = static_cast<DocPosition*>( dpb );
    DocPosition *newDp = createPosition( dp->type() );
    *newDp = *dp;
  }

  mPositions.setLocale( newList.locale() );
}

DocPosition* KraftDoc::createPosition( DocPositionBase::PositionType t )
{
    DocPosition *dp = new DocPosition( t );
    mPositions.append( dp );
    return dp;
}

void KraftDoc::slotRemovePosition( int pos )
{
  kDebug() << "Removing position " << pos << endl;

  foreach( DocPositionBase *dp, mPositions ) {
    kDebug() << "Comparing " << pos << " with " << dp->dbId().toString() << endl;
    if( dp->dbId() == pos ) {
      if( ! mPositions.removeAll( dp ) ) {
        kDebug() << "Could not remove!" << endl;
      } else {
        kDebug() << "Successfully removed the position " << dp << endl;
        mRemovePositions.append( dp->dbId() ); // remember to delete
      }
    }
  }
}

void KraftDoc::slotMoveUpPosition( int dbid )
{
  kDebug() << "Moving position " << dbid << " up" << endl;
  if( mPositions.count() < 1 ) return;
  int curPos = -1;

  // Search the one to move up
  for( int i = 0; curPos == -1 && i < mPositions.size(); i++ ) {
    if( (mPositions.at(i))->dbId() == dbid ) {
      curPos = i; // get out of the loop
    }
  }

  kDebug() << "Found: "<< curPos << ", count: " << mPositions.count() << endl;
  if( curPos < mPositions.size()-1 ) {
    mPositions.swap( curPos, curPos+1 );
  }
}

void KraftDoc::slotMoveDownPosition( int dbid )
{
  kDebug() << "Moving position " << dbid << " down" << endl;
  if( mPositions.count() < 1 ) return;
  int curPos = -1;

  // Search the one to move up
  for( int i = 0; curPos == -1 && i < mPositions.size(); i++ ) {
    if( (mPositions.at(i))->dbId() == dbid ) {
      curPos = i; // get out of the loop
    }
  }

  kDebug() << "Found: "<< curPos << ", count: " << mPositions.count();
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
        kDebug() << "Create new Document DB-Saver" << endl;
        mSaver = new DocumentSaverDB();
    }
    return mSaver;
}

Geld KraftDoc::nettoSum()
{
  return positions().nettoPrice();
}

Geld KraftDoc::bruttoSum()
{
  Geld g = nettoSum();
  g += vatSum();
  return g;
}

Geld KraftDoc::vatSum()
{
  return positions().taxSum( DocumentMan::self()->tax( date() ),
                             DocumentMan::self()->reducedTax( date() ) );

  // return Geld( nettoSum() * DocumentMan::self()->vat()/100.0 );
}

QString KraftDoc::country() const
{
  return mLocale->country();
}

QString KraftDoc::language() const
{
  return mLocale->language();
}

KLocale* KraftDoc::locale()
{
  return mLocale;
}

void KraftDoc::setCountryLanguage( const QString& lang, const QString& country )
{
  kDebug()<< "Setting country " << country << " and lang " << lang << endl;
  KConfig *cfg = KGlobal::config().data();
  mLocale->setCountry( country, cfg );
  mLocale->setLanguage( lang, cfg );
  mPositions.setLocale( mLocale );
}

 QString KraftDoc::partToString( Part p )
{
  if ( p == Header )
    return i18n( "Header" );
  else if ( p == Footer )
    return i18n( "Footer" );
  else if ( p == Positions )
    return i18n( "Items" );

  return i18n( "Unknown document part" );
}
