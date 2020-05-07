/***************************************************************************
                          katalog.h  -
                             -------------------
    begin                : Son Feb 8 2004
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

#ifndef KATALOG_H
#define KATALOG_H

#include <kraftcat_export.h>

#include <QStringList>

#include "floskeltemplate.h"
#include "catalogchapter.h"

#include "dbids.h"

/**
  *@author Klaas Freitag
  */



typedef enum {UnspecCatalog, MaterialCatalog, TemplateCatalog, PlantCatalog } KatalogType;

class QDomDocument;

class KRAFTCAT_EXPORT Katalog
{
public:
    Katalog();
    Katalog(const QString& );

    virtual ~Katalog();

    virtual int load();

    /**
     * reload the item with the given id or the entire catalog in case
     * the id is not valid.
     */
    virtual void reload( dbID ) = 0;

    virtual void setName( const QString& );
    virtual QString getName( ) const;

    /** find the ID for the corresponding chapter */
    virtual dbID chapterID(const QString&);

    /** get a list of all existing chapters of this catalog */
    virtual QList<CatalogChapter> getKatalogChapters( bool freshup = false );

    /** get the chapter name for the given ID */
    virtual QString chapterName(const dbID&);


    /** set the sortkey for a chapter. Note: The organisation of the sortkeys
     * between the different chapters is up to the caller of this method.
     */
    virtual void setChapterSortKey( const QString&, int );
    virtual int chapterSortKey( const QString& );
    /**
     * returns the KatalogType.
     */
    virtual KatalogType type();

    /** get the amount of entries in a chapter or the entire catalog */
    virtual int getEntriesPerChapter( const CatalogChapter& ) = 0;

    bool isReadOnly() { return m_readOnly; }
    void setReadOnly( bool state ) { m_readOnly = state; }

    void refreshChapterList();

    virtual QDomDocument toXML();
    virtual void writeXMLFile();

    virtual void recordUsage(int id) = 0;

    dbID id();

    QLocale *locale() { return mLocale; }
protected:
    QList<CatalogChapter> mChapters;
    QString     m_name;
    QString     m_description;
    int         m_setID;

    bool        m_readOnly;
    bool        mChapterListNeedsRefresh;
    QLocale *mLocale;

private:
    void init();
    
};

#endif
