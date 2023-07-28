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

#include <QString>
#include <QSqlQuery>
#include <QDateTime>
#include <QLocale>
#include <QDebug>
#include <QRegExp>

#include <KLocalizedString>

// application specific includes
#include "archiveman.h"
#include "archdoc.h"
#include "documentman.h"
#include "kraftdb.h"
#include "defaultprovider.h"
#include "kraftsettings.h"
#include "format.h"

namespace {

QString multilineHtml( const QString& str )
{
    QString re {str.toHtmlEscaped()};

    re.replace( '\n', "<br/>");
    return re;
}

} // end namespace

ArchDoc::ArchDoc()
    : mAttributes( QLatin1String("ArchDoc"))
{

}

ArchDoc::ArchDoc( const dbID& id )
    : mAttributes( QLatin1String("ArchDoc"))
{
    /* load archive from database */
    loadFromDb( id );
}

QString ArchDoc::docIdentifier() const
{
  QString re = docTypeStr();

  return i18n("%1 for %2 (Id %3)", re, ident() );
}

QString ArchDoc::dateStr() const
{
    return Format::toDateString(mDate, KraftSettings::self()->dateFormat());
}

QString ArchDoc::dateStrISO() const
{
    return mDate.toString("yyyy-MM-dd");
}

QString ArchDoc::preText() const
{
    return mPreText;
}

QString ArchDoc::preTextHtml() const
{
    return multilineHtml(mPreText);
}

QString ArchDoc::postText() const
{
    return mPostText;
}

QString ArchDoc::postTextHtml() const
{
    return multilineHtml(mPostText);
}


Geld ArchDoc::nettoSum() const
{
    const Geld g = positions().sumPrice();
    return g;
}

Geld ArchDoc::bruttoSum() const
{
    Geld g = nettoSum();
    const Geld ts = taxSum();
    g += ts;
    return g;
}

Geld ArchDoc::taxSum() const
{
    const Geld g = positions().taxSum();
    return  g;
}

Geld ArchDoc::fullTaxSum() const
{
    return positions().fullTaxSum();
}

Geld ArchDoc::reducedTaxSum() const
{
    return positions().reducedTaxSum();
}

QString ArchDoc::fullTaxPercentStr() const
{
   return Format::localeDoubleToString(mTax, *DefaultProvider::self()->locale());
}

QString ArchDoc::reducedTaxPercentStr() const
{
   return Format::localeDoubleToString(mReducedTax, *DefaultProvider::self()->locale());
}

QString ArchDoc::taxPercentStr() const
{
     DocPositionBase::TaxType tt = mPositions.listTaxation();
     if (tt == DocPositionBase::TaxType::TaxFull) {
         return fullTaxPercentStr();
     } else if (tt == DocPositionBase::TaxType::TaxReduced) {
         return reducedTaxPercentStr();
     }
     return QString();
}

QString ArchDoc::taxPercentNum() const
{
    DocPositionBase::TaxType tt = mPositions.listTaxation();
    if (tt == DocPositionBase::TaxType::TaxFull) {
        return fullTaxPercentNum();
    } else if (tt == DocPositionBase::TaxType::TaxReduced) {
        return reducedTaxPercentNum();
    }
    return QString();

}

QString ArchDoc::fullTaxPercentNum() const
{
    double t = tax();
    return QString::number(t, 'f', 2);
}

QString ArchDoc::reducedTaxPercentNum() const
{
    double t = reducedTax();
    return QString::number(t, 'f', 2);
}

double ArchDoc::tax() const
{
    return mTax;
}

double ArchDoc::reducedTax() const
{
    return mReducedTax;
}

void ArchDoc::loadFromDb( dbID id )
{
  mArchDocID = id;

  QSqlQuery q;
  q.prepare("SELECT archDocID, ident, docType, clientAddress, clientUid, " // pos 0..4
            "salut, goodbye, printDate, date, pretext, posttext, country, language, " // pos 5..12
            "projectLabel, predecessor, tax, reducedTax, state from archdoc WHERE archDocID=:id" ); // pos 13..17
  q.bindValue(":id", id.toInt());
  q.exec();

  // qDebug () << "Loading document id " << id.toString();

  if( q.next()) {
    QString docID;
    QString country;
    QString lang;
    docID         = q.value( 0 ).toString();
    mIdent        = q.value( 1 ).toString();
    mDocTypeStr   = q.value( 2 ).toString();
    mAddress      = q.value( 3 ).toString();
    mClientUid    = q.value( 4 ).toString();
    mSalut        = q.value( 5 ).toString();
    mGoodbye      = q.value( 6 ).toString();
    QVariant v    = q.value( 7 );
    mPrintDate    = v.toDateTime();
    mDate         = q.value( 8 ).toDate();
    mPreText      = KraftDB::self()->mysqlEuroDecode( q.value( 9 ).toString() );
    mPostText     = KraftDB::self()->mysqlEuroDecode( q.value( 10 ).toString() );
    country       = q.value( 11 ).toString();
    lang          = q.value( 12 ).toString();
    mProjectLabel = q.value( 13 ).toString();
    mPredecessor  = q.value( 14 ).toString();
    mTax          = q.value( 15 ).toDouble();
    mReducedTax   = q.value( 16 ).toDouble();
    mState        = q.value( 17 ).toInt();

    loadItems( docID );

    mAttributes.load(id);
  } else {
    // qDebug () << "ERR: Could not load archived doc with id " << id.toString();
  }
}

void ArchDoc::loadItems( const QString& archDocId )
{
  mPositions.clear();

  if ( archDocId.isEmpty() /* || ! archDocId.isNum() */ ) {
    // qDebug () << "ArchDocId is not crappy: " << archDocId;
    return;
  }

  QSqlQuery q;
  q.prepare("SELECT archPosID, archDocID, ordNumber, kind, postype, text, amount, " // pos 0..6
            "unit, price, overallPrice, taxType FROM archdocpos WHERE archDocID=:id ORDER BY ordNumber"); // pos 7..10
  q.bindValue(":id", archDocId);
  if( !q.exec() ) {
      qDebug() << "Error: " << q.lastError().nativeErrorCode();
  }

  mPositions.setTaxes(mTax, mReducedTax);

  while( q.next() ) {
    ArchDocPosition pos;
    pos.mPosNo = q.value( 2 ).toString();
    pos.mKind = q.value( 3 ).toString();
    pos.mText = q.value( 5 ).toString();
    pos.mAmount = q.value( 6 ).toDouble();
    pos.mUnit  = q.value( 7 ).toString();
    pos.mUnitPrice = Geld( q.value( 8 ).toDouble() );
    pos.mOverallPrice = q.value( 9 ).toDouble();

    int tt = q.value( 10 ).toInt();
    if ( tt == 1 )
      pos.mTaxType = DocPositionBase::TaxNone;
    else if ( tt == 2 )
      pos.mTaxType = DocPositionBase::TaxReduced;
    else if ( tt == 3 )
      pos.mTaxType = DocPositionBase::TaxFull;

    mPositions.append( pos );
  }
}

QList<ArchDocPosition> ArchDoc::itemslist() const
{
    return mPositions;
}

QDateTime ArchDoc::sentOutDate()
{
    QDateTime re;

    if ( mAttributes.hasAttribute( SentOutDateC ) ) {
        re = mAttributes["sentOutDate"].value().toDateTime();
    }
    return re;
}

void ArchDoc::setSentOutDate( const QDateTime& dt )
{
    QString attName(SentOutDateC);
    if( dt.isValid() ) {
        Attribute att(attName);
        att.setPersistant(true);
        att.setValue(dt);
        mAttributes[attName] = att;
    } else {
        mAttributes.markDelete(attName);
    }
    mAttributes.save(mArchDocID);
}

ArchDocDigest ArchDoc::toDigest() const
{
    return ArchDocDigest(mPrintDate, mState, mIdent, mDocTypeStr, mArchDocID);
}

/* ###################################################################### */

ArchDocDigest::ArchDocDigest()
{

}

ArchDocDigest::ArchDocDigest(QDateTime dt,  int s, const QString& ident, const QString & docType, dbID id )
  : mPrintDate( dt ),
    mState( s ),
    mArchDocId( id ),
    mIdent( ident ),
    mDocTypeStr(docType)
{

}

ArchDocDigest::~ArchDocDigest()
{

}

QString ArchDocDigest::pdfArchiveFileName() const
{
    const QString outputDir = ArchiveMan::self()->pdfBaseDir();
    const QString filename = ArchiveMan::self()->archiveFileName(archDocIdent(),
                                                           archDocId().toString(), "pdf" );
    const QString file = QString( "%1/%2" ).arg( outputDir ).arg( filename );

    return file;
}

bool ArchDocDigest::isInvoice() const
{
    // This is just a work around and should be fixed with an attribute for the doctype
    // at some point.
    return (mDocTypeStr == QStringLiteral("Rechnung"));
}

/* ###################################################################### */


