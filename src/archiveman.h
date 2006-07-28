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

class KraftDoc;
class dbID;

class ArchiveMan
{
  public:
    ~ArchiveMan();

    static ArchiveMan *self();
    dbID archiveDocument( KraftDoc* );
    dbID archiveDocumentDb( KraftDoc* );
  
  private:
    ArchiveMan();
    QDomElement xmlTextElement( QDomDocument, const QString&, const QString& );
    int archivePos( int, KraftDoc* );

    static ArchiveMan *mSelf;
};

#endif
