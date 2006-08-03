/***************************************************************************
                          KraftDoc.cpp  -
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

// QList<KraftView> *KraftDoc::pViewList = 0;

KraftDoc::KraftDoc(QWidget *parent, const char *name) : QObject(parent, name),
  mIsNew(true),
  mSaver(0)
{
  pViewList = new QList<KraftView>();
  pViewList->setAutoDelete(false);

}

KraftDoc::~KraftDoc()
{
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

void KraftDoc::setURL(const KURL &url)
{
  doc_url=url;
}

const KURL& KraftDoc::URL() const
{
  return doc_url;
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

bool KraftDoc::saveModified()
{
  bool completed=true;

  if(modified)
  {
    Portal *win=(Portal *) parent();
    int want_save = KMessageBox::warningYesNoCancel(win,
                                         i18n("The current file has been modified.\n"
                                              "Do you want to save it?"),
                                         i18n("Warning"));
    switch(want_save)
    {
      case KMessageBox::Yes:
           if (doc_url.fileName() == i18n("Untitled"))
           {
             // win->slotFileSaveAs();
           }
           else
           {
             saveDocument();
       	   };

       	   deleteContents();
           completed=true;
           break;

      case KMessageBox::No:
           setModified(false);
           deleteContents();
           completed=true;
           break;

      case KMessageBox::Cancel:
           completed=false;
           break;

      default:
           completed=false;
           break;
    }
  }

  return completed;
}

void KraftDoc::closeDocument()
{
  deleteContents();
}

bool KraftDoc::newDocument()
{
  /////////////////////////////////////////////////
  // TODO: Add your document initialization code here
  /////////////////////////////////////////////////
  modified=false;
  doc_url.setFileName(i18n("Untitled"));

  /* initialise data */
  mDate = QDate::currentDate();
  mIdent = QString("%1-%2").arg( mDate.year() ).arg( mDate.dayOfYear() );

  mIsNew = true;
  mAddress = QString::null;
  mAddressUid = QString::null;
  mPreText = QString::null;
  mPostText = QString::null;
  mDocType = i18n("Offer");
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

#if 0
    QDomDocument doc( "KraftDocument" );
    QDomElement root = doc.createElement( "KraftDocument" );
    doc.appendChild( root );

    root.appendChild( xmlTextElement( doc, "Greeting", mGreeting ) );

    root.appendChild( xmlTextElement( doc, "docType", mDocType ) );
    root.appendChild( xmlTextElement( doc, "preText", mPreText ) );
    root.appendChild( mPositions.domElement(doc) );
    root.appendChild( xmlTextElement( doc, "postText", mPreText ) );

    QString xml = doc.toString();
#endif
        // We go through the whole document and remove the positions
        // that are to delete because they now were deleted in the
        // database.
        for( DocPositionBase *dp = mPositions.first(); dp; dp = mPositions.next() ) {
          if( dp->toDelete() ) {
            mPositions.remove();
          }
        }
        // we sort the position list here again because the setup
        // routine KraftView::setupPositions uses the sort order
        // of the position list rather than the position method because
        // a non trivial position like 1.2, 1.3 etc. can be used more
        // easy. However the sort-compare method of docpositionlist must
        // be able to cope with that
        mPositions.sort();
        // for( DocPositionBase *dp = mPositions.first(); dp; dp = mPositions.next() ) {
        //  kdDebug() << "Sorted: " << dp->position() << endl;
        // }
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

DocPosition* KraftDoc::createPosition()
{
    DocPosition *dp = new DocPosition();

    mPositions.append( dp );
    dp->setPosition( QString::number( mPositions.count() ) );
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
    renumberPositions();
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
      renumberPositions();
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
      renumberPositions();
      slotUpdateAllViews( 0 );
    }
  }
}

int KraftDoc::slotAppendPosition( const DocPosition& pos )
{
  DocPosition *dp = createPosition();
  *dp = pos; // FIXME: Proper assignment operator

  dp->setPosition( QString::number( mPositions.count() ) );
  slotUpdateAllViews( 0 );
  return mPositions.count();
}

int KraftDoc::renumberPositions()
{
  int cnt = 1;
  DocPositionBase *dp;
  for( dp = mPositions.first(); dp; dp = mPositions.next() ) {
    dp->setPosition( QString::number( cnt ));
    if( ! dp->toDelete() )
      cnt++;
  }

  return cnt;
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

#include "kraftdoc.moc"
