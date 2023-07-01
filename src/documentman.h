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

#include <QDate>

#include "docguardedptr.h"
#include "docposition.h"

class QSqlQuery;

typedef QMap<QString, DocGuardedPtr> DocumentMap;

class DocumentMan
{
  public:
    ~DocumentMan();

    static DocumentMan *self();

    // persisting the docs
    DocGuardedPtr openDocumentByIdent( const QString& ident );
    DocGuardedPtr openDocumentByUuid(const QString& uuid);

    bool saveDocument(KraftDoc* doc);
    bool reloadDocument(KraftDoc* doc);
    void closeDocument(const QString& ident);

    DocGuardedPtr createDocument(const QString& docType, const QString& copyFromUuid = QString(),
                                 const DocPositionList &listToCopy = DocPositionList() );
    DocGuardedPtr copyDocument(const QString& copyFromUuid );

    bool convertDbToXml(const QString& docID, const QString &basePath);

    bool loadMetaFromFilename(const QString&, KraftDoc *doc);

    DocumentMan();
};

#endif
