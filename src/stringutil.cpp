/***************************************************************************
                  String helper functions
                             -------------------
    begin                : July 2023
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

#include <QString>
#include <QLocale>
#include <QDate>
#include <QDomElement>
#include <QDomDocument>

#include "format.h"

namespace KraftXml {

QDomElement textElement(QDomDocument& doc, const QString& name, const QString& value )
{
    QDomElement elem = doc.createElement( name );
    QDomText t = doc.createTextNode( value.toHtmlEscaped() );
    elem.appendChild( t );
    return elem;
}

QString childElemText(const QDomElement& elem, const QByteArray& childName)
{
    const QDomElement e = elem.firstChildElement(childName);
    const QString t = e.text();
    return t;
}

QDate childElemDate(const QDomElement& elem, const QString& childName)
{
    const QDomElement e = elem.firstChildElement(childName);
    const QString t = e.text();
    return QDate::fromString(t, "yyyy-MM-dd");
}

};

namespace KraftString {

QString replaceTags( const QString& w, QMap<QString, QString>& replaceMap )
{
    QString re{ w };

    QMultiMap<int, QString> reMap;
    for(const QString& key : replaceMap.keys()) {
        reMap.insert(key.length(), key);
    }

    QList<int> lens = reMap.keys();
    std::sort(lens.begin(), lens.end(), [](const int& i1, const int& i2) -> bool {
        return i1 > i2;
    });

    for (int len : lens) {
        const QStringList keys = reMap.values(len);
        for (const QString& k : keys) {
            re.replace(k, replaceMap[k]);
        }
    }
    return re;
}
}
