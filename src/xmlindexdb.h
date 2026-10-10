/***************************************************************************
                    XML Document Index in a SQLite database
                             -------------------
    begin                : Oct. 2026
    copyright            : (C) 2026 by Klaas Freitag
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

#ifndef XMLINDEXDB_H
#define XMLINDEXDB_H

#include <QDate>
#include <QFileInfo>
#include <QJsonObject>
#include <QMultiMap>
#include <QSqlDatabase>

class KraftDoc;
class DocDigest;
class QSqlQuery;
class DocTypes;

/**
 * @brief XmlIndexDb - document index of the XML documents, kept in an
 * independent SQLite database file in the Kraft v2 root directory.
 *
 * It is a drop-in replacement for XmlDocIndex with the same interface.
 * The database uses its own named connection, so it does not interfere
 * with the default connection used by KraftDB.
 *
 * Table layout:
 *
 *   docindex
 *     uuid          TEXT PRIMARY KEY  - document uuid
 *     ident         TEXT              - document ident, e.g. "2-2024", empty for drafts
 *     docType       TEXT NOT NULL     - document type string
 *     docDate       TEXT              - document date, ISO format (yyyy-MM-dd)
 *     subPath       TEXT NOT NULL     - path relative to the xml/pdf base dir,
 *                                       without extension, e.g. "2024/12/<uuid>"
 *     lastModified  TEXT              - ISO date time
 *     clientAddress TEXT
 *     prjtLabel     TEXT
 *     state         TEXT              - KraftDocState string
 *     whiteboard    TEXT
 *
 *   indexes on ident and docDate.
 *
 *   meta (key TEXT PRIMARY KEY, value TEXT) - holds the schema version.
 *
 * If the database file does not exist, it is created and populated: from
 * the legacy JSON index file if available, otherwise by scanning all XML
 * documents.
 */
class XmlIndexDb
{
public:
    XmlIndexDb();

    void setBasePath(const QString &basePath);
    static QMultiMap<QDate, QString> const &dateMap();

    const QFileInfo xmlPathByIdent(const QString &ident);
    const QFileInfo xmlPathByUuid(const QString &uuid);

    const QFileInfo pdfPathByIdent(const QString &ident);
    const QFileInfo pdfPathByUuid(const QString &uuid);

    // Adds an entry to the index, used with newly created documents
    void addEntry(KraftDoc *doc);
    void updateEntry(KraftDoc *doc);

    bool pdfOutdated(const QString &uuid);

    // uuid may also be a sub path ending with the uuid, as stored in the dateMap
    DocDigest findDigest(const QString &year, const QString &uuid);

    DocDigest toDocDigest(QJsonObject obj);

    // all digests in the index, sorted by document date, newest first
    QList<DocDigest> allDigests();

    static const QString ConnectionName;
    static const QString DbFileName;
    static const int SchemaVersion;

private:
    const QFileInfo fullPathWithExtension(const QString &subPath, const QString &extension);

    QSqlDatabase db() const;
    bool openDb(const QString &dbFile);
    bool createSchema();
    QString subPathBy(const QString &column, const QString &value);
    bool writeEntry(KraftDoc *doc, const QString &subPath);
    DocDigest toDocDigest(const QSqlQuery &q, const DocTypes &docTypes);

    bool importJsonIndex();
    void buildIndexFromXml();
    void buildDateMap();

    static QString subPathForDoc(KraftDoc *doc);

    static QMultiMap<QDate, QString> _dateMap;
};

#endif // XMLINDEXDB_H
