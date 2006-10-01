/***************************************************************************
           archdocposition.h  - a position in an archived document
                             -------------------
    begin                : Sep 2006
    copyright            : (C) 2006 by Klaas Freitag
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
#ifndef ARCHDOCPOSITION_H
#define ARCHDOCPOSITION_H

// include files for Qt
#include <qstring.h>
#include <qvaluelist.h>

// include files for KDE

// application specific includes
#include "einheit.h"
#include "geld.h"
#include "dbids.h"


class ArchDoc;
/**
@author Klaas Freitag
*/

class ArchDocPosition
{
    friend class ArchDoc;
  public:
    ArchDocPosition();
    ~ArchDocPosition(){};

    QString posNumber() const { return mPosNo; }

    QString text() const { return mText; } ;

    QString unit() const { return mUnit; }

    Geld unitPrice() const { return mUnitPrice; }
    Geld overallPrice();

    double amount() { return mAmount; }
    double vat() { return mVat; }

  private:
    QString mText;
    QString mPosNo;
    QString mUnit;
    Geld    mUnitPrice;
    double  mAmount;
    double  mVat;
    // No calculation yet
};

class ArchDocPositionList : public QValueList<ArchDocPosition>
{
  public:
    ArchDocPositionList();
    Geld sumPrice();
};


#endif

