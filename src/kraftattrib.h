/***************************************************************************
                 kraftattrib.h - Kraft Base Object Attributes
                             -------------------
    begin                : Feb. 4, 2023
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
#ifndef KRAFTATTRIB_H
#define KRAFTATTRIB_H

#include <QtGlobal>
#include <QVariant>
#include <QHash>

class QDomElement;

class KraftAttrib;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
using qhash_result_t = uint;
#else
using qhash_result_t = size_t;
#endif

qhash_result_t qHash(const KraftAttrib &a, qhash_result_t seed);

class KraftAttrib
{
public:
    explicit KraftAttrib();

    enum class Type {
        Unknown,
        Integer,
        String,
        Date,
        Color,
        Float
    };

    KraftAttrib(const QString& name, const QVariant& value, Type t);
    KraftAttrib(const QDomElement& elem);

    bool operator==(const KraftAttrib &other) const;

    QString name() const     { return _name;  }
    QVariant value() const   { return _value; }
    Type type() const        { return _type;  }

    static KraftAttrib::Type stringToType(const QString& type);
    QString typeString() const;

private:
    QString  _name;
    QVariant _value;
    Type     _type;

};


#endif // KRAFTATTRIB_H
