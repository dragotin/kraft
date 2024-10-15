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
const QString DefaultTmplFileName {"invoice.gtmpl"};
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

    QString reportFileName{name().toLower()};
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

// if hot, the id is updated in the database, otherwise not.

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
