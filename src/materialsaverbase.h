/***************************************************************************
             materialsaverbase  - Base class of a material saver
                             -------------------
    begin                : 2006-12-07
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

#ifndef _MATERIALSAVERBASE_H
#define _MATERIALSAVERBASE_H

// include files
#include <qobject.h>

/**
 *
 */
class StockMaterial;

class MaterialSaverBase : public QObject
{
    Q_OBJECT

public:
    MaterialSaverBase();
    ~MaterialSaverBase();

    virtual bool saveTemplate( StockMaterial* ) = 0;
    virtual void saveTemplateChapter( StockMaterial* ) = 0;
};

#endif

/* END */

