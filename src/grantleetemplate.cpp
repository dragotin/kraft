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
#include <klocalizedstring.h>

#include <QDebug>

#include <QFileInfo>

#include <KTextTemplate/Engine>
#include <KTextTemplate/Context>
#include <KTextTemplate/Template>
#include <KTextTemplate/TemplateLoader>

// make this class a QObject to parent the created QObjects in addToMappingHash()
// to it. That way, the allocated objects are automatically freed by the Qt mechanism
GrantleeFileTemplate::GrantleeFileTemplate( const QString& file)
    : QObject(),
      _tmplFileName(file)
{

}

void GrantleeFileTemplate::addToMappingHash( const QString& prefix, const QVariantHash& hash)
{
    QObject *obj;

    if (prefix.isNull()) {
        return;
    }

    if (_objs.contains(prefix)) {
        obj = _objs[prefix];
    } else {
        // make the created objects a child of the _p QObject
        obj = new QObject(this);
        _objs[prefix] = obj;
    }

    QHash<QString, QVariant>::const_iterator i = hash.constBegin();
    while (i != hash.constEnd()) {
        const auto key = i.key();
        const auto val = i.value();
        obj->setProperty(key.toLocal8Bit().data(), val);
        i++;
    }
}

void GrantleeFileTemplate::addToObjMapping(const QString& key, QObject *obj)
{
    _objs.insert(key, obj);
}

QString GrantleeFileTemplate::render(bool &ok) const
{
    QScopedPointer<KTextTemplate::Engine> engine(new KTextTemplate::Engine());

    QFileInfo fi(_tmplFileName);
    ok = true; // assume all goes well.

    auto loader = QSharedPointer<KTextTemplate::FileSystemTemplateLoader>::create();
    loader->setTemplateDirs( {fi.absolutePath()} );
    engine->addTemplateLoader( loader );

    QString output;
    auto t = engine->loadByName(fi.fileName());
    if (t->error() != KTextTemplate::Error::NoError) {
        ok = false;
        output = t->errorString();
        qDebug() << "TextTemplate template load failed:" << output;
    }

    if (ok) {
        KTextTemplate::Context c;

        QHash<QString, QObject*>::const_iterator i = _objs.constBegin();
        while (i != _objs.constEnd()) {
            const QString k = i.key();
            QObject *obj = i.value();
            c.insert(k, obj);
            ++i;
        }

        output = t->render(&c);
        if (t->error() != KTextTemplate::Error::NoError) {
            ok = false;
            // Rendering error.
            output = t->errorString();
            qDebug() << "TextTemplate template err:" << output;
        }
    }

    return output;
}




