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

#ifndef KRAFTOBJ_H
#define KRAFTOBJ_H

#include "kraftattrib.h"

#include <QObject>
#include <QUuid>
#include <QDateTime>
#include <QVariant>
#include <QSet>

/**
 * @brief The KraftObj class - a base object for most Objects in Kraft.
 *
 * It comes with certain properties that all the object share.
 */
class KraftObj
{
private:
    QUuid     _uuid;
    QDateTime _lastModified;
    bool      _modified;

    QMap<QString, KraftAttrib> _attribs;
    QSet<QString> _tags;

public:
    explicit KraftObj();

    QString uuid() const;
    QString createUuid();
    void setUuid( const QString& str ) { _uuid = QUuid(str); }

    QDateTime lastModified() const { return _lastModified; }
    void setLastModified( QDateTime d ) { _lastModified = d; }

    bool modified() const {return _modified;}
    void setModified(bool m = true);

    bool hasAttribute(const QString& name) const;
    void setAttribute(const KraftAttrib& attrib);
    void removeAttribute(const QString& name);

    KraftAttrib attribute(const QString& name) const;

    /* Convenience for the attributes that are plain strings, which most are.
     *
     * stringAttribute() returns the value trimmed, or an empty string if the
     * object has no such attribute. setStringAttribute() removes the attribute if
     * the value is empty, so that a value the user cleared leaves nothing behind.
     */
    QString stringAttribute(const QString& name) const;
    void setStringAttribute(const QString& name, const QString& value);
    QMap<QString,KraftAttrib> attributes() const { return _attribs; }

    void wipeAndSetTags(const QStringList& list);
    void addTag(const QString& tag);
    void removeTag(const QString& tag);
    bool hasTag(const QString& tag) const;
    QStringList allTags() const;

    QDomElement kobjXml(QDomDocument &xmldoc, const QString& elemName={"kobj"}) const;
    void parseKobjXml(QDomElement &elem);
};

Q_DECLARE_METATYPE(KraftObj)

#endif // KRAFTOBJ_H
