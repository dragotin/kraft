/***************************************************************************
                   numbercycle.h  - document number cycles
                             -------------------
    begin                : Jan 15 2009
    copyright            : (C) 2009 by Klaas Freitag
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

#ifndef NUMBERCYCLE_H
#define NUMBERCYCLE_H

#include <qstring.h>

#include "dbids.h"
#include "kraftcat_export.h"

class KRAFTCAT_EXPORT NumberCycle
{
public:
  NumberCycle();
  NumberCycle( dbID );

  void setName( const QString& );
  QString name();

  void setTemplate( const QString& );
  QString getTemplate();

  void setCounter( int );
  int  counter();

  static QString defaultName();

private:
  dbID id;
  QString mName;
  QString mTemplate;
  int     mCounter;
};

#endif
