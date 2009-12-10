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


#include <QString>
#include <QDateTime>
#include <QList>

#include "kraftglobals.h"
#include "einheit.h"
#include "calcpart.h"
#include "catalogtemplate.h"

/**
  *@author Klaas Freitag
  */

class QTreeWidgetItem;
class TemplateSaverBase;
class QDomDocument;
class QDomElement;

class FloskelTemplate: public CatalogTemplate {
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
    Geld unitPrice();
    Geld kostenPerKalcPart( const QString& part);

    QString getText() const { return m_text; }
    void setText( const QString& str ) { m_text = str; }
    void addCalcPart( CalcPart* cpart );
    void removeCalcPart( CalcPart *cpart );
    CalcPartList getCalcPartsList();
    CalcPartList getCalcPartsList(const QString& );
  CalcPartList decoupledCalcPartsList();

    Einheit einheit() const;
    void setEinheitId(int id);

    void setBenefit( double );
    double getBenefit();

    int  getTemplID() { return m_templID; }
    void setTemplID( int );

    int getChapterID() { return m_chapter; }
    void setChapterID(int id);

    bool hasTimeslice() { return m_zeitbeitrag; }
    void setHasTimeslice(bool ts) { m_zeitbeitrag = ts; }

    QDateTime modifyDate() { return m_modifyDate; }
    QDateTime createDate() { return m_createDate; }

    void setListViewItem( QTreeWidgetItem *it ) { m_listViewItem = it; }
    QTreeWidgetItem* getListViewItem() { return m_listViewItem; }

    virtual bool save();
    // virtual QDomElement toXML( QDomDocument&);

  FloskelTemplate& operator= ( FloskelTemplate& );
protected:
    virtual TemplateSaverBase* getSaver();
    virtual void deepCopyCalcParts( FloskelTemplate& );

private: // Private methods
#if 0
    QDomElement createDomNode( QDomDocument, const QString&, const QString&);
    void materialPartsToXML( QDomDocument&, QDomElement& );
    void fixPartsToXML( QDomDocument&, QDomElement& );
    void timePartsToXML( QDomDocument&, QDomElement& );
#endif
    virtual Geld calcPreis();

    QString          m_text;
    int              m_einheitID;
    int              m_templID;  // Database ID
    int              m_chapter;
    CalcPartList     m_calcParts;
    double           m_gewinn;
    bool             m_zeitbeitrag;
    QDateTime        m_modifyDate, m_createDate;
    Geld             m_preis; // preis only valid for manual calculation.
    QTreeWidgetItem  *m_listViewItem;
    TemplateSaverBase *m_saver;    /**  */

};

class FloskelTemplateList :public QList<FloskelTemplate*>
{
public:
  FloskelTemplateList() { }
};

typedef QListIterator<FloskelTemplate*> FloskelTemplateListIterator;


#endif
