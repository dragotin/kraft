/***************************************************************************
                          zeitcalcpart.h  -
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

#ifndef ZEITCALCPART_H
#define ZEITCALCPART_H

#include <calcpart.h>

#include "stdsatzman.h"

/**Implementiert einen zeitabhängigen Kalkulationsbeitrag.
  *@author Klaas Freitag
  */

class ZeitCalcPart : public CalcPart  {

public:
    ZeitCalcPart( const QString& name, int minutes, int prozent = 0);
  ZeitCalcPart();
    ~ZeitCalcPart();

    bool globalStdSetAllowed() { return m_allowGlobalStundensatz; }
    void setGlobalStdSetAllowed( bool s );

    virtual Geld basisKosten();
    /** Write property of Geld m_stundensatz. */
    virtual void setStundensatz( const StdSatz& _newVal);
    /** Read property of Geld m_stundensatz. */
    virtual StdSatz& getStundensatz();

    virtual QString getType() const;

    virtual void setMinuten( int m );
    virtual int getMinuten() { return m_minuten; }

private:
    int m_minuten;
    /**  */
    StdSatz m_stundensatz;
    bool m_allowGlobalStundensatz;
};

#endif
