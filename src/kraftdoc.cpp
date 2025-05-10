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
#include <QJsonObject>
#include <QWidget>

#include <QDebug>
#include <klocalizedstring.h>
#include <KLazyLocalizedString>

// application specific includes
#include "kraftdoc.h"
#include "docposition.h"
#include "defaultprovider.h"
#include "documentman.h"
#include "documentman.h"
#include "kraftdb.h"
#include "format.h"
#include "unitmanager.h"
#include "kraftsettings.h"
#include "docdigest.h"
#include "docidentgenerator.h"

namespace {

QString multilineHtml( const QString& str )
{
    QString re {str.toHtmlEscaped()};

    re.replace( '\n', "<br/>");
    return re;
}

} // end namespace

const QString KraftDocState::StateUndefinedStr = QStringLiteral("Undefined");
const QString KraftDocState::StateNewStr = QStringLiteral("New");
const QString KraftDocState::StateDraftStr{"Draft"};
const QString KraftDocState::StateFinalStr{"Final"};
const QString KraftDocState::StateRetractedStr{"Retracted"};
const QString KraftDocState::StateInvalidStr{"Invalid"};
const QString KraftDocState::StateConvertedStr{"Converted"};

const KLazyLocalizedString KraftDocState::StateUndefinedI18n = kli18n("Undefined");
const KLazyLocalizedString KraftDocState::StateNewI18n = kli18n("New");
const KLazyLocalizedString KraftDocState::StateDraftI18n = kli18n("Draft");
const KLazyLocalizedString KraftDocState::StateFinalI18n = kli18n("Final");
const KLazyLocalizedString KraftDocState::StateRetractedI18n = kli18n("Retracted");
const KLazyLocalizedString KraftDocState::StateInvalidI18n = kli18n("Invalid");
const KLazyLocalizedString KraftDocState::StateConvertedI18n = kli18n("Converted");

// =====================================================================================
void KraftDocState::setStateFromString(const QString& s)
{
    _state = State::Undefined;
    if (s.isEmpty()) return;

    if (s == StateUndefinedStr) {
        _state = State::Undefined;
    } else if ( s == StateNewStr) {
        _state = State::New;
    } else if ( s == StateDraftStr) {
        _state = State::Draft;
    } else if ( s == StateFinalStr) {
        _state = State::Final;
    } else if ( s == StateRetractedStr) {
        _state = State::Retracted;
    } else if ( s == StateInvalidStr) {
         _state = State::Invalid;
    } else if ( s == StateConvertedStr) {
         _state = State::Converted;
    } else {
        _state = State::Invalid;
    }
}

QString KraftDocState::stateStringI18n() const
{
    switch(_state) {
    case State::New:
        return StateNewI18n.toString();
        break;
    case State::Draft:
        return StateDraftI18n.toString();
        break;
    case State::Final:
        return StateFinalI18n.toString();
        break;
    case State::Retracted:
        return StateRetractedI18n.toString();
        break;
    case State::Invalid:
        return StateInvalidI18n.toString();
        break;
    case State::Undefined:
        return StateUndefinedI18n.toString();
        break;
    case State::Converted:
        return StateConvertedI18n.toString();
    }
    return StateUndefinedI18n.toString();
}

QString KraftDocState::stateString() const
{
    switch(_state) {
    case State::New:
        return StateNewStr;
        break;
    case State::Draft:
        return StateDraftStr;
        break;
    case State::Final:
        return StateFinalStr;
        break;
    case State::Retracted:
        return StateRetractedStr;
        break;
    case State::Invalid:
        return StateInvalidStr;
        break;
    case State::Undefined:
        return StateUndefinedStr;
        break;
    case State::Converted:
        return StateConvertedStr;
    }
    return StateUndefinedStr;
}

QList<KraftDocState::State> KraftDocState::validFollowStates(KraftDocState::State nowState)
{
    QList<State> re;

    switch(nowState) {
    case State::Converted:
    case State::Invalid:
    case State::Retracted:
        qDebug() << "No follow up state for converted.";
        break;
    case State::Draft:
        re.append(State::Final);
        break;
    case State::Final:
        re.append(State::Retracted);
        break;
    case State::New:
        re.append(State::Draft);
        break;
    case State::Undefined:
        re.append(State::Draft);
        re.append(State::Converted);
        re.append(State::Final);
        break;
    }
    return re;
}

bool KraftDocState::canBeFinalized() const
{
    bool re {false};
    const auto validStates = validFollowStates(_state);
    if (validStates.indexOf(State::Final) > -1) {
        // can be finalized
        re = true;
    }
    return re;
}

bool KraftDocState::forcesReadOnly()
{
    bool re{false};

    if (_state == State::Final) {
        re = true;
    }
    return re;
}

// =====================================================================================


KraftDoc::KraftDoc(QWidget *parent)
  : QObject(parent), KraftObj(),
    mDocTypeChanged(false),
    _fullTax(-1.0),
    _redTax(-1.0)
{
    _state.setState(KraftDocState::State::New);
}

KraftDoc::~KraftDoc()
{
}

void KraftDoc::clear()
{
    mAddressUid.clear();
    mProjectLabel.clear();
    mAddress.clear();
    mPreText.clear();
    mPostText.clear();
    mDocType.clear();
    mDocTypeChanged = false;
    mSalut.clear();
    mGoodbye.clear();
    mIdent.clear();
    mWhiteboard.clear();
    mPredecessor.clear();
    mPredecessorDbId.clear();

    // Two qualifiers for the locale settings.
    mCountry.clear();
    mLanguage.clear();

    mDate = QDate();

    // Time of supply
    _toSStart = QDateTime();
    _toSEnd = QDateTime();
    _owner.clear();

    mPositions.clear();
    mRemovePositions.clear();
    _fullTax = -1.0;
    _redTax = -1.0;
}

KraftDoc& KraftDoc::operator=( KraftDoc& origDoc )
{
  if ( this == &origDoc ) return *this;

  DocPositionListIterator it( origDoc.mPositions );

  while ( it.hasNext() ) {
    DocPosition *dp = it.next();

    DocPosition *newPos = new DocPosition();
    *newPos = *dp;
    newPos->setDbId( -1 );
    mPositions.append( newPos );
    // qDebug () << "Appending position " << dp->dbId().toString();
  }
  if (origDoc.modified()) {
      setModified();
  }

  _state.setState(KraftDocState::State::New);
  KraftObj::operator=(origDoc);
  _uuid = QUuid(); // clear the Uuid

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

  _lastModified = origDoc._lastModified;

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

bool KraftDoc::openDocument(DocumentSaverBase &loader, const QString& uuid)
{
    if (loader.loadByUuid(uuid, this)) {
        mDocTypeChanged = false;
        _modified=false;
        return true;
    } else {
        qDebug() << "Failed to load doc by Uuid";
    }
    return false;
}

bool KraftDoc::reloadDocument(DocumentSaverBase &loader)
{
    const QString uuid = this->uuid();
    mPositions.clear();
    mRemovePositions.clear();

    return openDocument(loader, uuid);
}

bool KraftDoc::saveDocument(DocumentSaverBase& saver)
{
    bool result = false;

    result = saver.saveDocument(this);

    if(result) {
        if ( state().isNew() ) {
            setLastModified( QDateTime::currentDateTime() );
        }

        // Go through the whole document and remove the positions
        // that are to delete because they now were deleted in the
        // database.
        DocPositionListIterator it( mPositions );
        while( it.hasNext() ) {
            DocPosition *dp = it.next();
            if( dp->toDelete() ) {
                // qDebug () << "Removing pos " << dp->dbId().toString() << " from document object";
                mPositions.removeAll( dp );
            }
        }
        _modified = false;
    }

    Q_EMIT saved(result);

    // FIXME - add this check
    // if (res) {
    //    _emitDBChangeSignal = false; // block sending of the signal
    //    slotCheckDocDatabaseChanged();
    //    _emitDBChangeSignal = true;
    // }
    return result;
}

DocDigest KraftDoc::toDigest()
{
    DocDigest digest(docType(), addressUid());
    digest.setUuid(uuid());
    digest.setDate(date());
    digest.setLastModified(lastModified());

    digest.setClientAddress(address());
    digest.setIdent(ident());
    digest.setWhiteboard(whiteboard());
    digest.setProjectLabel(projectLabel());
    digest.setState(_state);

    for( const auto &attrib : attributes()) {
        digest.setAttribute(attrib);
    }
    digest.setTags(allTags());

    return digest;
}

void KraftDoc::toJsonObj(QJsonObject& obj) const
{
    obj["uuid"] = uuid();
    obj["date"] = Format::toDateString(date(), Format::DateFormatIso);
    obj["lastModified"] = lastModified().toString(Qt::ISODate);
    obj["clientAddress"] = address();
    obj["ident"] = ident();
    obj["prjtLabel"] = projectLabel();
    obj["state"] = _state.stateString();
    obj["whiteboard"] = whiteboard();
    obj["docType"] = docType();
}

void KraftDoc::setTimeOfSupply(QDateTime start, QDateTime end)
{
    _toSStart = start;
    _toSEnd = end;
}

QString KraftDoc::docIdentifier() const
{
    QString re;
    if (isDraftState()) {
        re = i18nc("First document type, second date", "%1 from %2 (Draft)", docType(), dateStr());
    } else {
        // both components are already translated.
        re = QString("%1 %2").arg(docType()).arg(ident());
        // re = i18nc("First argument is the doctype, like Invoice, followed by the ID", "%1 %2", docType(), ident());
    }
    return re;
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
    DocPosition *dpb = it.next();
    DocPosition *newDp = createPosition( dpb->type() );
    *newDp = *dpb;

    // copy attribs and tags as they are not copied otherwise
    newDp->setTags(dpb->allTags());
    QMap<QString, KraftAttrib> attribs = dpb->attributes();
    for (const auto& attrib : attribs.values()) {
        newDp->setAttribute(attrib);
    }
    if(isNew) {
        newDp->createUuid();
    }
  }
}

DocPosition* KraftDoc::createPosition(DocPosition::Type t )
{
    DocPosition *dp = new DocPosition( t );
    mPositions.append( dp );
    return dp;
}

void KraftDoc::slotRemovePosition( int pos )
{
  // qDebug () << "Removing position " << pos;

  for( DocPosition *dp: mPositions ) {
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
    mPositions.swapItemsAt( curPos, curPos+1 );
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
    mPositions.swapItemsAt( curPos, curPos-1 );
  }
}

void KraftDoc::setTaxValues(double fullTax, double redTax)
{
    _fullTax = fullTax;
    _redTax  = redTax;
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
    Geld g;

    if (_fullTax < 0) {
        g = positions().fullTaxSum(UnitManager::self()->tax(date()));
    } else {
        g = positions().fullTaxSum(_fullTax);
    }
    return g;
}

Geld KraftDoc::reducedTaxSum() const
{
    if (_redTax < 0) {
        return positions().reducedTaxSum(UnitManager::self()->reducedTax(date()));
    } else {
        return positions().reducedTaxSum(_redTax);
    }
}

Geld KraftDoc::vatSum() const
{
    if (_fullTax < 0) {
        return positions().taxSum( UnitManager::self()->tax( date() ),
                                   UnitManager::self()->reducedTax( date() ) );
    } else {
        return positions().taxSum(_fullTax, _redTax);
    }
}

QString KraftDoc::taxPercentStr() const
{
     DocPosition::Tax tt = mPositions.listTaxation();
     if (tt == DocPosition::Tax::Full) {
         return fullTaxPercentStr();
     } else if (tt == DocPosition::Tax::Reduced) {
         return reducedTaxPercentStr();
     }
     return QString();
}

QString KraftDoc::taxPercentNum() const
{
    DocPosition::Tax tt = mPositions.listTaxation();
    if (tt == DocPosition::Tax::Full) {
        return fullTaxPercentNum();
    } else if (tt == DocPosition::Tax::Reduced) {
        return reducedTaxPercentNum();
    }
    return QString();

}

QString KraftDoc::fullTaxPercentNum() const
{
    double t = _fullTax;
    return QString::number(t, 'f', 2);
}

QString KraftDoc::reducedTaxPercentNum() const
{
    double t = _redTax;
    return QString::number(t, 'f', 2);
}

QString KraftDoc::fullTaxPercentStr() const
{
   return Format::localeDoubleToString(_fullTax, *DefaultProvider::self()->locale());
}

QString KraftDoc::reducedTaxPercentStr() const
{
   return Format::localeDoubleToString(_redTax, *DefaultProvider::self()->locale());
}


QString KraftDoc::country() const
{
    QLocale *loc = DefaultProvider::self()->locale();
    return loc->territoryToString(loc->territory());
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

bool KraftDoc::isInvoice() const
{
    // This is just a work around and should be fixed with an attribute for the doctype
    // at some point.
    // FIXME - this is not cool.
    return (docType() == QStringLiteral("Rechnung"));
}

bool KraftDoc::isDraftState() const
{
    return (_state.state() == KraftDocState::State::Draft);
}

QList<ReportItem*> KraftDoc::reportItemList() const
{
    // ReportItemList reList(positions());
    QList<ReportItem*> list;

    for( auto &pos : positions()) {
        ReportItem *ri = new ReportItem(pos);
        list.append(ri);
    }

    return list;
}

void KraftDoc::finalize()
{
    auto nowState = state().state();

    if (nowState == KraftDocState::State::Final) {
        qDebug() << "Document is already in final state";
        return;
    }

    QList<KraftDocState::State> allowed = KraftDocState::validFollowStates(nowState);
    if (!allowed.contains(KraftDocState::State::Final)) {
        qDebug() << "Document is in wrong state to be finalized" << state().stateString();
        return;
    }

    DocIdentGenerator *gen = new DocIdentGenerator;
    connect(gen, &DocIdentGenerator::newIdent, this, &KraftDoc::slotNewIdent);
    gen->generate(this);

}

void KraftDoc::slotNewIdent(const QString& ident)
{
    DocumentMan *man = DocumentMan::self();
    auto generator = qobject_cast<DocIdentGenerator*>(sender());

    if (ident.isEmpty()) {
        const QString errStr = generator->errorStr();

        // Error state - FIXME: Somehow display the error
        man->setDocProcessingError(errStr);
    } else {
        // a new ident is here. Lets set it and save the doc.
        setIdent(ident);
        state().setState(KraftDocState::State::Final);

        man->saveDocument(this);
    }
    delete generator;
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
 QString KraftDoc::resolveMacros(const QString& txtWithMacros, const DocPositionList dposList, const QDate& date, double fullTax, double redTax, const QString& dateFormat) const
 {
     QString myStr{txtWithMacros};
     QMap<QString, int> seenTags;

     QRegularExpression rxIf("\\s{1}IF_ANY_HAS_TAG\\(\\s*(\\w+)\\s*\\)");
     QRegularExpression rxEndif("\\s{1}END_HAS_TAG");
     QRegularExpression rxAmount("ITEM_COUNT_WITH_TAG\\(\\s*(\\w+)\\s*\\)");
     QRegularExpression rxAddDate("DATE_ADD_DAYS\\(\\s*(\\-{0,1}\\d+)\\s*\\)");
     // look for tag SUM_PER_TAG( HNDL )
     QRegularExpression rx("NETTO_SUM_PER_TAG\\(\\s*(\\w+)\\s*\\)");
     QRegularExpression rxBrutto("BRUTTO_SUM_PER_TAG\\(\\s*(\\w+)\\s*\\)");
     QRegularExpression rxVat("VAT_SUM_PER_TAG\\(\\s*(\\w+)\\s*\\)");

     int pos{0};
     QMap<QString, Geld> bruttoSums;
     QMap<QString, Geld> vatSums;
     Geld nettoSum;
     QRegularExpressionMatch match;

    while((pos = myStr.indexOf(rx, pos, &match)) > -1) {
        const QString lookupTag = match.captured(1);
        bruttoSums[lookupTag] = Geld();
        vatSums[lookupTag] = Geld();

        for (DocPosition *pb : dposList) {
            if (!pb->toDelete() && pb->hasTag(lookupTag)) {
                Geld netto = pb->overallPrice();

                Geld tax;
                if (pb->taxType() == DocPosition::Tax::Full)
                    tax = netto.percent(fullTax);
                else if (pb->taxType() == DocPosition::Tax::Reduced)
                    tax = netto.percent(redTax);

                bruttoSums[lookupTag] += netto;
                bruttoSums[lookupTag] += tax;
                vatSums[lookupTag] += tax;
                nettoSum += netto;
            }
        }

        myStr.replace(pos, match.captured().length(), nettoSum.toLocaleString());
    }

    // replace the Brutto- and vat tags if exist
     pos = 0;
     while ((pos = myStr.indexOf(rxBrutto, pos, &match)) > -1) {
         const QString lookupTag = match.captured(1);
         if (bruttoSums.contains(lookupTag)) {
             myStr.replace(pos, match.captured().length(), bruttoSums[lookupTag].toLocaleString());
         } else {
             qDebug() << "No Brutto sums computed for" << lookupTag;
         }
     }

     pos = 0;
     while ((pos = myStr.indexOf(rxVat, pos, &match))> -1) {
         const QString lookupTag = match.captured(1);
         if (vatSums.contains(lookupTag)) {
             myStr.replace(pos, match.captured().length(), vatSums[lookupTag].toLocaleString());
         }

     }


     // generate a list of all tags in any position
     for (DocPosition *pb : dposList) {
         if (!pb->toDelete()) {
             const auto tags = pb->allTags();
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
     while ((pos = myStr.indexOf(rxAmount, pos, &match))> -1) {
         const QString lookupTag = match.captured(1);
         int amount{0};
         if (seenTags.contains(lookupTag)) {
             amount = seenTags[lookupTag];
         }
         myStr.replace(pos, match.captured().length(), QString::number(amount));
     }

     pos = 0;
     while ((pos = myStr.indexOf(rxAddDate, pos, &match))> -1) {
         const QString addDaysStr = match.captured(1);
         qint64 addDays = addDaysStr.toInt();
         QDate newDate = date.addDays(addDays);
         const QString newDateStr = Format::toDateString(newDate, dateFormat.isEmpty() ? KraftSettings::self()->dateFormat() : dateFormat);
         myStr.replace(pos, match.captured().length(), newDateStr);
     }

     // IF_ANY_HAS_TAG(tag) ..... END_HAS_TAG
     // check the IF_HAS_TAG(tag) ... END_HAS_TAG macro
     pos = 0;
     while ((pos = myStr.indexOf(rxIf, pos, &match))> -1) {
         const QString lookupTag = match.captured(1);
         int endpos = myStr.lastIndexOf(rxEndif);
         if (endpos == -1) endpos = myStr.length();
         if (seenTags.contains(lookupTag)) {
             myStr.remove(endpos, 12 /* length of END_HAS_TAG */);
             myStr.remove(pos, match.captured().length());
         } else {
             // the tag was not seen, so this needs to be deleted.
             int len = endpos-pos+12;
             myStr.remove(pos, len);
         }
     }
     return myStr;
 }

 QString KraftDoc::dateStr() const
 {
     return Format::toDateString(mDate, KraftSettings::self()->dateFormat());
 }

 QString KraftDoc::dateStrISO() const
 {
     return mDate.toString("yyyy-MM-dd");
 }

 QString KraftDoc::preText() const
 {
     double fullTax = UnitManager::self()->tax(date());
     double redTax = UnitManager::self()->reducedTax(date());
     const QString myStr = resolveMacros(mPreText, positions(), date(), fullTax, redTax);
     return myStr;
 }

 QString KraftDoc::postText() const
 {
     double fullTax = UnitManager::self()->tax(date());
     double redTax = UnitManager::self()->reducedTax(date());
     const QString myStr = resolveMacros(mPostText, positions(), date(), fullTax, redTax);
     return myStr;
 }

 QString KraftDoc::preTextHtml() const
 {
     return multilineHtml(mPreText);
 }

 QString KraftDoc::postTextHtml() const
 {
     return multilineHtml(mPostText);
 }
