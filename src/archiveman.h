/***************************************************************************
                       archiveman.h  - Archive Manager
                             -------------------
    begin                : July 2006
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
#ifndef ARCHIVEMAN_H
#define ARCHIVEMAN_H

#include <qdom.h>

#include "dbids.h"

class KraftDoc;
class dbID;
class QDomDocument;

class ArchiveMan
{
  public:
    virtual ~ArchiveMan();

    static ArchiveMan *self();
    dbID archiveDocument( KraftDoc* );

    /**
     * query the document identifier id for a given database archive id
     */
    QString documentID( dbID archID ) const;

    QString xmlBaseDir() const;
    QString pdfBaseDir() const;
    QString archiveFileName( const QString&, const QString&, const QString& ) const;

    ArchiveMan();
  protected:
    virtual QDomDocument archiveDocumentXml( KraftDoc*,  const QString& );
    virtual dbID archiveDocumentDb( KraftDoc* );

  private:
    QDomElement xmlTextElement( QDomDocument, const QString&, const QString& );
    int archivePos( int, KraftDoc* );
    void ensureDirIsExisting( const QString& dir ) const;
};

#endif
