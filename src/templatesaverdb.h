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

#include "templatesaverbase.h"

/**
 *
 */
class FloskelTemplate;
class QSqlRecord;
class QString;
class TimeCalcPart;
class FixCalcPart;
class MaterialCalcPart;
class StockMaterial;

class TemplateSaverDB : public TemplateSaverBase
{
public:
    TemplateSaverDB();
    virtual ~TemplateSaverDB();

    virtual bool saveTemplate( FloskelTemplate* );
    virtual void saveTemplateChapter( FloskelTemplate* );
private:
  void fillTemplateBuffer( QSqlRecord*, FloskelTemplate*, bool );
  QString sqlWhereFromRecord( QSqlRecord * ) const;
};


class CalculationsSaverDB:public CalculationsSaverBase
{
public:
  CalculationsSaverDB();
  CalculationsSaverDB( TargetType tt );
  virtual ~CalculationsSaverDB() { }

  bool saveCalculations( CalcPartList, dbID );

private:
  bool saveFixCalcPart( FixCalcPart *cp, dbID );
  bool saveMaterialCalcPart( MaterialCalcPart *cp, dbID );
  bool saveTimeCalcPart( TimeCalcPart*, dbID );

  void fillFixCalcBuffer( QSqlRecord *buffer, FixCalcPart *cp );
  void fillMatCalcBuffer( QSqlRecord *buffer, MaterialCalcPart *cp );
  void fillTimeCalcBuffer( QSqlRecord*, TimeCalcPart* );

  QString mTableTimeCalc;
  QString mTableFixCalc;
  QString mTableMatCalc;
  QString mTableMatDetailCalc;
};

#endif

/* END */

