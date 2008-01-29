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

#include <klocale.h>

// application specific includes
#include "dbids.h"
#include "calcpart.h"
#include "attribute.h"
#include "einheit.h"

/**
@author Klaas Freitag
*/
class QDomElement;
class QDomDocument;
class Geld;
class dbID;
class KLocale;

class DocPositionBase : public QObject
{
  public:
    enum PositionType { Position, Header };
    DocPositionBase();
    DocPositionBase( const PositionType& );
    ~DocPositionBase() {}

    DocPositionBase(const DocPositionBase&);

    void setDbId( int id ) { m_dbId = id; }
    dbID dbId() { return dbID( m_dbId ); }

    void setAttribute( const Attribute& );
    void removeAttribute( const QString& );
    void loadAttributes();
    QString attribute(const QString& ) const;

    AttributeMap attributes();

  /**
   * Position means the number in the document
   */
    virtual QString position() { return m_position; }

    virtual void setToDelete( bool doit ) { mToDelete = doit; }
    virtual bool toDelete() { return mToDelete; }
    PositionType type() { return mType; }

    DocPositionBase& operator=( const DocPositionBase& );

  protected:
    int     m_dbId;
    QString m_position;
    bool    mToDelete;
    PositionType mType;
    AttributeMap mAttribs;
};


class DocPosition : public DocPositionBase
{
  public:
    DocPosition();
    DocPosition( const PositionType& );
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

    static const QString Kind;
    static const QString Discount;

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
    void setLocale( const KLocale& );
    KLocale* locale() { return &mLocale; }
  protected:
    int compareItems ( QPtrCollection::Item item1, QPtrCollection::Item item2 );

  private:
    QDomElement xmlTextElement( QDomDocument&, const QString& , const QString& );
    KLocale mLocale;
};

typedef QPtrListIterator<DocPositionList> DocPositionListIterator;

typedef QGuardedPtr<DocPositionBase> DocPositionGuardedPtr;
#endif
