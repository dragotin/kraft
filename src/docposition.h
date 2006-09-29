/***************************************************************************
                 docposition.h  - a position in a document
                             -------------------
    begin                : Fri Jan 20 2006
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
#ifndef DOCPOSITION_H
#define DOCPOSITION_H

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
class QDomElement;
class QDomDocument;

class DocPositionBase : public QObject
{
  public:
    enum PositionType { Position, Header };
    DocPositionBase();
    ~DocPositionBase() {}

    DocPositionBase(const DocPositionBase&);

    void setDbId( int id ) { m_dbId = id; }
    dbID dbId() { return dbID( m_dbId ); }
    virtual PositionType type() = 0;

  // virtual void setPosition( const QString& pos ) { m_position = pos; }
    virtual QString position() { return m_position; }

    virtual void setToDelete( bool doit ) { mToDelete = doit; }
    virtual bool toDelete() { return mToDelete; }
  protected:
    int     m_dbId;
    QString m_position;
    bool    mToDelete;
};

class DocPosition : public DocPositionBase
{
  public:
    DocPosition();
    ~DocPosition(){};

    void setText( const QString& string ) { m_text = string; }
    QString text() const { return m_text; } ;

    void setUnit( const Einheit& unit ) { m_unit = unit; }
    Einheit unit() const { return m_unit; }

    void setUnitPrice( const Geld& g ) { m_unitPrice = g; }
    Geld unitPrice() const { return m_unitPrice; }
    Geld overallPrice();

    void setAmount( double amount ) { m_amount = amount; }
    double amount() { return m_amount; }

    DocPosition& operator=( const DocPosition& );
    PositionType type() { return Position; }
  private:
    QString m_text;
    Einheit m_unit;
    Geld    m_unitPrice;
    double  m_amount;
    // No calculation yet
};

class DocPositionList : public QPtrList<DocPositionBase>
{
  public:
    DocPositionList();
    Geld sumPrice();
    QDomElement domElement( QDomDocument& );
    DocPositionBase *positionFromId( int id );
    QString posNumber( DocPositionBase* );
  protected:
    int compareItems ( QPtrCollection::Item item1, QPtrCollection::Item item2 );

  private:
    QDomElement xmlTextElement( QDomDocument&, const QString& , const QString& );
};

typedef QPtrListIterator<DocPositionList> DocPositionListIterator;

typedef QGuardedPtr<DocPositionBase> DocPositionGuardedPtr;
#endif
