/***************************************************************************
                     kraftobj.h - Kraft Base Object
                             -------------------
    begin                : Feb. 2, 2023
    copyright            : (C) 2023 by Klaas Freitag
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

#include "kraftobj.h"

KraftObj::KraftObj()
     :_modified{false}
{

}

QString KraftObj::uuid() const
{
    if (_uuid.isNull()) {
        return QString();
    }
    return _uuid.toString(QUuid::WithoutBraces);
}

bool KraftObj::hasAttribute(const QString& name)
{
    return _attribs.contains(name);
}

void KraftObj::setAttribute(const KraftAttrib& attrib)
{
    const QString name = attrib.name();

    _attribs.insert(attrib.name(), attrib);
}

KraftAttrib KraftObj::attribute(const QString& name)
{
    if (_attribs.contains(name))
        return _attribs[name];
    else
        return KraftAttrib();
}


void KraftObj::addTag(const QString& tag)
{
    if (!tag.isEmpty())
        _tags.insert(tag);
}
void KraftObj::removeTag(const QString& tag)
{
    _tags.remove(tag);
}

bool KraftObj::hasTag(const QString& tag) const
{
    return _tags.contains(tag);
}

void KraftObj::setTags(const QStringList& list)
{
    for( const auto &l : list) {
        addTag(l);
    }
}

QStringList KraftObj::allTags() const
{
    QStringList re;

    QSet<QString>::const_iterator i = _tags.constBegin();
    while (i != _tags.constEnd()) {
        re << *i;
        ++i;
    }
    return re;
}
