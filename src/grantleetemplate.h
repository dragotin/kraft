/***************************************************************************
           GrantleeTemplate.cpp - fill a template with text tags
                             -------------------
    begin                : March 2020
    copyright            : (C) 2020 by Klaas Freitag
    email                : kraft@freisturz.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef GRANTLEETEMPLATE_H
#define GRANTLEETEMPLATE_H

#include <qobject.h>
#include <qmap.h>
#include <qstring.h>

#include "texttemplateinterface.h"

#include <grantlee/engine.h>
#include <grantlee/template.h>

class GrantleeFileTemplate : public QObject
{
    Q_OBJECT
public:
    GrantleeFileTemplate( const QString& file);

    void addToObjMapping(const QString& key, QObject *obj);
    void addToMappingHash( const QString& prefix, const QVariantHash& hash);

    QString render(bool &ok) const;

private:
    const QString& _tmplFileName;
    QHash<QString, QObject*> _objs;
};

#endif
