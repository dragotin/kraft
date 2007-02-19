/***************************************************************************
                          fixcalcpart.h  -
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

#ifndef FIXCALCPART_H
#define FIXCALCPART_H

#include "calcpart.h"

/**Implementiert einen fixen Betrag pro kalkulierter Einheit.
  *@author Klaas Freitag
  */

class FixCalcPart : public CalcPart  {
public:
  FixCalcPart();
    FixCalcPart( QString name, Geld preis, int prozent = 0);
    ~FixCalcPart();

    virtual Geld basisKosten();

    /* der Preis für eine Einheit */
    Geld unitPreis();
    void setUnitPreis( Geld );

    void setMenge(double);
    double getMenge() const { return m_amount; }

    QString getType() const;

private: // Private attributes
    /**  */
    Geld    m_fixPreis;
    double  m_amount;
};

#endif
