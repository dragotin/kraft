/***************************************************************************
                          timecalcpart.cpp  -
                             -------------------
    begin                : Don Jan 1 2004
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

#include <QDebug>

#include "klocalizedstring.h"

#include "timecalcpart.h"


TimeCalcPart::TimeCalcPart()
  :CalcPart(),
   _duration( 0 ),
   m_allowGlobalStundensatz(false),
   _timeUnit(Minutes)
{

}

TimeCalcPart::TimeCalcPart(const QString& name, int minutes, TimeUnit unit, int prozent)
    :CalcPart( name, prozent ),
      _duration( minutes ),
      m_allowGlobalStundensatz(true),
      _timeUnit( unit )
{

}

TimeCalcPart::~TimeCalcPart()
{

}

StdSatz& TimeCalcPart::getStundensatz()
{
    return m_stundensatz;
}

/** Write property of Geld m_stundensatz. */
void TimeCalcPart::setStundensatz( const StdSatz& _newVal)
{
    // qDebug() << "stundensatz gesetzt: " << _newVal.toString() << endl;
    m_stundensatz = _newVal;
    setDirty(true);
}

void TimeCalcPart::setGlobalStdSetAllowed( bool s  )
{
    if( m_allowGlobalStundensatz != s )
    {
        m_allowGlobalStundensatz = s;
        setDirty(true);
    }
}

void TimeCalcPart::setDuration( int duration, const QString& unitStr )
{
    if( _duration != duration || timeUnitString(_timeUnit) != unitStr ) {
        _duration = duration;
        _timeUnit = timeUnitFromString(unitStr);
        setDirty(true);
    }
}

QString TimeCalcPart::timeUnitString( const TimeUnit& unit )
{
    if( unit == Minutes ) {
        return i18n("Minutes");
    } else if( unit == Hours) {
        return i18n("Hours");
    }
    return i18n("Seconds");
}

QStringList TimeCalcPart::timeUnitStrings()
{
    // When adding something here make sure to adjust other places in the file
    return QStringList() << timeUnitString(Minutes) <<
                            timeUnitString(Seconds) <<
                            timeUnitString(Hours);
}

TimeCalcPart::TimeUnit TimeCalcPart::timeUnitFromString( const QString& unit)
{
    const QStringList li = timeUnitStrings();
    int pos = li.indexOf(unit);

    return timeUnitFromInt(pos);
}

TimeCalcPart::TimeUnit TimeCalcPart::timeUnitFromInt( int index )
{
    // the static_cast here need to be updated if new enums are added
    if( index > -1 && index <= static_cast<int>(Hours)) {
        switch (index) {
            case static_cast<int>(Minutes):
            case static_cast<int>(Seconds):
            case static_cast<int>(Hours):
                return static_cast<TimeUnit>(index);
            default:
                // this is actually an error case, forgot to add a pot. new enum...
                return Minutes;
        }
    }
    return Minutes;
}

int TimeCalcPart::timeUnitIndex() const
{
    // Make sure to adopt this if a new unit is added!
    if( _timeUnit == Hours )
        return 2;
    else if( _timeUnit == Seconds )
        return 1;
    else
        return 0;
}

qint32 TimeCalcPart::durationToSeconds() const
{
    if( _timeUnit == Minutes ) {
        return _duration * 60;
    } else if( _timeUnit == Hours ) {
        return 60*60*_duration;
    }
    // seconds is default
    return _duration;
}

Geld TimeCalcPart::basisKosten()
{
    StdSatz stdSatz = getStundensatz();
    const Geld g( (stdSatz.getPreis().toLong() * durationToSeconds()) / 360000.0);
    return g;
}

QString TimeCalcPart::getType() const
{
    return KALKPART_TIME;
}
