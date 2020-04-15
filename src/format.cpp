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

#include <QString>
#include <QLocale>



namespace Format {

/**
 * @brief localeDoubleToString - convert a double into a string locale aware.
 * @param val - the value
 * @param loc - a locale, if skipped, the default locale is used.
 * @return the string
 *
 * The additional cleverness is that the returned string has the right
 * precision, ie. it returns "2" for val = 2.00 but "2.23" if val is 2.23.
 */
QString localeDoubleToString(double val, const QLocale& loc)
{
    int prec = 0;
    const QString num = QString::number(val);
    if( num.contains( QChar('.') ) ) {
        // there is a decimal point
        // calculate the precision
        prec = num.length() - (1+num.lastIndexOf( QChar('.') ) );
        if (prec > 3 ) prec = 3; // lets dont go wild
    }

    const QString re = loc.toString(val, 'f', prec);
    return re;
}

}
