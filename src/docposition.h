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
class PositionViewWidget;

class DocPositionBase : public QObject
{
  public:
  enum PositionType { Position, ExtraDiscount, Header };
  enum TaxType { TaxInvalid, TaxNone, TaxReduced, TaxFull };

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
    void setAttributeMap( AttributeMap );

    void setText( const QString& string ) { m_text = string; }
    QString text() const { return m_text; } ;

    void setTag( const QString& );
    void removeTag( const QString& );
    bool hasTag( const QString& );
    QStringList tags();
    
    int taxTypeNumeric();
    void setTaxType( DocPositionBase::TaxType );

  /**
   * Position means the number in the document
   */
    virtual QString positionNumber() { return m_position; } // FIXME: Position not working
    virtual void setPositionNumber( const QString& str ) { m_position = str; }
    virtual void setToDelete( bool doit ) { mToDelete = doit; }
    virtual bool toDelete() { return mToDelete; }
    PositionType type() { return mType; }

    DocPositionBase& operator=( const DocPositionBase& );

  protected:
    int     m_dbId;
    QString m_position;
    QString m_text;
    bool    mToDelete;
    TaxType mTaxType;
    PositionType mType;
    AttributeMap mAttribs;
};


class DocPosition : public DocPositionBase
{
  public:
    DocPosition();
    DocPosition( const PositionType& );
    ~DocPosition(){};


    void setUnit( const Einheit& unit ) { m_unit = unit; }
    Einheit unit() const { return m_unit; }

    void setUnitPrice( const Geld& g ) { m_unitPrice = g; }
    Geld unitPrice() const { return m_unitPrice; }
    Geld overallPrice();

    void setAmount( double amount ) { m_amount = amount; }
    double amount() { return m_amount; }

    DocPosition& operator=( const DocPosition& );
    
    PositionViewWidget* associatedWidget() { return mWidget; }
    void setAssociatedWidget( PositionViewWidget *w ) { mWidget = w; }

    static const QString Kind;
    static const QString Discount;
    static const QString Tags;
    static const QString ExtraDiscountTagRequired;

  private:
    Einheit m_unit;
    Geld    m_unitPrice;
    double  m_amount;
    PositionViewWidget *mWidget;

    // No calculation yet

};

class DocPositionList : public QPtrList<DocPositionBase>
{
  public:
    DocPositionList();

    QDomElement domElement( QDomDocument& );
    DocPositionBase *positionFromId( int id );
    QString posNumber( DocPositionBase* );
    void setLocale( KLocale* );
    KLocale* locale() { return mLocale; }

    Geld nettoPrice();
    Geld bruttoPrice( double fullTax, double reducedTax );
    Geld taxSum( double fullTax, double reducedTax );
    Geld fullTaxSum( double fullTax );
    Geld reducedTaxSum( double reducedTax );

  protected:
    int compareItems ( QPtrCollection::Item item1, QPtrCollection::Item item2 );

  private:
    QDomElement xmlTextElement( QDomDocument&, const QString& , const QString& );
    KLocale *mLocale;
};

typedef QPtrListIterator<DocPositionBase> DocPositionListIterator;

typedef QGuardedPtr<DocPositionBase> DocPositionGuardedPtr;
#endif
