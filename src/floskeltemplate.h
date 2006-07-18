/***************************************************************************
                          floskeltemplate.h  -
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

#ifndef FLOSKELTEMPLATE_H
#define FLOSKELTEMPLATE_H


#include <qstring.h>
#include <qdatetime.h>
#include <qptrlist.h>

#include "kraftglobals.h"
#include "einheit.h"
#include "calcpart.h"

/**
  *@author Klaas Freitag
  */

typedef enum {Unknown, ManualPrice, Calculation} CalculationType;

class KListViewItem;
class TemplateSaverBase;
class QDomDocument;
class QDomElement;

class FloskelTemplate {
public:
    FloskelTemplate();
    FloskelTemplate(int tID, const QString& text, int einheit,
                    int chapter, int calcKind,
                    const QDateTime&, const QDateTime& );
    FloskelTemplate( FloskelTemplate& );

    virtual ~FloskelTemplate();
    /** No descriptions */
    void setManualPrice( Geld p ) { m_preis = p; }
    Geld manualPrice() { return m_preis; }
    Geld einheitsPreis();
    Geld kostenPerKalcPart( const QString& part);

    QString getText() const { return m_text; }
    void setText( const QString& str ) { m_text = str; }
    void addCalcPart( CalcPart* cpart );
    void removeCalcPart( CalcPart *cpart );
    CalcPartList getCalcPartsList();
    CalcPartList getCalcPartsList(const QString& );

    CalculationType calcKind();
    void setCalculationType( CalculationType t );
    QString calcKindString() const ;

    Einheit einheit() const;
    void setEinheitId(int id);

    void setGewinn( double );
    double getGewinn();

    int  getTemplID() { return m_templID; }
    void setTemplID( int );

    int getChapterID() { return m_chapter; }
    void setChapterID(int id);

    bool hasTimeslice() { return m_zeitbeitrag; }
    void setHasTimeslice(bool ts) { m_zeitbeitrag = ts; }

    QDateTime modifyDate() { return m_modifyDate; }
    QDateTime createDate() { return m_createDate; }

    void setListViewItem( KListViewItem *it ) { m_listViewItem = it; }
    KListViewItem* getListViewItem() { return m_listViewItem; }

    virtual bool save();
    virtual QDomElement toXML( QDomDocument&);

protected:
    virtual TemplateSaverBase* getSaver();

    
  
private: // Private methods
    QDomElement createDomNode( QDomDocument, const QString&, const QString&);
    void materialPartsToXML( QDomDocument&, QDomElement& );
    void fixPartsToXML( QDomDocument&, QDomElement& );
    void timePartsToXML( QDomDocument&, QDomElement& );
    
    virtual Geld calcPreis();

    QString          m_text;
    int              m_einheitID;
    int              m_templID;  // Database ID
    int              m_chapter;
    CalcPartList     m_calcParts;
    double           m_gewinn;
    bool             m_zeitbeitrag;
    QDateTime        m_modifyDate, m_createDate;
    CalculationType  m_calcType;
    Geld             m_preis; // preis only valid for manual calculation.
    KListViewItem   *m_listViewItem;
    TemplateSaverBase *m_saver;    /**  */

};

typedef QPtrList<FloskelTemplate> FloskelTemplateList;
typedef QPtrListIterator<FloskelTemplate> FloskelTemplateListIterator;


#endif
