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
#include "kraftdoc.h"

class QSqlQuery;

typedef QMap<QString, DocGuardedPtr> DocumentMap;

class DocumentMan
{
  public:
    ~DocumentMan();

    static DocumentMan *self();
    DocGuardedPtr openDocumentByUuid(const QString& uuid);

    // FIXME: Use this to display an error in the main screen if
    // something fails
    void setDocProcessingError(const QString& errStr);

    // persisting the docs
    bool saveDocument(KraftDoc* doc);
    bool reloadDocument(KraftDoc* doc);
    void closeDocument(const QString& ident);

    // create new docs
    DocGuardedPtr createDocument(const QString& docType, const QString& copyFromUuid = QString(),
                                 const DocPositionList &listToCopy = DocPositionList() );
    DocGuardedPtr copyDocument(const QString& copyFromUuid );

    bool loadMetaFromFilename(const QString&, KraftDoc *doc);

    DocumentMan();
};

#endif
