/***************************************************************************
                          floskel.h  -  
                             -------------------
    begin                : Mit Dez 31 2003
    copyright            : (C) 2003 by Klaas Freitag
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

#ifndef FLOSKEL_H
#define FLOSKEL_H

#include <qstring.h>

#include "kraftglobals.h"
#include "einheit.h"
/**
  *@author Klaas Freitag
  */

class Floskel {
public: 
    Floskel();
    ~Floskel();
   
private:
    QString  text;
    Einheit  einheit;
};

#endif
