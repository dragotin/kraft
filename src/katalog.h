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

#include <qptrlist.h>
#include <qdict.h>
#include <qstringlist.h>

#include "floskeltemplate.h"
#include "dbids.h"

/**
  *@author Klaas Freitag
  */



typedef enum {UnspecKatalog, MaterialKatalog, TemplateKatalog, PlantCatalog } KatalogType;

class QDomDocument;

class Katalog
{
public:
    Katalog(const QString& name);
    Katalog();
    virtual ~Katalog();

    virtual int load();

    virtual void setName( const QString& );
    virtual QString getName( ) const;

    /** find the ID for the corresponding chapter */
    virtual int chapterID(const QString&);

    /** get a list of all existing chapters of this catalog */
    virtual QStringList getKatalogChapters( bool freshup = false );

    /** get the chapter name for the given ID */
    virtual QString chapterName(const dbID&);

    /** Add a catalog chapter */
    virtual void addChapter( const QString&, int );

    /** remove a catalog chapter and move existing entries in it to the replacement
        catalog. */
    virtual bool removeChapter( const QString&, const QString& replace = QString() );

    /** rename catalog chapter */
    virtual void renameChapter( const QString&, const QString& );

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
    virtual int getEntriesPerChapter( const QString& = QString() ) = 0;

    bool isReadOnly() { return m_readOnly; }
    void setReadOnly( bool state ) { m_readOnly = state; }


    virtual QDomDocument toXML();
    virtual void writeXMLFile();
protected:
    dbIdDict   *m_chapterIDs;
    QStringList m_chapters;
    QString     m_name;
    QString     m_description;
    int         m_setID;

    bool        m_readOnly;
private:
    void init();
};

#endif
