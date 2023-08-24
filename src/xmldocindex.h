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
#include <QFuture>

class KraftDoc;

class XmlDocIndex
{
public:
    XmlDocIndex();

    void setBasePath(const QString& basePath);

    const QString xmlPathByIdent(const QString& ident);
    const QString xmlPathByUuid(const QString& uuid);
    static QMultiMap<QDate, QString> const &dateMap();

    const QString pdfPathByUuid(const QString& uuid);

    // Adds an entry to the index, used with newly created documents
    void addEntry(KraftDoc *doc, const QString &xmlFile);

private:
    void buildIndex();

    static QString _basePath;

    static QMap<QString, QString> _identMap;
    static QMultiMap<QDate, QString> _dateMap;
    static QMap<QString, QString> _uuidMap;
    static QFuture<void> _future;
};

#endif // XMLDOCINDEX_H
