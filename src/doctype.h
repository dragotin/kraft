/***************************************************************************
                 doctype.h - doc type class
                             -------------------
    begin                : Oct. 2007
    copyright            : (C) 2007 by Klaas Freitag
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
#ifndef DOCTYPE_H
#define DOCTYPE_H

// include files for Qt
#include <qstring.h>
#include <qmap.h>

#include "attribute.h"
#include "dbids.h"

// application specific includes

/**
@author Klaas Freitag
*/

typedef QMap<QString, dbID> idMap;

class DocType
{
  public:
  DocType();
  /** 
   * create a doctype from its localised or tech name 
   */
  DocType( const QString& );

  static QStringList all();
  static QStringList allLocalised();
  
  QString techName() const;
  QString localisedName() const;
  
  bool allowDemand();
  bool allowAlternative();

  protected:

  private:
  static void init();

  AttributeMap mAttributes;
  QString      mName;
  dbID         mId;
  static idMap mNameMap;
};

#endif
