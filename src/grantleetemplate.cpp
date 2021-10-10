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
    _engine = new Grantlee::Engine();

    QFileInfo fi(_tmplFileName);

    auto loader = QSharedPointer<Grantlee::FileSystemTemplateLoader>::create();
    loader->setTemplateDirs( {fi.absolutePath()} );
    _engine->addTemplateLoader( loader );

    _template = _engine->loadByName(fi.fileName());
    if (_template->error() != Grantlee::Error::NoError) {
        qDebug() << "Grantlee template load failed:" << _template->errorString();
    }

}

bool GrantleeFileTemplate::isOk() const
{
    return _template->error() == Grantlee::Error::NoError;
}

void GrantleeFileTemplate::addToMapping(const QString& key, const QVariant& variant)
{
    _mapping.insert(key, variant);
}

void GrantleeFileTemplate::addToObjMapping(const QString& key, QObject *obj)
{
    _objs.insert(key, obj);
}

QString GrantleeFileTemplate::render(bool *ok) const
{
    QScopedPointer<Grantlee::Engine> engine(new Grantlee::Engine());

    Grantlee::Context c(_mapping);

    QHash<QString, QObject*>::const_iterator i = _objs.constBegin();
    while (i != _objs.constEnd()) {
        const QString k = i.key();
        QObject *obj = i.value();
        c.insert(k, obj);
        ++i;
    }

    QString output = _template->render(&c);
    if (_template->error() != Grantlee::Error::NoError) {
        if (ok) *ok = false;
        // Rendering error.
        output = _template->errorString();
        qDebug() << "Grantlee template err:" << output;
    }

    return output;
}




