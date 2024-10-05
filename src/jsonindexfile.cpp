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

#include <QStringList>

#include "jsonindexfile.h"
#include "defaultprovider.h"

const QString JsonIndexFile::YearsDataStr = QStringLiteral("yearsData");
const QString IndexFileName = QStringLiteral("kraftindx.json");

QJsonObject JsonIndexFile::_indexJsonObj = QJsonObject();

JsonIndexFile::JsonIndexFile()
{
    load(indexFileInfo());
}

QFileInfo JsonIndexFile::indexFileInfo()
{
    const QDir dir{DefaultProvider::self()->kraftV2Dir(DefaultProvider::KraftV2Dir::Root)};
    return QFileInfo(dir.absoluteFilePath(IndexFileName));
}

void JsonIndexFile::load(const QFileInfo& indxFile)
{
    if (!indxFile.exists()) {
        qDebug() << "Can not load index file, Index File not existing" << indxFile.filePath();
        return;
    }

    // check if already read. The Json object is maintained in memory
    if (!_indexJsonObj.isEmpty()) {
        return;
    }
    qDebug() << "Building internal index from indx file" << indxFile.absolutePath();

    QFile loadFile(indxFile.absoluteFilePath());
    if (loadFile.open(QIODevice::ReadOnly)) {
        const QByteArray loadedData = loadFile.readAll();

        QJsonDocument loadDoc(QJsonDocument::fromJson(loadedData));

        _indexJsonObj = loadDoc.object();
    }
}

QStringList JsonIndexFile::years()
{
    const QJsonObject jsonYears = _indexJsonObj[YearsDataStr].toObject();
    const QStringList years = jsonYears.keys();

    return years;
}

QJsonArray JsonIndexFile::docsPerYear(const QString& year)
{
    QJsonArray reArr;

    if (years().indexOf(year) > -1) {
        reArr = _indexJsonObj[YearsDataStr].toObject()[year].toArray();
    }
    return reArr;
}

QJsonObject JsonIndexFile::findDocObj(const QString& year, const QString& uuid)
{
    QJsonObject re;

    if (!year.isEmpty()) {
        const QJsonArray &arr = docsPerYear(year);

        // qDebug() << "Year" << year << "contains amount of entries" << arr.size();

        for (const QJsonValue& jval : arr) {
            const QJsonObject &obj = jval.toObject();
            const QString& uu = obj["uuid"].toString();
            if (uuid.endsWith(uu)) {
                re = obj;
                break;
            }
        }
    }
    return re;
}


bool JsonIndexFile::updateDoc(KraftDoc* doc)
{
    const QDate d = doc->date();
    const QString yearStr = QString::number(d.year());
    QJsonArray yearArr = docsPerYear(yearStr);

    const QString uuid = doc->uuid();

    // remove the doc index for the doc to update
    for (auto it = yearArr.begin(); it != yearArr.end(); ++it) {
        QJsonObject obj = (*it).toObject();
        if (obj.value("uuid").toString() == uuid) {
            yearArr.erase(it);
            break;
        }
    }

    // ...and write back to the index object
    QJsonObject yearsDataObj = _indexJsonObj[YearsDataStr].toObject();
    yearsDataObj[yearStr] = yearArr;

    _indexJsonObj[YearsDataStr] = yearsDataObj;

    // add the updated one
    return addDoc(doc);
}

bool JsonIndexFile::addDoc(KraftDoc* doc)
{
    const QDate d = doc->date();
    const QString yearStr = QString::number(d.year());

    // get the current list of docs of the year
    QJsonArray arr = docsPerYear(yearStr);
    QJsonObject obj;
    doc->toJsonObj(obj);

    // and append the new obj
    arr.push_back(obj);

    QJsonObject yearsDataObj = _indexJsonObj[YearsDataStr].toObject();
    yearsDataObj[yearStr] = arr;

    _indexJsonObj[YearsDataStr] = yearsDataObj;

    return writeFile(_indexJsonObj);
}

bool JsonIndexFile::writeFile(QJsonObject &json)
{
    QDir dir(DefaultProvider::self()->kraftV2Dir(DefaultProvider::KraftV2Dir::Root));
    const QString& path = dir.absoluteFilePath(IndexFileName);
    qDebug() << "Writing index file to" << path;

    QFile saveFile(path);

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }

    saveFile.write(QJsonDocument(json).toJson());
    saveFile.close();

    return true;
}
