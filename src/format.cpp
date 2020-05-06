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
#include <QDate>

#include "format.h"

namespace Format {

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

QString toDateString( const QDate& date, const QString& format )
{
    if (format == Format::DateFormatIso) {
        return date.toString(Qt::ISODate);
    }
    if (format == DateFormatShort || format.isEmpty()) {
        return date.toString(Qt::DefaultLocaleShortDate);
    }
    if (format == DateFormatLong) {
        return date.toString(Qt::DefaultLocaleLongDate);
    }
    if (format == DateFormatRFC) {
        return date.toString(Qt::RFC2822Date);
    }
    if (format == DateFormatGerman) {
        return date.toString("dd.MM.yyyy");
    }
    return date.toString(format); // good luck!
}

QString toDateTimeString(const QDateTime& dt, const QString &format)
{
    const QString dateStr = QString("%1, %2:%3").arg(toDateString(dt.date(), format))
            .arg(dt.time().hour(), 2, 10, QLatin1Char('0'))
            .arg(dt.time().minute(), 2, 10, QLatin1Char('0'));
    return dateStr;
}

}
