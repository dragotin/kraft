/***************************************************************************
             documentsaverdb - save documents to the database
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

#ifndef _DOCUMENTSAVERDB_H
#define _DOCUMENTSAVERDB_H

#include "documentsaverbase.h"

class KraftDoc;
class QSqlRecord;
class dbID;
class QString;

class DocumentSaverDB : public DocumentSaverBase
{
    Q_OBJECT

public:
    DocumentSaverDB();
    virtual ~DocumentSaverDB();

    virtual bool saveDocument( KraftDoc* );
    virtual void load( const QString& , KraftDoc * );
protected:
    virtual void loadPositions( const QString&, KraftDoc* );
    virtual void saveDocumentPositions( KraftDoc* );
private:
    const QString PosTypePosition;
    const QString PosTypeExtraDiscount;
    const QString PosTypeHeader;
};

#endif

/* END */

