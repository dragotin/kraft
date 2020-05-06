/***************************************************************************
                                docdigest.cpp
                             -------------------
    begin                : Wed Mar 15 2006
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

#include <QString>
#include <QLocale>
#include <QSqlQuery>

#include <kcontacts/addressee.h>

#include "docdigest.h"
#include "defaultprovider.h"
#include "format.h"
#include "kraftsettings.h"

DocDigest::DocDigest( dbID id, const QString& type, const QString& clientID )
  :mID(id), mType( type ), mClientId( clientID ), mLocale( "kraft" ),
    _archDocLazyLoaded(false)
{

}

DocDigest::DocDigest()
  :mLocale( "kraft" ), _archDocLazyLoaded(false)
{
}

QString DocDigest::date() const
{
    return Format::toDateString(mDate, KraftSettings::self()->dateFormat());
}

QDate DocDigest::rawDate() const
{
    return mDate;
}

QString DocDigest::lastModified() const
{
    const QString re = QString( "%1 %2").arg( Format::toDateString(mLastModified.date(), KraftSettings::self()->dateFormat()))
            .arg(mLastModified.time().toString("hh:mm"));
    return re;
}

ArchDocDigestList DocDigest::archDocDigestList()
{
    if( !_archDocLazyLoaded ) {
        const QString id(ident());

        qDebug() << "Querying archdocs for document ident " << id;
        QSqlQuery query;
        query.prepare("SELECT archDocID, ident, printDate, state FROM archdoc WHERE"
                      " ident=:id ORDER BY printDate DESC" );
        query.bindValue(":id", id);
        query.exec();

        while(query.next()) {
            int archDocID = query.value(0).toInt();
            const QString dbIdent = query.value(1).toString();
            QDateTime printDateTime = query.value(2).toDateTime();
            int state = query.value(3).toInt();
            mArchDocs.append( ArchDocDigest( printDateTime, state, dbIdent, dbID(archDocID) ) );
        }
        _archDocLazyLoaded = true;
    }
    return mArchDocs;
}

KContacts::Addressee DocDigest::addressee() const
{
  return mContact;
}
void DocDigest::setAddressee( const KContacts::Addressee& contact )
{
  mContact = contact;
}


/* *************************************************************************** */

DocDigestsTimeline::DocDigestsTimeline()
  :mMonth( 0 ), mYear( 0 )
{

}

DocDigestsTimeline::DocDigestsTimeline( int m,  int y )
  :mMonth( m ), mYear( y )
{

}

void DocDigestsTimeline::setDigestList( const DocDigestList& list )
{
  mDigests = list;
}
