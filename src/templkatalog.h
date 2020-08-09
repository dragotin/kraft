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

#ifndef TEMPLKATALOG_H
#define TEMPLKATALOG_H

#include <sys/types.h>

#include "floskeltemplate.h"
#include "katalog.h"
#include "dbids.h"

/**
  *@author Klaas Freitag
  */
class MaterialCalcPart;
class QDomDocument;

class TemplKatalog : public Katalog
{
public:
    TemplKatalog(const QString& name);
    ~TemplKatalog();

    int load(const QString&);
    int load() override;
    void reload( dbID ) override;

    /** No descriptions */
    FloskelTemplateList getFlosTemplates( const CatalogChapter& chapter );

    KatalogType type() override { return TemplateCatalog; }

    QDomDocument toXML() override;

    /** get the amount of entries in a chapter or the entire catalog */
    int getEntriesPerChapter( const CatalogChapter& ) override;

    int addNewTemplate( FloskelTemplate *tmpl );

    void recordUsage(int id) override;

public slots:
    void writeXMLFile() override;
    void deleteTemplate( int );
private:
    int loadCalcParts( FloskelTemplate* );
    int loadTimeCalcParts( FloskelTemplate* );
    int loadFixCalcParts( FloskelTemplate* );
    int loadMaterialCalcParts( FloskelTemplate * );

    FloskelTemplateList m_flosList;
};

#endif
