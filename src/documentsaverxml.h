/***************************************************************************
             DocumentSaverXML - save documents to the database
                             -------------------
    begin                : 2006-02-21
    copyright            : (C) 2005 by Klaas Freitag
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

#ifndef _DocumentSaverXML_H
#define _DocumentSaverXML_H

#include "documentsaverbase.h"

class KraftDoc;
class QSqlRecord;
class dbID;
class QString;

class DocumentSaverXML : public DocumentSaverBase
{
    Q_OBJECT

public:
    DocumentSaverXML();
    virtual ~DocumentSaverXML();

    virtual bool saveDocument( KraftDoc* ) override;
    virtual void load( const QString& , KraftDoc * ) override;
protected:
    virtual void loadPositions( const QString&, KraftDoc* );
    virtual void saveDocumentPositions( KraftDoc* );
private:
};

#endif

/* END */

