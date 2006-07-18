
/***************************************************************************
             geld  -
                             -------------------
    begin                : 2004-16-08
    copyright            : (C) 2004 by Klaas Freitag
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

#ifndef _GELD_H
#define _GELD_H


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// include files

/**
 *
 */

class Geld
{
public:
    Geld();
    Geld(long);
    Geld(double);

    ~Geld();

    Geld& operator=(const long);
    Geld& operator=(const Geld&);

    Geld operator/(const double);
    Geld operator*(const long);
    Geld operator*(const double);
    Geld& operator+=(const Geld&);

    bool operator!=(Geld);

    QString toString() const;
    double toDouble();
    long   toLong();
private:
    long m_cent;
};

#endif

/* END */









