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
#include "stringutil.h"

#include <QDomElement>

KraftObj::KraftObj()
     :_modified{false}
{

}

void KraftObj::setModified(bool m)
{
    _modified = m;
    _lastModified = QDateTime::currentDateTime();
}

QString KraftObj::uuid() const
{
    if (_uuid.isNull()) {
            return QString();
    }
    return _uuid.toString(QUuid::WithoutBraces);
}

QString KraftObj::createUuid()
{
    if (_uuid.isNull()) {
        _uuid = QUuid::createUuid();
        setModified();
    }
    return uuid();
}

bool KraftObj::hasAttribute(const QString& name) const
{
    return _attribs.contains(name);
}

void KraftObj::setAttribute(const KraftAttrib& attrib)
{
    if (!attrib.name().isEmpty()) {
        _attribs.insert(attrib.name(), attrib);
        setModified();
    }
}

void KraftObj::removeAttribute(const QString& name)
{
    if (!name.isEmpty() && _attribs.contains(name)) {
        _attribs.remove(name);
        setModified();
    }
}

KraftAttrib KraftObj::attribute(const QString& name) const
{
    if (_attribs.contains(name))
        return _attribs[name];
    else
        return KraftAttrib();
}


void KraftObj::addTag(const QString& tag)
{
    if (!tag.isEmpty() && !_tags.contains(tag)) {
        _tags.insert(tag);
        setModified();
    }
}
void KraftObj::removeTag(const QString& tag)
{
    if (_tags.contains(tag)) {
        _tags.remove(tag);
        setModified();
    }
}

bool KraftObj::hasTag(const QString& tag) const
{
    return _tags.contains(tag);
}

void KraftObj::setTags(const QStringList& list)
{
    _tags.clear();
    for( const auto &l : list) {
        addTag(l);
    }
    setModified();
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

void KraftObj::parseKobjXml(QDomElement &elem)
{
    Q_ASSERT(! elem.isNull());
    _uuid = QUuid::fromString(KraftXml::childElemText(elem, "uuid"));

    QDate d = KraftXml::childElemDate(elem, "lastModified");
    QDateTime dt;
    dt.setDate(d);
    setLastModified(dt);

    QDomElement attribsElem = elem.firstChildElement("attribs");
    QDomElement attribElem = attribsElem.firstChildElement("attrib");
    while(!attribElem.isNull()) {
        KraftAttrib attr(attribElem);
        setAttribute(attr);
        attribElem = attribElem.nextSiblingElement("attrib");
    }

    QDomElement tagsElem = elem.firstChildElement("tags");
    QDomElement tagElem = tagsElem.firstChildElement("tag");
    while (!tagElem.isNull()) {
        const QString t = tagElem.text();
        addTag(t);
        tagElem = tagElem.nextSiblingElement("tag");
    }

}

QDomElement KraftObj::kobjXml(QDomDocument &xmldoc, const QString& elemName) const
{
    QDomElement objAttr = xmldoc.createElement(elemName);
    {
        const auto uid = uuid();
        objAttr.appendChild(KraftXml::textElement(xmldoc, "uuid", uid));
    }
    {
        const auto dt = lastModified();
        if (dt.isValid()) {
            objAttr.appendChild(KraftXml::textElement(xmldoc, "lastModified", dt.toString(Qt::ISODate)));
        }
    }
    {
        QDomElement attribsXml = xmldoc.createElement("attribs");
        const auto attribs = attributes();
        for (const auto &a : attribs) {
            attribsXml.appendChild(a.toXml(xmldoc));
        }
        objAttr.appendChild(attribsXml);
    }
    {
        QDomElement tagsXml = xmldoc.createElement("tags");
        for (const QString& tag : allTags()) {
            tagsXml.appendChild(KraftXml::textElement(xmldoc, "tag", tag));
        }
        objAttr.appendChild(tagsXml);
    }
    return objAttr;
}
