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
#include <klocalizedstring.h>

// application specific includes
#include "kraftdoc.h"
#include "docposition.h"
#include "defaultprovider.h"
#include "documentman.h"
#include "doctype.h"
#include "documentman.h"
#include "kraftdb.h"
#include "format.h"
#include "unitmanager.h"
#include "kraftsettings.h"

// FIXME: Make KraftDoc inheriting DocDigest!

KraftDoc::KraftDoc(QWidget *parent)
  : QObject(parent),
    _modified(false),
    mDocTypeChanged(false),
    _state(State::New)
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
    // qDebug () << "Appending position " << dp->dbId().toString();
  }

  _modified = origDoc._modified;
  _state = State::New;

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

bool KraftDoc::openDocument(DocumentSaverBase &loader, const QString& ident)
{
    if (loader.loadByIdent(ident, this)) {
        mDocTypeChanged = false;
        _modified=false;
        _state = State::Draft;
        return true;
    }
    return false;
}

bool KraftDoc::reloadDocument(DocumentSaverBase &loader)
{
    const QString ident = mIdent;
    mPositions.clear();
    mRemovePositions.clear();

    return openDocument(loader, ident);
}

bool KraftDoc::saveDocument(DocumentSaverBase& saver)
{
    bool result = false;

    result = saver.saveDocument(this);

    if(result) {
        if ( isNew() ) {
            setLastModified( QDateTime::currentDateTime() );
        }

        // Go through the whole document and remove the positions
        // that are to delete because they now were deleted in the
        // database.
        DocPositionListIterator it( mPositions );
        while( it.hasNext() ) {
            DocPositionBase *dp = it.next();
            if( dp->toDelete() ) {
                // qDebug () << "Removing pos " << dp->dbId().toString() << " from document object";
                mPositions.removeAll( dp );
            }
        }
        _modified = false;
    }
    // FIXME - add this check
    // if (res) {
    //    _emitDBChangeSignal = false; // block sending of the signal
    //    slotCheckDocDatabaseChanged();
    //    _emitDBChangeSignal = true;
    // }
    return result;
}

void KraftDoc::setTimeOfSupply(QDateTime start, QDateTime end)
{
    _toSStart = start;
    _toSEnd = end;
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
  // qDebug () << "Removing position " << pos;

  foreach( DocPositionBase *dp, mPositions ) {
    // qDebug () << "Comparing " << pos << " with " << dp->dbId().toString();
    if( dp->dbId() == pos ) {
      if( ! mPositions.removeAll( dp ) ) {
        // qDebug () << "Could not remove!";
      } else {
        // qDebug () << "Successfully removed the position " << dp;
        mRemovePositions.append( dp->dbId() ); // remember to delete
      }
    }
  }
}

void KraftDoc::slotMoveUpPosition( int dbid )
{
  // qDebug () << "Moving position " << dbid << " up";
  if( mPositions.count() < 1 ) return;
  int curPos = -1;

  // Search the one to move up
  for( int i = 0; curPos == -1 && i < mPositions.size(); i++ ) {
    if( (mPositions.at(i))->dbId() == dbid ) {
      curPos = i; // get out of the loop
    }
  }

  // qDebug () << "Found: "<< curPos << ", count: " << mPositions.count();
  if( curPos < mPositions.size()-1 ) {
    mPositions.swap( curPos, curPos+1 );
  }
}

void KraftDoc::slotMoveDownPosition( int dbid )
{
  // qDebug () << "Moving position " << dbid << " down";
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
    return positions().fullTaxSum(UnitManager::self()->tax(date()));
}

Geld KraftDoc::reducedTaxSum() const
{
    return positions().reducedTaxSum(UnitManager::self()->reducedTax(date()));
}

Geld KraftDoc::vatSum() const
{
  return positions().taxSum( UnitManager::self()->tax( date() ),
                             UnitManager::self()->reducedTax( date() ) );

}

void KraftDoc::setStateFromString(const QString& s)
{
    _state = State::Undefined;
    if (s.isEmpty()) return;

    if (s == "Draft") {
        _state = State::Draft;
    } else if ( s == "Sent") {
        _state = State::Sent;
    } else if ( s == "Retracted") {
        _state = State::Retracted;
    } else if ( s == "Invalid") {
         _state = State::Invalid;
    } else {
        _state = State::Invalid;
    }
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
  if ( p == Part::Header )
    return i18nc( "Document part header", "Header" );
  else if ( p == Part::Footer )
    return i18nc( "Document part footer", "Footer" );
  else if ( p == Part::Positions )
    return i18nc( "Document part containing the items", "Items" );

  return i18n( "Unknown document part" );
}

 QString KraftDoc::preTextRaw() const
 {
     return mPreText;
 }

 QString KraftDoc::postTextRaw() const
 {
     return mPostText;
 }

 /**
  * @brief KraftDoc::resolveMacros
  * @param txtWithMacros - the string that might contain any macros
  * @param dposList - the list of document items
  * @return - the text with resolved macros
  *
  * The following macros are supported:
  * 1. SUM_PER_TAG(tag): The netto sum of all items that are tagged with the named tag
  * 2. IF_ANY_HAS_TAG(tag) .. END_HAS_TAG: The text between the macros only appears if
  *    at least one of all items is tagged with the named tag
  * 3. ITEM_COUNT_WITH_TAG(tag): This macro is replaced with the amount of items that
  *    are tagged with this tag
  *
  * 4. DATE_ADD_DAYS(days): Adds the amount of days to the date delivered in the call parameters
  */
 QString KraftDoc::resolveMacros(const QString& txtWithMacros, const DocPositionList dposList, const QDate& date, double fullTax, double redTax) const
 {
     QString myStr{txtWithMacros};
     QMap<QString, int> seenTags;

     QRegExp rxIf("\\s{1}IF_ANY_HAS_TAG\\(\\s*(\\w+)\\s*\\)");
     QRegExp rxEndif("\\s{1}END_HAS_TAG");
     QRegExp rxAmount("ITEM_COUNT_WITH_TAG\\(\\s*(\\w+)\\s*\\)");
     QRegExp rxAddDate("DATE_ADD_DAYS\\(\\s*(\\-{0,1}\\d+)\\s*\\)");
     // look for tag SUM_PER_TAG( HNDL )
     QRegExp rx("NETTO_SUM_PER_TAG\\(\\s*(\\w+)\\s*\\)");
     QRegExp rxBrutto("BRUTTO_SUM_PER_TAG\\(\\s*(\\w+)\\s*\\)");
     QRegExp rxVat("VAT_SUM_PER_TAG\\(\\s*(\\w+)\\s*\\)");

     int pos{0};
     QMap<QString, Geld> bruttoSums;
     QMap<QString, Geld> vatSums;
     Geld nettoSum;

     while ((pos = rx.indexIn(myStr, pos)) != -1) {

         const QString lookupTag = rx.cap(1);
         bruttoSums[lookupTag] = Geld();
         vatSums[lookupTag] = Geld();

         for (DocPositionBase *pb : dposList) {
             DocPosition *p = static_cast<DocPosition*>(pb);
             if (!p->toDelete() && p->hasTag(lookupTag)) {
                 Geld netto = p->overallPrice();

                 Geld tax;
                 if (p->taxType() == DocPositionBase::TaxType::TaxFull)
                     tax = netto.percent(fullTax);
                 else if (p->taxType() == DocPositionBase::TaxType::TaxReduced)
                     tax = netto.percent(redTax);

                 bruttoSums[lookupTag] += netto;
                 bruttoSums[lookupTag] += tax;
                 vatSums[lookupTag] += tax;
                 nettoSum += netto;
             }
         }

         myStr.replace(pos, rx.matchedLength(), nettoSum.toLocaleString());
     }

     // replace the Brutto- and vat tags if exist
     pos = 0;
     while ((pos = rxBrutto.indexIn(myStr, pos)) != -1) {
         const QString lookupTag = rxBrutto.cap(1);
         if (bruttoSums.contains(lookupTag)) {
             myStr.replace(pos, rxBrutto.matchedLength(), bruttoSums[lookupTag].toLocaleString());
         } else {
             qDebug() << "No Brutto sums computed for" << lookupTag;
         }
     }

     // vat tags
     pos = 0;
     while ((pos = rxVat.indexIn(myStr, pos)) != -1) {
         const QString lookupTag = rxVat.cap(1);
         if (vatSums.contains(lookupTag)) {
             myStr.replace(pos, rxVat.matchedLength(), vatSums[lookupTag].toLocaleString());
         }
     }

     // generate a list of all tags in any position
     for (DocPositionBase *pb : dposList) {
         DocPosition *p = static_cast<DocPosition*>(pb);
         if (!p->toDelete()) {
             const auto tags = p->tags();
             for (const QString& lookupTag : tags) {
                 if (seenTags.contains(lookupTag)) {
                     seenTags[lookupTag] = 1+seenTags[lookupTag];
                 } else {
                     seenTags[lookupTag] = 1;
                 }
             }
         }
     }

     pos = 0;
     while ((pos = rxAmount.indexIn(myStr, pos)) != -1) {
         const QString lookupTag = rxAmount.cap(1);
         int amount{0};
         if (seenTags.contains(lookupTag)) {
             amount = seenTags[lookupTag];
         }
         myStr.replace(pos, rxAmount.matchedLength(), QString::number(amount));
     }

     pos = 0;
     while ((pos = rxAddDate.indexIn(myStr, pos)) != -1) {
         const QString addDaysStr = rxAddDate.cap(1);
         qint64 addDays = addDaysStr.toInt();
         QDate newDate = date.addDays(addDays);
         const QString newDateStr = Format::toDateString(newDate, KraftSettings::self()->dateFormat());
         myStr.replace(pos, rxAddDate.matchedLength(), newDateStr);
     }

     // IF_ANY_HAS_TAG(tag) ..... END_HAS_TAG
     // check the IF_HAS_TAG(tag) ... END_HAS_TAG macro
     pos = 0;
     while ((pos = rxIf.indexIn(myStr, pos)) != -1) {
         const QString lookupTag = rxIf.cap(1);
         int endpos = myStr.lastIndexOf(rxEndif);
         if (endpos == -1) endpos = myStr.length();
         if (seenTags.contains(lookupTag)) {
             myStr.remove(endpos, 12 /* length of END_HAS_TAG */);
             myStr.remove(pos, rxIf.matchedLength());
         } else {
             // the tag was not seen, so this needs to be deleted.
             int len = endpos-pos+12;
             myStr.remove(pos, len);
         }
     }
     return myStr;
 }

 QString KraftDoc::preText() const
 {
     double fullTax = DocumentMan::self()->tax(date());
     double redTax = DocumentMan::self()->reducedTax(date());
     const QString myStr = resolveMacros(mPreText, positions(), date(), fullTax, redTax);
     return myStr;
 }

 QString KraftDoc::postText() const
 {
     double fullTax = DocumentMan::self()->tax(date());
     double redTax = DocumentMan::self()->reducedTax(date());
     const QString myStr = resolveMacros(mPostText, positions(), date(), fullTax, redTax);
     return myStr;
 }
