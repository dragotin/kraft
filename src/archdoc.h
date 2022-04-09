/***************************************************************************
                          archdoc.h  -
                             -------------------
    begin                : Sep 2006
    copyright            : (C) 2006 by Klaas Freitag
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

#ifndef ARCHDOC_H
#define ARCHDOC_H

// include files for QT
#include <QString>
#include <QDateTime>
#include <QMap>
#include <QObject>

#include "archdocposition.h"
#include "geld.h"
#include "dbids.h"

class QLocale;
class AttributeMap;

class ArchDocDigest
{
public:

    /** Constructor for the fileclass of the application */
    ArchDocDigest();
    ArchDocDigest( QDateTime, int, const QString&, const QString&, dbID );
    /** Destructor for the fileclass of the application */
    virtual ~ArchDocDigest();

    QDateTime printDate() const {
        return mPrintDate;
    }

    int archDocState() const {
        return mState;
    }

    dbID archDocId() const {
        return mArchDocId;
    }

    QString archDocIdent() const {
        return mIdent;
    }

    QString docTypeStr() const {
        return mDocTypeStr;
    }

    QString pdfArchiveFileName() const;

    // FIXME: This is a workaround for the XRechnung - so far only the german
    // doctype "Rechnung" enables the XRechnung support.
    bool hasXRechnungExport() const;

protected:
    QDateTime mPrintDate;
    int       mState;
    dbID      mArchDocId;
    QString   mIdent;
    QString   mDocTypeStr;
};


class ArchDoc: public QObject, public ArchDocDigest
{
    Q_OBJECT

    Q_PROPERTY(QString docType READ docTypeStr)
    Q_PROPERTY(QString address READ address)
    Q_PROPERTY(QString clientUid READ clientUid)
    Q_PROPERTY(QString ident READ ident)
    Q_PROPERTY(QString salut READ salut)
    Q_PROPERTY(QString goodbye READ goodbye)
    Q_PROPERTY(QString preText READ preText)
    Q_PROPERTY(QString preTextHtml READ preTextHtml)
    Q_PROPERTY(QString postText READ postText)
    Q_PROPERTY(QString postTextHtml READ postTextHtml)
    Q_PROPERTY(QString projectLabel READ projectLabel)
    Q_PROPERTY(QString predecessor READ predecessor)
    Q_PROPERTY(QString docIDStr READ archDocIdStr)
    Q_PROPERTY(QString docIdentifier READ docIdentifier)
    Q_PROPERTY(QString dateStr READ dateStr)
    Q_PROPERTY(QString dateStrISO READ dateStrISO)

    Q_PROPERTY(QString nettoSumStr READ nettoSumStr)
    Q_PROPERTY(QString nettoSumNum READ nettoSumNum)
    Q_PROPERTY(QString bruttoSumStr READ bruttoSumStr)
    Q_PROPERTY(QString bruttoSumNum READ bruttoSumNum)
    Q_PROPERTY(QString taxSumStr READ taxSumStr)
    Q_PROPERTY(QString taxSumNum READ taxSumNum)
    Q_PROPERTY(QString fullTaxSumStr READ fullTaxSumStr)
    Q_PROPERTY(QString fullTaxSumNum READ fullTaxSumNum)
    Q_PROPERTY(QString reducedTaxSumStr READ reducedTaxSumStr)
    Q_PROPERTY(QString reducedTaxSumNum READ reducedTaxSumNum)

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

    Q_PROPERTY(QList<ArchDocPosition> items READ itemslist)
    Q_PROPERTY(bool hasIndividualTaxation READ hasIndividualTaxation)

public:

    const QString SentOutDateC {"SentOutDate"};

    /** Constructor for the fileclass of the application */
    ArchDoc();
    ArchDoc( const dbID& );
    /** Destructor for the fileclass of the application */
    virtual ~ArchDoc() {};

    ArchDocPositionList positions() const { return mPositions; }
    QList<ArchDocPosition> itemslist() const;

    QDate date() const      { return mDate; }
    QString dateStr() const;
    QString dateStrISO() const;

    QString address() const { return mAddress; }

    QString clientUid() const { return mClientUid; }

    QString ident() const { return mIdent;    }

    QString salut() const { return mSalut;    }

    QString goodbye() const { return mGoodbye;    }

    QString preText() const;
    QString preTextHtml() const;

    QString postText() const;
    QString postTextHtml() const;

    QString projectLabel() const { return mProjectLabel; }
    QString predecessor() const { return mPredecessor; }

    QString archDocIdStr() const { return archDocId().toString(); }

    QString docIdentifier() const;

    Geld nettoSum() const;
    QString nettoSumStr() const { return nettoSum().toLocaleString(); }
    QString nettoSumNum() const { return nettoSum().toNumberString(); }

    Geld bruttoSum() const;
    QString bruttoSumStr() const { return bruttoSum().toLocaleString(); }
    QString bruttoSumNum() const { return bruttoSum().toNumberString(); }

    Geld taxSum() const;
    QString taxSumStr() const { return taxSum().toLocaleString(); }
    QString taxSumNum() const { return taxSum().toNumberString(); }
    Geld fullTaxSum() const;
    QString fullTaxSumStr() const { return fullTaxSum().toLocaleString(); }
    QString fullTaxSumNum() const { return fullTaxSum().toNumberString(); }
    Geld reducedTaxSum() const;
    QString reducedTaxSumStr() const { return reducedTaxSum().toLocaleString(); }
    QString reducedTaxSumNum() const { return reducedTaxSum().toNumberString(); }

    QString fullTaxPercentNum() const;
    QString fullTaxPercentStr() const;
    QString reducedTaxPercentNum() const;
    QString reducedTaxPercentStr() const;
    QString taxPercentStr() const;
    QString taxPercentNum() const;

    QString dueDate() const { return _dueDate.toString("yyyy-MM-dd"); }
    QString buyerRef() const { return _buyerRef; }

    void setDueDate(const QDate& d) { _dueDate = d; }
    void setBuyerRef(const QString& br) { _buyerRef = br; }

    static QString taxMarkerNoTax()   { return QStringLiteral("1"); }
    static QString taxMarkerReduced() { return QStringLiteral("2"); }
    static QString taxMarkerFull()    { return QStringLiteral("");  }

    bool hasIndividualTaxation() const { return mPositions.hasIndividualTaxes(); }

    double tax() const;
    double reducedTax() const;

    ArchDocDigest toDigest() const;

    // when the document was sent to the customer.
    QDateTime sentOutDate();
    void setSentOutDate( const QDateTime& dt );

    void loadFromDb( dbID );

protected:
    void loadItems( const QString& );

    dbID mArchDocID;
    QString mAddress;
    QString mClientUid;
    QString mPreText;
    QString mPostText;
    QString mSalut;
    QString mGoodbye;
    QString mIdent;
    QString mProjectLabel;
    QString mPredecessor;

    QString _buyerRef;
    QDate _dueDate;

    double  mTax;
    double  mReducedTax;

    QDate     mDate;

    ArchDocPositionList mPositions;
    AttributeMap mAttributes;
};



#endif // ARCHDOC_H
