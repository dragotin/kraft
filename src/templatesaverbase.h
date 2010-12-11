/***************************************************************************
             templatesaverbase  - Base class of a template save class
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

#ifndef _TEMPLATESAVERBASE_H
#define _TEMPLATESAVERBASE_H

// include files
#include <qobject.h>

/**
 *
 */
class FloskelTemplate;
class dbID;
class CalcPartList;

class TemplateSaverBase
{
public:
    TemplateSaverBase();
    virtual ~TemplateSaverBase();

    virtual bool saveTemplate( FloskelTemplate* );
    virtual void saveTemplateChapter( FloskelTemplate* );
private:

};

class CalculationsSaverBase
{
public:
  enum TargetType { Template, Document };
  CalculationsSaverBase();
  virtual ~CalculationsSaverBase() { }
  CalculationsSaverBase( TargetType );

  virtual bool saveCalculations( CalcPartList, dbID ) = 0;

};

#endif

/* END */

