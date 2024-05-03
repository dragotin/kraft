/***************************************************************************
                 doctype.cpp - doc type class
                             -------------------
    begin                : Oct. 2007
    copyright            : (C) 2007 by Klaas Freitag
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
#include <QtCore>
#include <QSqlQuery>

// application specific includes
#include "doctype.h"
#include "kraftdb.h"
#include "numbercycle.h"
#include "attribute.h"
#include "defaultprovider.h"
#include "stringutil.h"

/**
@author Klaas Freitag
*/

namespace {
const QString XRechnungTmplStr   {"XRechnungTmpl"};
const QString WatermarkFileStr   {"watermarkFile"};
const QString DocTemplateFileStr {"docTemplateFile"};
const QString IdentNumberCycleStr{"identNumberCycle"};
const QString DocMergeIdentStr   {"docMergeIdent"};
const QString DayCounterDateStr  {"dayCounterDate"};
const QString DayCounterStr  {"dayCounter"};
const QString AppendPDFStr   {"AppendPDFFile"};
const QString DefaultTmplFileName {"invoice.trml"};
const QString XRechnungEnabled {"XRechnungEnabled"};
}


idMap DocType::mNameMap = idMap();

DocType::DocType()
    : mAttributes( QStringLiteral( "DocType" ) ),
      mDirty( false )
{
    init();
}

DocType::DocType( const QString& name, bool dirty )
    : mAttributes( QStringLiteral( "DocType" ) ),
      mName( name ),
      mDirty( dirty )
{
    init();
    if ( mNameMap.contains( name ) ) {
        dbID id = mNameMap[ name ];

        mAttributes.load( id );
    }

    readFollowerList();
    readIdentTemplate();
}

void DocType::init()
{
    // === Start to fill static content
    if ( ! mNameMap.empty() ) return;

    QSqlQuery q;
    q.prepare( "SELECT docTypeID, name FROM DocTypes ORDER BY name" );
    q.exec();

    while ( q.next() ) {
        dbID id( q.value(0).toInt() );
        QString name = q.value(1).toString();

        mNameMap[ name ] = id;
        // QString h = DefaultProvider::self()->locale()->translate( cur.value( "name" ).toString() );
    }
}

void DocType::clearMap()
{
    mNameMap.clear();
}

QStringList DocType::all()
{
    init();

    QStringList re;

    QSqlQuery q;
    q.prepare( "SELECT docTypeID, name FROM DocTypes ORDER BY name" );
    q.exec();

    while ( q.next() ) {
        re << q.value(1).toString();
    }

    return re;
}

QStringList DocType::allLocalised()
{
    return all();
}

// static function to retrieve id of a certain doctype
dbID DocType::docTypeId( const QString& docType )
{
    dbID id;
    init();
    if ( mNameMap.contains( docType ) ) {
        id = mNameMap[ docType ];

        return id;
    } else {
        qCritical()<< "Can not find id for doctype named " << docType;
    }
    return id;
}

bool DocType::allowDemand()
{
    bool re = false;

    if ( mAttributes.contains( "AllowDemand" ) ) {
        re = true;
    }
    return re;
}

bool DocType::allowAlternative()
{
    bool re = false;

    if ( mAttributes.contains( "AllowAlternative" ) ) {
        re = true;
    }
    return re;
}

bool DocType::pricesVisible()
{
    bool re = true;
    if( mAttributes.contains("HidePrices")) {
        re = false;
    }
    return re;
}

bool DocType::substractPartialInvoice()
{
    bool re = false;
    if( mAttributes.contains("SubstractPartialInvoice")) {
        re = true;
    }
    return re;
}

bool DocType::partialInvoice()
{
    bool re = false;
    if( mAttributes.contains("PartialInvoice")) {
        re = true;
    }
    return re;
}

// returns the amount of followers added
int DocType::setAllFollowers( const QStringList& followers)
{
    QSqlQuery q;
    q.prepare("INSERT INTO DocTypeRelations (typeId, followerId, sequence) VALUES (:typeId, :followerId, 0)");
    QSqlQuery qu;
    qu.prepare("UPDATE DocTypeRelations SET sequence=:seq WHERE typeId=:typeId AND followerId=:followerId");

    // get "my" doc type Id
    int typeId = mNameMap[mName].toInt();
    q.bindValue(":typeId", typeId);
    qu.bindValue(":typeId", typeId);

    // get the max sequence for me
    int seq = 0;
    {
        QSqlQuery cq;
        cq.prepare("SELECT MAX(sequence) FROM DocTypeRelations WHERE typeId=:tdId");
        cq.bindValue(":tdId", typeId);
        cq.exec();
        if( cq.next() ) {
            seq = cq.value(0).toInt();
        }
    }

    const QStringList existingFollowers = follower();
    int cnt = 0; // simple counter to return.
    for( const QString& f : followers ) {
        if( mNameMap.contains(f) ) {
            int followerId = mNameMap[f].toInt();
            if( !existingFollowers.contains(f) ) {
                q.bindValue(":followerId", followerId );
                q.exec();
                cnt++;
            }
            // use the updater
            qu.bindValue(":seq", ++seq);
            qu.bindValue(":followerId", followerId);
            qu.exec();
        }
    }
    return cnt;
}


QStringList DocType::follower()
{
    return mFollowerList;
}

void DocType::readFollowerList()
{
    QSqlQuery q;
    q.prepare( "SELECT typeId, followerId, sequence FROM DocTypeRelations WHERE typeId=:type ORDER BY sequence");
    q.bindValue( ":type", mNameMap[mName].toInt() );
    q.exec();

    while ( q.next() ) {
        dbID followerId( q.value(1).toInt() );

        idMap::Iterator it;
        for ( it = mNameMap.begin(); it != mNameMap.end(); ++it ) {
            if ( it.value() == followerId ) {
                mFollowerList << it.key();
            }
        }
    }
}

QString DocType::numberCycleName()
{
    QString re = NumberCycle::defaultName();
    if ( mAttributes.hasAttribute(IdentNumberCycleStr) ) {
        re = mAttributes[IdentNumberCycleStr].value().toString();
    }
    return re;
}

void DocType::setNumberCycleName( const QString& name )
{
    if ( name.isEmpty() ) return;

    if ( name != NumberCycle::defaultName() ) {
        Attribute att(IdentNumberCycleStr);
        att.setPersistant( true );
        att.setValue( name );
        mAttributes[IdentNumberCycleStr] = att;
    } else {
        // remove default value from map
        mAttributes.markDelete(IdentNumberCycleStr);
        // qDebug () << "Removing identNumberCycle Attribute";
    }
    mDirty = true;
    readIdentTemplate();
}

/* This method looks for the template file for the doctype. The rule is:
 * 1. Set the filename to look for to the name of the doc type, lowercase
 *    and with spaces replaced
 * 2. Check for an attribute for this doc type.
 *    - if that is an absolute path, return it.
 *    - if not absolute, set the filename to look for to that value
 * 3. Look for the name in KRAFT_HOME
 * 4. Look for the name in rel. system Path
 * 5. Look for the name in QStandardPaths
 * 6. If still empty, fall back to default invoice.trml - which also
 *    sets ReportLab as default
 */

QString DocType::templateFile()
{
    QString tmplFile;
    const auto dfp = DefaultProvider::self();
    QString searchStr;

    QString reportFileName = QString( "%1.trml").arg( name().toLower() );
    reportFileName.replace(QChar(' '), QChar('_'));

    if ( mAttributes.hasAttribute(DocTemplateFileStr) ) {
        tmplFile = mAttributes[DocTemplateFileStr].value().toString();

        qDebug() << "Template File:" << tmplFile;
        if( !tmplFile.isEmpty() ) {
            QFileInfo fi(tmplFile);
            if( fi.isAbsolute() ) {
                return tmplFile;
            } else {
                // it is not an absolute file name, try to find it
                reportFileName = tmplFile;
            }
            tmplFile.clear();
        }
    }    

    // check for reportlab template
    if (tmplFile.isEmpty())  {
        searchStr = QString("reports/%1.trml").arg(reportFileName);
        tmplFile = dfp->locateFile(searchStr);
    }

    // check for weasyprint template
    if (tmplFile.isEmpty())  {
        searchStr = QString("reports/%1.gtmpl").arg(reportFileName);
        tmplFile = dfp->locateFile(searchStr);
    }

    // not found - check invoice.trml
    if (tmplFile.isEmpty())  {
        searchStr = QString("reports/%1").arg(DefaultTmplFileName);
        tmplFile = dfp->locateFile(searchStr);
    }

    if( tmplFile.isEmpty() ) {
        qDebug () << "unable to find a template file for " << name();
    } else {
        qDebug () << "Found template file " << tmplFile;
    }
    return tmplFile;
}

void DocType::setTemplateFile( const QString& name )
{
    if ( name.isEmpty() || name == DefaultTmplFileName) { // the default is returned anyway.
        // remove default value from map
        mAttributes.markDelete(DocTemplateFileStr);
        // qDebug () << "Removing docTemplateFile Attribute";
    } else {
        Attribute att(DocTemplateFileStr);
        att.setPersistant( true );
        att.setValue( name );
        mAttributes[DocTemplateFileStr] = att;
    }
    mDirty = true;
}

QString DocType::mergeIdent()
{
    QString re = "0";
    if ( mAttributes.hasAttribute(DocMergeIdentStr) ) {
        re = mAttributes[DocMergeIdentStr].value().toString();
    }

    return re;
}

void DocType::setMergeIdent( const QString& ident )
{
    if ( !ident.isEmpty() ) {
        Attribute att(DocMergeIdentStr);
        att.setPersistant( true );
        att.setValue( ident );
        mAttributes[DocMergeIdentStr] = att;
    } else {
        // remove default value from map
        mAttributes.markDelete(DocMergeIdentStr);
        // qDebug () << "Removing docMergeIdent Attribute";
    }
    mDirty = true;

}

QString DocType::xRechnungTemplate()
{
    return attributeValueString(XRechnungTmplStr);
}

void DocType::setXRechnungTemplate(const QString& tmpl)
{
    setAttribute(XRechnungTmplStr, tmpl);
}

QString DocType::attributeValueString(const QString& attribName) const
{
    QString re;
    if (attribName.isEmpty()) {
        return re;
    }
    if (mAttributes.hasAttribute(attribName)) {
        const auto att = mAttributes.value(attribName);
        re = att.value().toString();
    }
    return re;
}

void DocType::setAttribute( const QString& attribute, const QString& val)
{
    if ( !(attribute.isEmpty() || val.isEmpty()) ) {
        Attribute att( attribute );
        att.setPersistant( true );
        att.setValue( val);
        mAttributes[attribute] = att;
        mDirty = true;
    }
    // remove empty attribute
    if (!attribute.isEmpty() && val.isEmpty()) {
        mAttributes.markDelete(attribute);
        mDirty = true;
    }
}

QString DocType::watermarkFile()
{
    QString re;
    if ( mAttributes.hasAttribute( WatermarkFileStr ) ) {
        re = mAttributes[WatermarkFileStr].value().toString();
    }

    return re;
}

void DocType::setWatermarkFile( const QString& file )
{
    if ( !file.isEmpty() ) {
        Attribute att( WatermarkFileStr );
        att.setPersistant( true );
        att.setValue( file );
        mAttributes[WatermarkFileStr] = att;
    } else {
        // remove default value from map
        mAttributes.markDelete( WatermarkFileStr );
        // qDebug () << "Removing docMergeFile Attribute";
    }
    mDirty = true;
}

QString DocType::appendPDF() const
{
    return attributeValueString(AppendPDFStr);
}

void DocType::setAppendPDFFile(const QString& file)
{
    setAttribute(AppendPDFStr, file);
}

/**
 * @brief DocType::generateDocumentIdent
 * @param docDate
 * @param addressUid
 * @param id: Current Id to be used.
 * @param dayCnt: Current day counter to be used.
 * @return
 */
QString DocType::generateDocumentIdent(const QDate& docDate,
                                       const QString& addressUid,
                                       int id, int dayCnt)
{

    /*
   * The pattern may contain the following tags:
   * %y - the year of the documents date.
   * %w - the week number of the documents date
   * %d - the day number of the documents date
   * %m - the month number of the documents date
   * %M - the month number of the documents date
   * %c - the customer id from kaddressbook
   * %i - the uniq identifier from db.
   * %n - the uniq identifier that resets every day and starts from 1
   * %type - the localised doc type (offer, invoice etc.)
   * %uid  - the customer uid
   */

    // Load the template and check if there is a uniq id included.
    QString pattern = identTemplate();
    if ( pattern.indexOf( "%i" ) == -1 && pattern.indexOf("%n") == -1) {
        qWarning() << "No %i found in identTemplate, appending it to meet law needs!";
        if (!pattern.endsWith('-'))
            pattern += QStringLiteral("-");
        pattern += QStringLiteral("%i");
    }

    QMap<QString, QString> m;

    m[ "%yyyy" ] = docDate.toString( "yyyy" );
    m[ "%yy" ] = docDate.toString( "yy" );
    m[ "%y" ] = docDate.toString( "yyyy" );

    QString h;
    h = QString("%1").arg( docDate.weekNumber(), 2, 10, QChar('0') );
    m[ "%ww" ] = h;
    m[ "%w" ] = QString::number( docDate.weekNumber( ) );

    m[ "%dd" ] = docDate.toString( "dd" );
    m[ "%d" ] = docDate.toString( "d" );

    m[ "%m" ] = QString::number( docDate.month() );

    m[ "%MM" ] = docDate.toString( "MM" );
    m[ "%M" ] = docDate.toString( "M" );

    h = QString("%1").arg(id, 6, 10, QChar('0') );
    m[ "%iiiiii" ] = h;

    h = QString("%1").arg(id, 5, 10, QChar('0') );
    m[ "%iiiii" ] = h;

    h = QString("%1").arg(id, 4, 10, QChar('0') );
    m[ "%iiii" ] = h;

    h = QString("%1").arg(id, 3, 10, QChar('0') );
    m[ "%iii" ] = h;

    h = QString("%1").arg(id, 2, 10, QChar('0') );
    m[ "%ii" ] = h;

    m[ "%i" ] = QString::number( id );

    h = QString("%1").arg(dayCnt, 6, 10, QChar('0') );
    m[ "%nnnnnn" ] = h;

    h = QString("%1").arg(dayCnt, 5, 10, QChar('0') );
    m[ "%nnnnn" ] = h;

    h = QString("%1").arg(dayCnt, 4, 10, QChar('0') );
    m[ "%nnnn" ] = h;

    h = QString("%1").arg(dayCnt, 3, 10, QChar('0') );
    m[ "%nnn" ] = h;

    h = QString("%1").arg(dayCnt, 2, 10, QChar('0') );
    m[ "%nn" ] = h;

    m[ "%n" ] = QString::number(dayCnt);

    m[ "%c" ] = addressUid;
    m[ "%type" ] = name();
    m[ "%uid" ] = addressUid;

    QString re = StringUtil::replaceTagsInString( pattern, m );
    // qDebug () << "Generated document ident: " << re;

    return re;
}

// if hot, the id is updated in the database, otherwise not.
int DocType::nextIdentId( bool hot )
{
    QString numberCycle = numberCycleName();

    if ( numberCycle.isEmpty() ) {
        qCritical() << "NumberCycle name is empty";
        return -1;
    }

    QSqlQuery qLock;
    if ( hot ) {
        qLock.exec( "LOCK TABLES numberCycles WRITE" );
    }

    QSqlQuery q;
    q.prepare( "SELECT lastIdentNumber FROM numberCycles WHERE name=:name" );

    int num = -1;
    q.bindValue( ":name", numberCycle );
    q.exec();
    if ( q.next() ) {
        num = 1+( q.value( 0 ).toInt() );
        // qDebug () << "Got current number: " << num;

        if ( hot ) {
            QSqlQuery setQuery;
            setQuery.prepare( "UPDATE numberCycles SET lastIdentNumber=:newNumber WHERE name=:name" );
            setQuery.bindValue( ":name", numberCycle );
            setQuery.bindValue( ":newNumber", num );
            setQuery.exec();
            if ( setQuery.isActive() ) {
                // qDebug () << "Successfully created new id number for numbercycle " << numberCycle << ": " << num;
            }
        }
    }
    if ( hot ) {
        qLock.exec( "UNLOCK TABLES" );
    }

    return num;
}

int DocType::nextDayCounter(const QDate& docDate)
{
    int dayCnt {0};

    // Check the attribute for the day counter.
    QDate storedDate = mAttributes[DayCounterDateStr].value().toDate();
    // increment the day counter by one
    dayCnt = 1+mAttributes[DayCounterStr].value().toInt();

    if (storedDate != docDate) {
        // the daycounter is outdated. Reset the counter and update the date.
        setAttribute(DayCounterDateStr, docDate.toString(Qt::ISODate));
        dayCnt = 1;
    }
    setAttribute(DayCounterStr, QString::number(dayCnt));
    save();

    return dayCnt;
}

QString DocType::identTemplate()
{
    return mIdentTemplate;
}

void DocType::setIdentTemplate( const QString& t )
{
    mIdentTemplate = t;
}

void DocType::readIdentTemplate()
{
    QSqlQuery q;
    QString tmpl;

    const QString defaultTempl = QString::fromLatin1( "%y%ww-%i" );

    QString numberCycle = numberCycleName();
    if ( numberCycle.isEmpty() ) {
        qCritical() << "Numbercycle for doctype is empty, returning default";
        mIdentTemplate = defaultTempl;
    }
    // qDebug () << "Picking ident Template for numberCycle " << numberCycle;

    q.prepare( "SELECT identTemplate FROM numberCycles WHERE name=:name" );

    q.bindValue( ":name", numberCycle );
    q.exec();
    if ( q.next() ) {
        tmpl = q.value( 0 ).toString();
        // qDebug () << "Read ident template from database: " << tmpl;
    }

    // FIXME: Check again.
    if ( tmpl.isEmpty() ) {
        // qDebug () << "Writing ident template to database: " << pattern;
        QSqlQuery insQuery;
        insQuery.prepare( "UPDATE numberCycles SET identTemplate=:pattern WHERE name=:name" );
        insQuery.bindValue( ":name", numberCycle );
        insQuery.bindValue( ":pattern", defaultTempl);
        insQuery.exec();
        tmpl = defaultTempl;
    }
    mIdentTemplate = tmpl;
}

QString DocType::name() const
{
    return mName;
}

void DocType::setName( const QString& name )
{
    QString oldName = mName;
    dbID id = mNameMap[ oldName ]; // The old id.
    mNameMap[ name ] = id;
    mNameMap.remove( oldName );
    mName = name;
    mDirty = true;
}

void DocType::setXRechnungEnabled(bool state)
{
    if (state == isXRechnungEnabled())
        return;

    if (state)
        setAttribute(XRechnungEnabled, QStringLiteral("Yes"));
    else
        mAttributes.markDelete(XRechnungEnabled);
    mDirty = true;
}

bool DocType::isXRechnungEnabled() const
{
    bool re{false};

    if (mAttributes.contains(XRechnungEnabled)) {
        auto xre = mAttributes[XRechnungEnabled].value().toString();
        re = (xre == QStringLiteral("Yes"));
    }
    return re;
}

/*
 * Saves the name and the attriutes (numbercycle, demand, etc.)
 */
void DocType::save()
{
    if ( !mDirty ) {
        // qDebug () << "Saving: not DIRTY!";
        return;
    }

    if ( !mNameMap.contains( mName ) ) {
        qCritical() << "nameMap does not contain id for " << mName;
        return;
    }
    dbID id = mNameMap[ mName ];

    QSqlQuery q;

    bool doInsert = false;
    if ( id.isOk() ) {
        q.prepare( "UPDATE DocTypes SET name=:name WHERE docTypeId=:id" );
        q.bindValue( ":id", id.toInt() );
    } else {
        q.prepare( "INSERT INTO DocTypes (name) VALUES (:name)" );
        doInsert = true;
    }

    q.bindValue( ":name", mName );
    q.exec();

    if ( doInsert ) {
        mNameMap[mName] = KraftDB::self()->getLastInsertID();
    }

    mAttributes.save( mNameMap[mName] );
}
