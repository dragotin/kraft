/***************************************************************************
                       documentman.h  - Document Manager
                             -------------------
    begin                : 2006
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

#ifndef DOCUMENTMAN_H
#define DOCUMENTMAN_H

#include "docdigest.h"
#include "kraftdoc.h"

class DocPosition;
class QSqlQuery;

typedef QMap<QString, DocGuardedPtr> DocumentMap;

class DocumentMan
{
  public:
    ~DocumentMan();

    static DocumentMan *self();

    DocDigestList latestDocs( int );
    DocDigestsTimelineList docsTimelined();

    DocGuardedPtr createDocument( const QString& docType, const QString& copyFromId = QString() );
    DocGuardedPtr openDocument( const QString& );

    double tax( const QDate& );
    double reducedTax( const QDate& );
    void clearTaxCache();

  private:
    bool readTaxes( const QDate& );
    DocDigest digestFromQuery( QSqlQuery& );
    const QString mColumnList;

    double mFullTax;
    double mReducedTax;
    QDate  mTaxDate;

    DocumentMan();

    static DocumentMan *mSelf;
};

#endif
