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
#include "format.h"
#include "kraftsettings.h"

DocDigest::DocDigest(const QString& type, const QString& clientID)
  :KraftObj(), mType( type ), mClientId( clientID ), mLocale( "kraft" )
{

}

DocDigest::DocDigest()
  :KraftObj(), mLocale( "kraft" )
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

ArchDocDigestList DocDigest::archDocDigestList() const
{
    const QString id(ident());

    qDebug() << "Querying archdocs for document ident " << id;
    QSqlQuery query;
    query.prepare("SELECT archDocID, ident, docType, printDate, state FROM archdoc WHERE"
                  " ident=:id ORDER BY printDate DESC" );
    query.bindValue(":id", id);
    query.exec();

    ArchDocDigestList archDocs;
    while(query.next()) {
        int archDocID = query.value(0).toInt();
        const QString dbIdent = query.value(1).toString();
        const QString docType = query.value(2).toString();
        QDateTime printDateTime = query.value(3).toDateTime();
        int state = query.value(4).toInt();
        archDocs.append( ArchDocDigest( printDateTime, state, dbIdent, docType, dbID(archDocID) ) );
    }

    return archDocs;
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
