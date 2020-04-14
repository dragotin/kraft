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

#include "grantleetemplate.h"
#include "klocalizedstring.h"

#include <QDebug>

#include <QFileInfo>
#include <string.h>

#include <grantlee/engine.h>
#include <grantlee/context.h>
#include <grantlee/template.h>
#include <grantlee/templateloader.h>

GrantleeFileTemplate::GrantleeFileTemplate( const QString& file)
    :_tmplFileName(file)
{
}

void GrantleeFileTemplate::addToMapping(const QString& key, const QVariant& variant)
{
    _mapping.insert(key, variant);
}

void GrantleeFileTemplate::addToObjMapping(const QString& key, QObject *obj)
{
    _objs.insert(key, obj);
}

QString GrantleeFileTemplate::render(bool &ok) const
{
    QScopedPointer<Grantlee::Engine> engine(new Grantlee::Engine());

    QFileInfo fi(_tmplFileName);
    ok = true; // assume all goes well.

    auto loader = QSharedPointer<Grantlee::FileSystemTemplateLoader>::create();
    loader->setTemplateDirs( {fi.absolutePath()} );
    engine->addTemplateLoader( loader );

    QString output;
    auto t = engine->loadByName(fi.fileName());
    if (t->error() != Grantlee::Error::NoError) {
        ok = false;
        output = t->errorString();
        qDebug() << "Grantlee template load failed:" << output;
    }

    if (ok) {
        Grantlee::Context c(_mapping);

        QHash<QString, QObject*>::const_iterator i = _objs.constBegin();
        while (i != _objs.constEnd()) {
            const QString k = i.key();
            QObject *obj = i.value();
            c.insert(k, obj);
            ++i;
        }

        output = t->render(&c);
        if (t->error() != Grantlee::Error::NoError) {
            ok = false;
            // Rendering error.
            output = t->errorString();
            qDebug() << "Grantlee template err:" << output;
        }
    }

    return output;
}




