#ifndef DOCDIGEST_H
#define DOCDIGEST_H

#include <qdatetime.h>
#include <qvaluelist.h>

#include "dbids.h"

/***************************************************************************
                          docdigest.h  -
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


class DocDigest
{
  
public:
  DocDigest( dbID id, const QString& type, const QString& clientID );
  DocDigest();
  
  QString clientName();
  void setClientId( const QString& id ) { mClientId = id; }
  
  QString type() { return mType; }
  void setType( const QString& t ) { mType = t; }
  
  QString date();
  void setDate( const QDate& date ) { mDate = date; }
  
  QString lastModified();
  void setLastModified( const QDate& date ) { mLastModified = date; }
  
  QString id()   { return mID.toString(); } 
  void setId( dbID id ) { mID = id; }
  
  QString ident()   { return mIdent; } 
  void setIdent( const QString& ident ) { mIdent = ident; }
protected:
  dbID mID;
  QString mType;
  QString mClientId;
  QString mIdent;
  QDate   mLastModified;
  QDate   mDate;
};

typedef QValueList<DocDigest> DocDigestList;
typedef QValueListIterator<DocDigestList> DocDigestListIterator;
#endif
