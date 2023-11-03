/***************************************************************************
                     kraftdoc.h - Kraft document class
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

#ifndef KRAFTDOC_H
#define KRAFTDOC_H

// include files for QT
#include <qobject.h>
#include <qstring.h>
#include <qdatetime.h>

#include "docposition.h"
#include "dbids.h"
#include "kraftobj.h"
#include "reportitemlist.h"

// forward declaration of the Kraft classes

class DocumentSaverBase;
class Geld;
class DocDigest;

class KraftView;

class KraftDocState
{
public:
    enum class State {
        Undefined,  // Not defined at all.
        New,        // New document, not yet saved
        Draft,      // Draft. Saved on disk
        Final,      // Final. Saved and with an official doc number
        Retracted,  // Sent to customer, but retracted
        Converted,  // Converted from a previous Kraft version that did not have proper states
        Invalid     // Invalidated. Never sent out
    };

    const QString StateUndefinedStr{"Undefined"};
    const QString StateNewStr{"New"};
    const QString StateDraftStr{"Draft"};
    const QString StateFinalStr{"Final"};
    const QString StateRetractedStr{"Retracted"};
    const QString StateInvalidStr{"Invalid"};
    const QString StateConvertedStr{"Converted"};

    KraftDocState::State state() const { return _state; }
    QString stateString() const;
    void setState( State s) { _state = s; }
    void setStateFromString(const QString& s);
    bool isNew() const { return _state == State::New; }
    QList<KraftDocState::State> validFollowStates(KraftDocState::State nowState);

private:
    State _state;
};

class KraftDoc : public QObject, public KraftObj
{
    Q_OBJECT
    Q_PROPERTY(QString docType READ docType)
    Q_PROPERTY(QString address READ address)
    Q_PROPERTY(QString clientUid READ addressUid)
    Q_PROPERTY(QString ident READ ident)
    Q_PROPERTY(QString salut READ salut)
    Q_PROPERTY(QString goodbye READ goodbye)
    Q_PROPERTY(QString preText READ preText)
    Q_PROPERTY(QString preTextHtml READ preTextHtml)
    Q_PROPERTY(QString postText READ postText)
    Q_PROPERTY(QString postTextHtml READ postTextHtml)

    Q_PROPERTY(QString projectLabel READ projectLabel)
    Q_PROPERTY(QString predecessor READ predecessor)
    Q_PROPERTY(QString docIDStr READ uuid)

    Q_PROPERTY(QString docIdentifier READ docIdentifier)
    Q_PROPERTY(QString dateStr READ dateStr)
    Q_PROPERTY(QString dateStrISO READ dateStrISO)

    Q_PROPERTY(QString nettoSumStr READ nettoSumStr)
    Q_PROPERTY(QString nettoSumNum READ nettoSumNum)

    Q_PROPERTY(QString bruttoSumStr READ bruttoSumStr)
    Q_PROPERTY(QString bruttoSumNum READ bruttoSumNum)

    Q_PROPERTY(QString taxSumStr READ vatSumStr)
    Q_PROPERTY(QString taxSumNum READ vatSumNum)

    Q_PROPERTY(QString fullTaxSumStr READ fullTaxSumStr)
    Q_PROPERTY(QString fullTaxSumNum READ fullTaxSumNum)

    Q_PROPERTY(QString reducedTaxSumStr READ reducedTaxSumStr)
    Q_PROPERTY(QString reducedTaxSumNum READ reducedTaxSumNum)

    Q_PROPERTY(QString owner READ owner)

    Q_PROPERTY(QString dueDateStrISO READ dueDate)
    Q_PROPERTY(QString buyerReference READ buyerRef)

    Q_PROPERTY(QString fullTaxPercentNum READ fullTaxPercentNum)
    Q_PROPERTY(QString fullTaxPercentStr READ fullTaxPercentStr)
    Q_PROPERTY(QString reducedTaxPercentNum READ reducedTaxPercentNum)
    Q_PROPERTY(QString reducedTaxPercentStr READ reducedTaxPercentStr)
    Q_PROPERTY(QString taxPercentStr READ taxPercentStr)
    Q_PROPERTY(QString taxPercentNum READ taxPercentNum)

    Q_PROPERTY(QString taxMarkerFull READ taxMarkerFull)
    Q_PROPERTY(QString taxMarkerReduced READ taxMarkerReduced)

    Q_PROPERTY(QList<ReportItem*> reportItems READ reportItemList)
    Q_PROPERTY(bool hasIndividualTaxation READ hasIndividualTaxation)
    Q_PROPERTY(bool isInvoice READ isInvoice)

public:
    enum class Part { Header,  Positions, Footer, Unknown };
    static QString partToString( Part );

    /** Constructor for the fileclass of the application */
    KraftDoc(QWidget *parent = nullptr);
    /** Destructor for the fileclass of the application */
    ~KraftDoc();

    KraftDoc& operator=( KraftDoc& );

    DocPosition* createPosition( DocPositionBase::PositionType t = DocPositionBase::Position );
    DocPositionList positions() const { return mPositions; }
    void setPositionList(DocPositionList , bool isNew = false);
    QList<ReportItem*> reportItemList() const;

    DocDigest toDigest();

    QDate date() const { return mDate; }
    void setDate( QDate d ) { mDate = d; }
    QString dateStr() const;
    QString dateStrISO() const;

    QString docType() const { return mDocType; }
    void setDocType( const QString& s );
    bool docTypeChanged() { return mDocTypeChanged; }

    QString addressUid() const { return mAddressUid; }
    void setAddressUid( const QString& id ) { mAddressUid = id; }

    QString address() const { return mAddress; }
    void setAddress( const QString& adr ) { mAddress = adr; }

    QString ident() const   { return mIdent; }
    void setIdent( const QString& str ) { mIdent = str; }

    QString salut() const   { return mSalut;    }
    void setSalut( const QString& str ) { mSalut = str; }

    QString goodbye() const   { return mGoodbye;    }
    void setGoodbye( const QString& str ) { mGoodbye = str; }

    // take a string with macros and generate the text replacements for the macros
    // with the help of the position list
    QString resolveMacros(const QString& txtWithMacros, const DocPositionList dposList, const QDate &date, double fullTax, double redTax) const;

    // preText is the variant with expanded macros
    QString preText() const;
    QString preTextHtml() const;
    // preTextRaw is the variant with macros not expanded
    QString preTextRaw() const;
    void setPreTextRaw( const QString& str ) { mPreText = str; }

    QString postText() const;
    QString postTextHtml() const;
    // postTextRaw is the variant with macros not expanded
    QString postTextRaw() const;
    void setPostTextRaw( const QString& str ) { mPostText = str; }

    QString whiteboard() const { return mWhiteboard; }
    void setWhiteboard( const QString& w ) { mWhiteboard = w; }

    QString projectLabel() const { return mProjectLabel; }
    void setProjectLabel( const QString& w ) { mProjectLabel = w; }

    QString predecessor() const { return mPredecessor; }
    void setPredecessor( const QString& w );
    QString predecessorDbId() const { return mPredecessorDbId; }
    void setPredecessorDbId( const QString& pId ) { mPredecessorDbId = pId; }

    void setTimeOfSupply(QDateTime start, QDateTime end = QDateTime());
    QDateTime timeOfSupplyStart() { return _toSStart; }
    QDateTime timeOfSupplyEnd() { return _toSEnd; }

    QString owner() { return _owner; }
    void setOwner(const QString& owner) { _owner = owner; }

    QString docIdentifier() const;
    DBIdList removePositionList() { return mRemovePositions; }

    QString taxPercentStr() const;
    QString taxPercentNum() const;
    QString reducedTaxPercentStr() const;
    QString reducedTaxPercentNum() const;
    QString fullTaxPercentStr() const;
    QString fullTaxPercentNum() const;

    Geld nettoSum() const;
    QString nettoSumStr() const { return nettoSum().toLocaleString(); }
    QString nettoSumNum() const { return nettoSum().toNumberString(); }

    Geld bruttoSum() const;
    QString bruttoSumStr() const { return bruttoSum().toLocaleString(); }
    QString bruttoSumNum() const { return bruttoSum().toNumberString(); }

    Geld fullTaxSum() const;
    QString fullTaxSumStr() const { return fullTaxSum().toLocaleString(); }
    QString fullTaxSumNum() const { return fullTaxSum().toNumberString(); }

    Geld reducedTaxSum() const;
    QString reducedTaxSumStr() const { return reducedTaxSum().toLocaleString(); }
    QString reducedTaxSumNum() const { return reducedTaxSum().toNumberString(); }

    Geld vatSum() const;
    QString vatSumStr() const { return vatSum().toLocaleString(); }
    QString vatSumNum() const { return vatSum().toNumberString(); }

    QString country() const;
    QString language() const;

    QString dueDate() const { return _dueDate.toString("yyyy-MM-dd"); }
    QString buyerRef() const { return _buyerRef; }
    void setDueDate(const QDate& d) { _dueDate = d; }
    void setBuyerRef(const QString& br) { _buyerRef = br; }

    static QString taxMarkerNoTax()   { return QStringLiteral("1"); }
    static QString taxMarkerReduced() { return QStringLiteral("2"); }
    static QString taxMarkerFull()    { return QStringLiteral("");  }

    bool hasIndividualTaxation() const { return mPositions.hasIndividualTaxes(); }

    bool isInvoice() const;

    void setTaxValues(double fullTax, double redTax);

    void clear();

    KraftDocState& state() { return _state; }

public slots:
    /** calls redrawDocument() on all views connected to the document object and is
   *  called by the view by which the document has been changed.
   *  As this view normally repaints itself, it is excluded from the paintEvent.
   */
    int slotAppendPosition( const DocPosition& );

    // The following slots take get the db id as argument
    void slotRemovePosition( int );
    void slotMoveUpPosition( int );
    void slotMoveDownPosition( int );

protected:
    /** closes the current document FIXME: remove and put to destructor */
    void closeDocument();

    /** loads the document by filename and format and emits the updateViews() signal */
    bool openDocument(DocumentSaverBase &loader, const QString& uuid);

    /** fetch the document from database back FIXME needs a loader */
    bool reloadDocument(DocumentSaverBase &loader);

    /** saves the document under filename and format.*/
    bool saveDocument(DocumentSaverBase &saver);

private:
    /** deletes the document's contents */
    void deleteItems();

    QString mAddressUid;
    QString mProjectLabel;
    QString mAddress;
    // mPreText and postText always have the raw variant without expanded macros
    QString mPreText;
    QString mPostText;
    QString mDocType;
    bool    mDocTypeChanged;
    QString mSalut;
    QString mGoodbye;
    QString mIdent;
    QString mWhiteboard;
    QString mPredecessor;
    QString mPredecessorDbId;

    // Two qualifiers for the locale settings.
    QString mCountry;
    QString mLanguage;

    QDate   mDate;

    QString _buyerRef;
    QDate _dueDate;

    // Time of supply
    QDateTime _toSStart;
    QDateTime _toSEnd;
    QString   _owner;

    DocPositionList mPositions;
    DBIdList mRemovePositions;
    KraftDocState   _state;

    double _fullTax, _redTax;
    friend class DocumentMan;
};

#endif // KraftDoc_H
