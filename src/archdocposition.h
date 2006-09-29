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
#include <qptrlist.h>
#include <qstring.h>
#include <qguardedptr.h>
#include <qobject.h>

// include files for KDE
#include <klocale.h>
#include <kdebug.h>

// application specific includes
#include "einheit.h"
#include "geld.h"
#include "dbids.h"


/**
@author Klaas Freitag
*/

class ArchDocPosition
{
  public:
    ArchDocPosition();
    ~ArchDocPosition(){};

    QString posNumber() const { return m_posNo; }

    QString text() const { return m_text; } ;

    Einheit unit() const { return m_unit; }

    Geld unitPrice() const { return m_unitPrice; }
    Geld overallPrice();

    double amount() { return m_amount; }

  private:
    QString m_text;
    QString m_posNo;
    Einheit m_unit;
    Geld    m_unitPrice;
    double  m_amount;
    double  m_vat;
    // No calculation yet
};

class ArchDocPositionList : public QValueList<ArchDocPosition>
{
  public:
    ArchDocPositionList();
    Geld sumPrice();
};

typedef QValueListIterator<ArchDocPositionList> ArchDocPositionListIterator;

#endif

