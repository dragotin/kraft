/***************************************************************************
             materialcalcpart  -
                             -------------------
    begin                : 2004-09-05
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

#ifndef _MATERIALCALCPART_H
#define _MATERIALCALCPART_H

// include files
#include <QHash>
#include "calcpart.h"

/**
 *
 */
class StockMaterial;
class StockMaterialList;
class QString;
class QVariant;

class MaterialCalcPart : public CalcPart
{

public:
    MaterialCalcPart();
    MaterialCalcPart( long mCalcID, long matID, int procent, double amount );
    MaterialCalcPart( long matID, int procent, double amount );
    ~MaterialCalcPart();

    virtual Geld basisKosten();
    QString getType() const;

    StockMaterial* getMaterial();

    bool setCalcAmount( double newAmount );
    double getCalcAmount(){return m_calcAmount;};

protected:
    void getMatFromID(long matID);

private:
    long m_calcID;
    double m_calcAmount;
    StockMaterial *m_mat;
};

#endif

/* END */

