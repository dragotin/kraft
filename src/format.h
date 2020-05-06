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
    /**
     * @brief localeDoubleToString - convert a double into a string locale aware.
     * @param val - the value
     * @param loc - a locale, if skipped, the default locale is used.
     * @return the string
     *
     * The additional cleverness is that the returned string has the right
     * precision, ie. it returns "2" for val = 2.00 but "2.23" if val is 2.23.
     */

    QString localeDoubleToString(double val, const QLocale& loc = QLocale());

    const QString DateFormatIso = QStringLiteral("ISO");
    const QString DateFormatShort = QStringLiteral("Short");
    const QString DateFormatLong = QStringLiteral("Long");
    const QString DateFormatRFC = QStringLiteral("RFC");
    const QString DateFormatGerman = QStringLiteral("German");

    /**
     * @brief toDateString - format date to a given format
     * @param date - the QDate
     * @param format - A name of a format (read from settings) or format string
     * @return  the string containing the date.
     */

    QString toDateString(const QDate& date, const QString &format);
    QString toDateTimeString(const QDateTime& dt, const QString &format);

}


#endif // FORMAT_H
