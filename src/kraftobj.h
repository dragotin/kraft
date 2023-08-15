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

public:
    explicit KraftObj();

    QString uuid() const;
    QString createUuid();
    void setUuid( const QString& str ) { _uuid = QUuid(str); }

    QDateTime lastModified() const { return _lastModified; }
    void setLastModified( QDateTime d ) { _lastModified = d; }

    bool modified() {return _modified;}

    bool hasAttribute(const QString& name);
    void setAttribute(const KraftAttrib& attrib);
    KraftAttrib attribute(const QString& name);
    QMap<QString,KraftAttrib> attributes() const { return _attribs; }

    void setTags(const QStringList& list);
    void addTag(const QString& tag);
    void removeTag(const QString& tag);
    bool hasTag(const QString& tag) const;
    QStringList allTags() const;

public slots:
    void setModified() {_modified = true;}

signals:

protected:
    QUuid     _uuid;
    QDateTime _lastModified;
    bool      _modified;

    QMap<QString, KraftAttrib> _attribs;
    QSet<QString> _tags;
};

Q_DECLARE_METATYPE(KraftObj)

#endif // KRAFTOBJ_H
