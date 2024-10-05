/***************************************************************************
               kraftattrib.cpp - Kraft Base Object Attributes
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

#include "kraftattrib.h"

#include <QDomElement>
#include <QDate>
#include <QDebug>
#include <QHash>
#include <QDomElement>
#include <QDomDocument>

KraftAttrib::KraftAttrib()
{

}

KraftAttrib::KraftAttrib(const QString& name, const QVariant& value, Type t)
    :_name(name),
      _value(value),
      _type(t)
{

}

bool KraftAttrib::operator==(const KraftAttrib &other) const
{
    return _name == other.name() &&
            _value == other.value() &&
            _type == other.type();
}

qhash_result_t qHash(const KraftAttrib &a, qhash_result_t seed)
{
    QtPrivate::QHashCombine hash;

    seed = hash(seed,a.name());
    seed = hash(seed, a.value().toString());
    seed = hash(seed, a.typeString());

    return seed;
}

QString KraftAttrib::typeString() const
{
    if (_type == Type::Integer)
        return QStringLiteral("integer");
    else if(_type == Type::String)
        return QStringLiteral("string");
    else if(_type == Type::Date)
        return QStringLiteral("date");
    else if(_type == Type::Color)
        return QStringLiteral("color");
    else if(_type == Type::Float)
        return QStringLiteral("float");
    else
        return QStringLiteral("unknown");
}

KraftAttrib::Type KraftAttrib::stringToType(const QString& type)
{
    if (type == QStringLiteral("integer")) {
        return KraftAttrib::Type::Integer;
    } else if (type == QStringLiteral("date")) {
        return KraftAttrib::Type::Date;
    } else if (type == QStringLiteral("string")) {
        return KraftAttrib::Type::String;
    } else if (type == QStringLiteral("color")) {
        return KraftAttrib::Type::Color;
    } else if (type == QStringLiteral("float")) {
        return KraftAttrib::Type::Float;
    }
    return KraftAttrib::Type::Unknown;
}

KraftAttrib::KraftAttrib(const QDomElement& elem)
{
    const QDomElement n = elem.firstChildElement("name");
    _name = n.text();

    const QDomElement t = elem.firstChildElement("type");
    _type  = stringToType(t.text());

    const QDomElement v = elem.firstChildElement("value");
    const QString vStr = v.text();

    if (_type == Type::Integer) {
        bool ok;
        int i = vStr.toInt(&ok);

        if (ok) {
            _value = QVariant(i);
        }
    } else if (_type == Type::Float) {
        bool ok;
        float f = vStr.toFloat(&ok);

        if (ok) {
            _value = QVariant(f);
        }
    } else if (_type == Type::Date) {
        const QDate d = QDate::fromString(vStr, "yyyy-MM-dd");
        if (d.isValid()) {
            _value = QVariant(d);
        }
    } else if (_type == Type::Color) {
        // not yet implemented.
        qDebug() << "Color not yet implemented";
    } else /* _type == Type::String */ {
        _value = QVariant(vStr);
    }

}

QDomElement KraftAttrib::toXml(QDomDocument& xmldoc) const
{
    QDomElement attr = xmldoc.createElement("attrib");
    {
        QDomElement name = xmldoc.createElement("name");
        name.setNodeValue(_name);
        attr.appendChild(name);
    }
    {
        QDomElement value = xmldoc.createElement("value");
        value.setNodeValue(_value.toString());
        attr.appendChild(value);
    }
    {
        QDomElement type = xmldoc.createElement("type");
        type.setNodeValue(typeString());
        attr.appendChild(type);
    }
    return attr;
}
