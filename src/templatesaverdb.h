/***************************************************************************
             templatesaverdb  -
                             -------------------
    begin                : 2005-20-00
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

#ifndef _TEMPLATESAVERDB_H
#define _TEMPLATESAVERDB_H


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// include files

#include "templatesaverbase.h"

/**
 *
 */
class FloskelTemplate;
class QSqlRecord;
class ZeitCalcPart;
class FixCalcPart;
class MaterialCalcPart;
class StockMaterial;

class TemplateSaverDB : public TemplateSaverBase
{
    Q_OBJECT

public:
    TemplateSaverDB();
    ~TemplateSaverDB();

    virtual bool saveTemplate( FloskelTemplate* );

protected:
    virtual bool saveTimeCalcPart( ZeitCalcPart *cp, FloskelTemplate *tmpl );
    virtual bool saveFixCalcPart( FixCalcPart *cp, FloskelTemplate *tmpl );
    virtual bool saveMaterialCalcPart( MaterialCalcPart *cp, FloskelTemplate *tmpl );

    virtual void fillTemplateBuffer( QSqlRecord*, FloskelTemplate*, bool );
    virtual void fillZeitCalcBuffer( QSqlRecord *buffer, ZeitCalcPart *cp );
    virtual void fillFixCalcBuffer( QSqlRecord *buffer, FixCalcPart *cp );
    virtual void fillMatCalcBuffer( QSqlRecord *buffer, MaterialCalcPart *cp );
    virtual void storeMaterialDetail( MaterialCalcPart *cp, StockMaterial *mat );

private:
    QString sqlWhereFromRecord( QSqlRecord * ) const;
};

#endif

/* END */

