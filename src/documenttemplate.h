/***************************************************************************
            Template for Kraft Documents - Grantlee and ctemplate
                             -------------------
    begin                : March 2020
    copyright            : (C) 2020 by Klaas Freitag
    email                : kraft@freisturz.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DOCUMENTTEMPLATE_H
#define DOCUMENTTEMPLATE_H

#include <kcontacts/addressee.h>

#include "archdoc.h"

class DocumentTemplate
{
public:
    DocumentTemplate( const QString& tmplFile );
    virtual ~DocumentTemplate(){ };

    virtual const QString expand(ArchDoc *archDoc,
                                 const KContacts::Addressee &myContact,
                                 const KContacts::Addressee &customerContact) = 0;

    QString error() const { return _errorStr; }

protected:
    QString _tmplFile;
    QString _errorStr;
};

// ==================================================================================

class CTemplateDocumentTemplate : public DocumentTemplate
{
public:
    CTemplateDocumentTemplate(const QString& tmplFile);

    const QString expand(ArchDoc *archDoc,
                         const KContacts::Addressee &myContact,
                         const KContacts::Addressee &customerContact) override;
};

// ==================================================================================

class GrantleeDocumentTemplate : public DocumentTemplate
{
public:
    GrantleeDocumentTemplate(const QString& tmplFile);

    const QString expand(ArchDoc *archDoc,
                         const KContacts::Addressee &myContact,
                         const KContacts::Addressee &customerContact) override;
};

#endif // DOCUMENTTEMPLATE_H
