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
class KraftView;  

class DocPositionBase : public QObject
{
  public:
    enum PositionType { Position, ExtraDiscount, Header };
    DocPositionBase();
    DocPositionBase( const PositionType& );
    virtual ~DocPositionBase() {}

    DocPositionBase(const DocPositionBase&);

    DocPositionBase& operator=( const DocPositionBase& );

    void setDbId( int id ) { m_dbId = id; }
    dbID dbId() { return dbID( m_dbId ); }

    void setAttribute( const Attribute& );
    void removeAttribute( const QString& );
    void loadAttributes();
    QString attribute(const QString& ) const;

    AttributeMap attributes();

    void setText( const QString& string ) { m_text = string; }
    QString text() const { return m_text; } ;

  /**
   * Position means the number in the document
   */
    virtual int position() { return m_position; }
    virtual void setPosition( int pos ) { m_position = pos; }

    virtual void setToDelete( bool doit ) { mToDelete = doit; }
    virtual bool toDelete() { return mToDelete; }
    PositionType type() { return mType; }


  protected:
    int     m_dbId;
    int     m_position;
    QString m_text;
    bool    mToDelete;
    PositionType mType;
    AttributeMap mAttribs;
};

class DocPosition;

class Pricing
{
  public:
    Pricing( DocPosition* );
    virtual ~Pricing();
    
    void setAmount( double amount ) { mAmount = amount; }
    double amount() { return mAmount; }
    
    virtual void setUnitPrice( const Geld& g ) { mUnitPrice = g; }
    virtual Geld unitPrice() const { return mUnitPrice; }
    virtual Geld overallPrice();

  protected:  
    DocPosition* mMyPosition;
  private:
    Geld         mUnitPrice;
    double       mAmount;

};

class DiscountPricing : public Pricing 
{
  public:
  DiscountPricing( DocPosition*, KraftView* );
    ~DiscountPricing();

    void setDiscount( double );
    double discount();

    void setFilterTag( const QString& );
    QString tag() const;

    Geld unitPrice() const;
    Geld overallPrice();
  private:
    double  mDiscount;
    QString mTag;
    KraftView *mKraftView;

};


class DocPosition : public DocPositionBase
{
  public:
    DocPosition();
    DocPosition( const PositionType& );
    ~DocPosition();

    DocPosition( const DocPosition& );
    DocPosition& operator=( const DocPosition& );

    void setUnit( const Einheit& unit ) { m_unit = unit; }
    Einheit unit() const { return m_unit; }

    Pricing *pricing() { return mPricing; }
    void setPricing( Pricing* );

    void setUnitPrice( const Geld& g ) { mPricing->setUnitPrice( g ); }
    Geld unitPrice() const     { return mPricing->unitPrice(); }
    Geld overallPrice()        { return mPricing->overallPrice(); }

    void setAmount( double d ) { mPricing->setAmount(d); }
    double amount()            { return mPricing->amount(); }

    static const QString Kind;
    static const QString Discount;
    // static const QString Discount;

  private:
    Pricing *mPricing;
    Einheit m_unit;
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
    void setLocale( KLocale* );
    KLocale* locale() { return mLocale; }

  protected:
    int compareItems ( QPtrCollection::Item item1, QPtrCollection::Item item2 );

  private:
    QDomElement xmlTextElement( QDomDocument&, const QString& , const QString& );
    KLocale *mLocale;
};

typedef QPtrListIterator<DocPositionList> DocPositionListIterator;

typedef QGuardedPtr<DocPositionBase> DocPositionGuardedPtr;
#endif
