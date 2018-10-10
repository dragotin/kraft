/***************************************************************************
                          calcpart.h  -
                             -------------------
    begin                : Mit Dez 31 2003
    copyright            : (C) 2003 by Klaas Freitag
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

#ifndef CALCPART_H
#define CALCPART_H

#include <QObject>
#include <QList>

#include "dbids.h"
#include "kraftglobals.h"

/**This file contains a part of a calculation.
  *@author Klaas Freitag
  */

class CalcPart {
public:
    CalcPart();
    CalcPart( int prozent );
    CalcPart( const QString& name, int prozent = 0 );
    virtual ~CalcPart();

    /** Write property of int m_prozentPlus. */
    virtual void setProzentPlus( const double& _newVal);
    /** Read property of int m_prozentPlus. */
    virtual const double& getProzentPlus();
    /** base costs for one unit */
    virtual Geld basisKosten();

    void setName( const QString& name );
    QString getName() const { return m_name; }

    virtual QString getType() const;

    virtual bool isDirty() { return m_dirty; }
    virtual void setDirty( bool b ) { m_dirty = b; }

    virtual dbID getDbID() { return m_dbId; }
    virtual void setDbID( dbID id ) { m_dbId = id; }

    virtual dbID getTemplID() { return m_templId; }
    virtual void setTemplID( dbID id ) { m_templId = id; }

    virtual void setToDelete(bool );
    virtual bool isToDelete();

private:
    double  m_prozentPlus;
    QString m_name;
    dbID    m_dbId;
    dbID    m_templId;
    bool    m_dirty;
    bool    m_toDelete;
};

class CalcPartList : public QList<CalcPart*>
{
public:
  CalcPartList();

  Geld calcPrice();
  Geld costPerCalcPart( const QString& );
  CalcPartList getCalcPartsList( const QString& );
  CalcPartList decoupledCalcPartsList();
};

typedef QListIterator<CalcPart*> CalcPartListIterator; 

#endif
