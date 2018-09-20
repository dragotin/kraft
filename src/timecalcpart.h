/***************************************************************************
                          Timecalcpart.h  -
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

#ifndef TIMECALCPART_H
#define TIMECALCPART_H

#include <calcpart.h>

#include "stdsatzman.h"

/**
  *@author Klaas Freitag
  */

class TimeCalcPart : public CalcPart  {

public:
    enum TimeUnit {Minutes, Seconds, Hours};

    TimeCalcPart( const QString& name, int minutes, TimeUnit unit, int prozent = 0);
    TimeCalcPart();
    ~TimeCalcPart();

    bool globalStdSetAllowed() { return m_allowGlobalStundensatz; }
    void setGlobalStdSetAllowed( bool s );
    void setDuration(int duration, const QString &unitStr );

    static QStringList timeUnitStrings();
    static QString timeUnitString( const TimeUnit& unit );
    static TimeCalcPart::TimeUnit timeUnitFromString( const QString& unit);
    static TimeCalcPart::TimeUnit timeUnitFromInt(int index);

    qint32 durationToSeconds() const;
    qint32 duration()  const  { return _duration; }
    TimeUnit timeUnit() const { return _timeUnit; }
    int timeUnitIndex() const;

    virtual Geld basisKosten();
    /** Write property of Geld m_stundensatz. */
    virtual void setStundensatz( const StdSatz& _newVal);
    /** Read property of Geld m_stundensatz. */
    virtual StdSatz& getStundensatz();

    virtual QString getType() const;

private:
    qint32 _duration;
    /**  */
    StdSatz m_stundensatz;
    bool m_allowGlobalStundensatz;
    TimeUnit _timeUnit;
};

#endif
