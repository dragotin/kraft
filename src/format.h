/***************************************************************************
                  Simple format functions for double, date etc.
                             -------------------
    begin                : March 2020
    copyright            : (C) 2020 by Klaas Freitag
    email                : kraft@freisturz.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef FORMAT_H
#define FORMAT_H

#include <QLocale>

class QString;

namespace Format {

    QString localeDoubleToString(double val, const QLocale& loc = QLocale());

}


#endif // FORMAT_H
