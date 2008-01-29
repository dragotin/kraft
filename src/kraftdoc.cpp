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
#include <qdir.h>
#include <qwidget.h>
#include <qptrlist.h>

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
#include "kraftdoc.h"
#include "portal.h"
#include "kraftview.h"
#include "docposition.h"
#include "documentsaverdb.h"
#include "defaultprovider.h"
#include "documentman.h"

// FIXME: Make KraftDoc inheriting DocDigest!

KraftDoc::KraftDoc(QWidget *parent, const char *name)
  : QObject(parent, name),
    mIsNew(true),
    mLocale(0),
    mSaver(0)
{
  pViewList = new QList<KraftView>();
  pViewList->setAutoDelete(false);
  mLocale = new KLocale( "kraft" );
}

KraftDoc::~KraftDoc()
{
  delete mLocale;
}

KraftView* KraftDoc::firstView()
{
  if( pViewList->count() > 0 ) {
    return pViewList->first();
  }
  return 0;
}

void KraftDoc::addView(KraftView *view)
{
  pViewList->append(view);
}

void KraftDoc::removeView(KraftView *view)
{
  pViewList->remove(view);
}

void KraftDoc::slotUpdateAllViews( KraftView *sender )
{
  KraftView *w;
  if(pViewList) {
    for(w=pViewList->first(); w!=0; w=pViewList->next()) {
      kdDebug() << "VIEW REDRAW, sender is " << sender << endl;
      if(w!=sender)
        w->redrawDocument( ); // no cache
    }
  }

}

void KraftDoc::closeDocument()
{
  deleteContents();
}

bool KraftDoc::newDocument()
{
  modified=false;

  /* initialise data */
  mDate = QDate::currentDate();
  mIdent = QString();

  mIsNew = true;
  mAddress = QString::null;
  mAddressUid = QString::null;

  mDocType = DefaultProvider::self()->docType();
  mPreText = DefaultProvider::self()->defaultText( mDocType, KraftDoc::Header );
  mPostText = DefaultProvider::self()->defaultText( mDocType, KraftDoc::Footer );

  mCountry  = DefaultProvider::self()->locale()->country();
  mLanguage = DefaultProvider::self()->locale()->language();

  mSalut = QString::null;
  mGoodbye = QString::null;

  return true;
}

bool KraftDoc::openDocument(const QString& id )
{
  DocumentSaverBase *loader = getSaver();
  loader->load( id, this );

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
        // that are to delete because they now were deleted in they
        // database.
        for( DocPositionBase *dp = mPositions.first(); dp;  ) {
          if( dp->toDelete() ) {
            kdDebug() << "Removing pos " << dp->dbId().toString() << " from document object" << endl;
            mPositions.remove();
            dp = mPositions.current();
          } else {
            dp = mPositions.next();
          }
        }
        modified = false;
    }
    return result;
}

QString KraftDoc::docIdentifier()
{
  QString re = docType();

  KABC::AddressBook *adrBook =  KABC::StdAddressBook::self();
  KABC::Addressee contact;
  if( adrBook ) {
    contact = adrBook->findByUid( addressUid() );
  }

  return i18n("%1 for %2 (Id %3)").arg( docType() ).arg( contact.realName() ).arg( ident() );
}

void KraftDoc::deleteContents()
{
  /////////////////////////////////////////////////
  // TODO: Add implementation to delete the document contents
  /////////////////////////////////////////////////

}

void KraftDoc::setPositionList( const DocPositionList& newList )
{
  mPositions.clear();

  QPtrListIterator<DocPositionBase> it( newList );
  DocPositionBase *dpb;
  // for( DocPositionBase *dpb = newList.first(); dpb; dpb = newList.next() ) {
  while ( ( dpb = it.current() ) != 0 ) {
    ++it;
    DocPosition *dp = static_cast<DocPosition*>( dpb );
    DocPosition *newDp = createPosition();
    *newDp = *dp;
  }
}

DocPosition* KraftDoc::createPosition()
{
    DocPosition *dp = new DocPosition();
    mPositions.append( dp );
    return dp;
}

void KraftDoc::slotRemovePosition( int pos )
{
  kdDebug() << "Removing position " << pos << endl;

  DocPositionBase *dp = 0;

  bool found = false;
  for( dp = mPositions.first(); !found && dp; dp = mPositions.next() ) {
    kdDebug() << "Comparing " << pos << " with " << dp->dbId().toString() << endl;
    if( dp->dbId() == pos ) {
      if( ! mPositions.removeRef( dp ) ) {
        kdDebug() << "Could not remove!" << endl;
      } else {
        kdDebug() << "Successfully removed the position " << dp << endl;
        mRemovePositions.append( dp->dbId() ); // remember to delete
        found = true;
      }
    }
  }

  if( found ) {
    slotUpdateAllViews( 0 );
  }
}

void KraftDoc::slotMoveUpPosition( int dbid )
{
  kdDebug() << "Moving position " << dbid << " up" << endl;
  DocPositionBase *dpLoop = mPositions.first();
  dpLoop = mPositions.next(); // Jump to second one, first cant be moved up
  DocPositionBase *dp = 0;
  int curPos = -1;

  for( ; curPos == -1 && dpLoop; dpLoop = mPositions.next() ) {
         kdDebug() << "Comparing " << dbid << " with " << dpLoop->dbId().toString() << endl;
         if( dpLoop->dbId() == dbid ) {
           curPos = mPositions.at();
           dp = mPositions.take();
         }
       }

  kdDebug() << "Found: "<< curPos << ", count: " << mPositions.count() << ", dp: " << dp << endl;
  if( curPos > -1 && dp ) {
    if( mPositions.insert( curPos-1, dp ) ) {
      kdDebug() << "Inserted successfully" << endl;
      slotUpdateAllViews( 0 );
    }
  }
}

void KraftDoc::slotMoveDownPosition( int dbid )
{
  kdDebug() << "Moving position " << dbid << " down" << endl;
  DocPositionBase *dpLoop = 0;
  DocPositionBase *dp = 0;
  DocPositionBase *dpLast = mPositions.last();
  int curPos = -1;

  for( dpLoop = mPositions.first(); curPos == -1 && dpLoop != dpLast;
        dpLoop = mPositions.next() ) {
    kdDebug() << "Comparing " << dbid << " with " << dpLoop->dbId().toString() << endl;
    if( dpLoop->dbId() == dbid ) {
      curPos = mPositions.at();
      dp = mPositions.take();
    }
  }

  kdDebug() << "Found: "<< curPos << ", count: " << mPositions.count() << ", dp: " << dp << endl;
  if( curPos > -1 && dp ) {
    if( mPositions.insert( curPos+1, dp ) ) {
      kdDebug() << "Inserted successfully" << endl;
      slotUpdateAllViews( 0 );
    }
  }
}

int KraftDoc::slotAppendPosition( const DocPosition& pos )
{
  DocPosition *dp = createPosition();
  *dp = pos; // FIXME: Proper assignment operator

  slotUpdateAllViews( 0 );
  return mPositions.count();
}

DocumentSaverBase* KraftDoc::getSaver( const QString& )
{
    if( ! mSaver )
    {
        kdDebug() << "Create new Document DB-Saver" << endl;
        mSaver = new DocumentSaverDB();
    }
    return mSaver;
}

Geld KraftDoc::nettoSum()
{
  return positions().sumPrice();
}

Geld KraftDoc::bruttoSum()
{
  Geld g = nettoSum();
  g += vatSum();
  return g;
}

Geld KraftDoc::vatSum()
{
  return Geld( nettoSum() * DocumentMan::self()->vat()/100.0 );
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
  kdDebug()<< "Setting country " << country << " and lang " << lang << endl;
  mLocale->setCountry( country );
  mLocale->setLanguage( lang );
}

 QString KraftDoc::partToString( Part p )
{
  if ( p == Header )
    return i18n( "Header" );
  else if ( p == Footer )
    return i18n( "Footer" );
  else if ( p == Positions )
    return i18n( "Positions" );

  return i18n( "Unknown document part" );
}


#include "kraftdoc.moc"
