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
#include <QPointer>
#include <QObject>

#include <QList>

// application specific includes
#include "dbids.h"
#include "calcpart.h"
#include "attribute.h"
#include "einheit.h"
#include "kraftobj.h"

/**
@author Klaas Freitag
*/
class QString;
class QDomElement;
class QDomDocument;
class Geld;
class dbID;
class QLocale;
class PositionViewWidget;

class DocPositionBase : public KraftObj
{

public:
    enum PositionType { Position, ExtraDiscount, Text, Demand, Alternative };
    enum TaxType { TaxInvalid = 0, TaxNone = 1, TaxReduced = 2, TaxFull = 3, TaxIndividual = 4 };

    DocPositionBase();
    DocPositionBase( const PositionType& );
    ~DocPositionBase() {}

    DocPositionBase(const DocPositionBase&);

    void setDbId( int id ) { m_dbId = id; }
    dbID dbId() { return dbID( m_dbId ); }

    void setText( const QString& string ) { m_text = string; }
    QString text() const { return m_text; }
    
    int taxTypeNumeric();
    TaxType taxType();
    void setTaxType( DocPositionBase::TaxType );
    void setTaxType( int );
    void setTaxType(const QString&);

    /**
   * Position means the number in the document
   */
    int positionNumber() { return m_position; }
    void setPositionNumber( const int& pos ) { m_position = pos; }
    void setToDelete( bool doit ) { mToDelete = doit; }
    bool toDelete() { return mToDelete; }
    PositionType type() { return mType; }
    QString typeStr();

    DocPositionBase& operator=( const DocPositionBase& );

protected:
    int     m_dbId;
    int     m_position;
    QString m_text;
    bool    mToDelete;
    TaxType mTaxType;
    PositionType mType;
};


class DocPosition : public DocPositionBase
{
public:
    DocPosition();
    DocPosition( const PositionType& );

    void setUnit( const Einheit& unit ) { m_unit = unit; }
    Einheit unit() const { return m_unit; }

    void setUnitPrice( const Geld& g ) { m_unitPrice = g; }
    Geld unitPrice() const { return m_unitPrice; }
    Geld overallPrice();

    void setAmount( double amount ) { m_amount = amount; }
    double amount() { return m_amount; }
    
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

class DocPositionList : public QList<DocPositionBase*>
{
public:
    DocPositionList();

    // QDomElement domElement( QDomDocument& );
    DocPositionBase *positionFromId( int id );
    QString posNumber( DocPositionBase* );

    Geld nettoPrice();
    Geld bruttoPrice( double fullTax, double reducedTax );
    Geld taxSum(double fullTax, double redTax );
    Geld fullTaxSum( double fullTax );
    Geld reducedTaxSum( double reducedTax );

protected:
    int compareItems ( DocPosition *dp1, DocPosition *dp2 );

private:
    QDomElement xmlTextElement( QDomDocument&, const QString& , const QString& );
};

typedef QListIterator<DocPositionBase*> DocPositionListIterator;

typedef DocPositionBase* DocPositionGuardedPtr;
#endif
