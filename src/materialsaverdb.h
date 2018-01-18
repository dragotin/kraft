/***************************************************************************
             materialsaverdb  -
                             -------------------
    begin                : 2006-12-07
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

#ifndef _MATERIALSAVERDB_H
#define _MATERIALSAVERDB_H

// include files

#include "materialsaverbase.h"

/**
 *
 */
class QSqlRecord;
class StockMaterial;

class MaterialSaverDB : public MaterialSaverBase
{
public:
    static MaterialSaverBase* self();
    MaterialSaverDB();

private:
    virtual bool saveTemplate( StockMaterial* );
    virtual void fillMaterialBuffer( QSqlRecord &, StockMaterial* , bool );
    virtual void saveTemplateChapter( StockMaterial* );
};

#endif

/* END */

