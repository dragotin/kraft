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
#include <QFileInfo>
#include <QMap>

class KraftDoc;
class DocDigest;

class XmlDocIndex
{
public:
    XmlDocIndex();

    void setBasePath(const QString& basePath);
    static QMultiMap<QDate, QString> const &dateMap();

    const QFileInfo xmlPathByIdent(const QString& ident);
    const QFileInfo xmlPathByUuid(const QString& uuid);

    const QFileInfo pdfPathByIdent(const QString& ident);
    const QFileInfo pdfPathByUuid(const QString& uuid);

    // Adds an entry to the index, used with newly created documents
    void addEntry(KraftDoc *doc);
    void updateEntry(KraftDoc *doc);

    bool pdfOutdated(const QString& uuid);

    DocDigest findDigest(const QString& year, const QString& uuid);

private:
    const QFileInfo pathByUuid(const QString& uuid, const QString& extension = QString());

    bool buildIndexFromFile();
    void buildIndexFile();

    static QMap<QString, QString> _identMap;
    static QMultiMap<QDate, QString> _dateMap;
    static QMap<QString, QString> _uuidMap;
};

#endif // XMLDOCINDEX_H
