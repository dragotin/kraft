/***************************************************************************
                          XML Document Index
                             -------------------
    begin                : Feb. 2023
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

#ifndef XMLDOCINDEX_H
#define XMLDOCINDEX_H

#include <QObject>
#include <QMap>

class XmlDocIndex
{
public:
    XmlDocIndex(const QString& basePath);

    const QString pathByIdent(const QString& ident);
    static QMap<QDate, QString> const &dateMap();

private:
    void buildIndex();

    QString _basePath;

    static QMap<QString, QString> _identMap;
    static QMap<QDate, QString> _dateMap;
};

#endif // XMLDOCINDEX_H
