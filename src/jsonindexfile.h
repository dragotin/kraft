/***************************************************************************
                          JSON Index File of all documents
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

#ifndef JSONINDEXFILE_H
#define JSONINDEXFILE_H

#include <QString>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

class KraftDoc;
class DocDigest;
class QFileInfo;

/**
 * @brief JsonIndexFile - simple persistence of the document index
 *
 */

class JsonIndexFile
{
public:
    JsonIndexFile();

    /**
     * @brief indexFileInfo
     *
     * reads the base path from the Defaultprovider
     *
     * @return a FileInfo object pointing to the index file
     */
    QFileInfo indexFileInfo();

    /**
     * @brief years - lists all the indexed years
     *
     * content of the list can be used to get all docs of a specific year
     *
     * @return Stringlist of all years in the index
     */
    QStringList years();

    /**
     * @brief docsPerYear - array with all docs of a year
     * @param year
     * @return QJsonArray with all document objects of a year
     */
    QJsonArray docsPerYear(const QString& year);

    /**
     * @brief addDoc - add a new document to index file
     * @param doc
     * @return true if save succeeded
     */
    bool addDoc(KraftDoc* doc);

    /**
     * @brief updateDoc - update a document in index file
     * @param doc
     * @return true if save succeeded
     */
    bool updateDoc(KraftDoc* doc);

    /**
     * @brief findDocObj - looks up an index entry for uuid and year
     *
     * @param year
     * @param uuid
     * @return a json object with document digest data
     */
    QJsonObject findDocObj(const QString& year, const QString& uuid);

    /**
     * @brief writeFile - persists the json doc to a file on disk
     * @param json
     * @return true if save succeeded
     */
    static bool writeFile(QJsonObject &json);

    const static QString YearsDataStr;
private:
    void load(const QFileInfo& indxFile);

    static QJsonObject _indexJsonObj;
};

#endif // JSONINDEXFILE_H
