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
#include <QDirIterator>
#include <QtConcurrent/QtConcurrentRun>
#include <QElapsedTimer>

#include "xmldocindex.h"
#include "kraftdoc.h"
#include "documentman.h"
#include "defaultprovider.h"
#include "jsonindexfile.h"
#include "docdigest.h"

QMap<QString, QString> XmlDocIndex::_identMap = QMap<QString, QString>();
QMap<QString, QString> XmlDocIndex::_uuidMap = QMap<QString, QString>();
QMultiMap<QDate, QString> XmlDocIndex::_dateMap = QMultiMap<QDate, QString>();

XmlDocIndex::XmlDocIndex()
{
}

void XmlDocIndex::setBasePath(const QString& basePath)
{
    Q_UNUSED(basePath); // FIXME: Remove

    // check for the index file - if it does not exist, create it
    JsonIndexFile jsonIndx;
    QFileInfo fi = jsonIndx.indexFileInfo();
    if (!fi.exists()) {
        QElapsedTimer timer;
        timer.start();
        buildIndexFile();
        fi.refresh();
    }
    if (fi.exists()) {
        buildIndexFromFile();
    }
}

const QFileInfo XmlDocIndex::fullPathWithExtension(const QString& subPath, const QString& extension)
{
    QFileInfo fi;
    if (subPath.isEmpty())
        return fi;

    QString re{subPath};
    if (!extension.isEmpty()) {
        if (!extension.startsWith('.')) {
            re.append('.');
        }
        re.append(extension);
    }
    const auto pathSelect{extension.endsWith("pdf", Qt::CaseInsensitive) ? DefaultProvider::KraftV2Dir::PdfDocs : DefaultProvider::KraftV2Dir::XmlDocs};
    const QString dir{DefaultProvider::self()->kraftV2Dir(pathSelect)};

    fi.setFile(QDir(dir), re);
    return fi;

}

const QFileInfo XmlDocIndex::xmlPathByUuid(const QString& uuid)
{
    if (!uuid.isEmpty() && _uuidMap.contains(uuid)) {
        const QString sub{_uuidMap[uuid]};
        return fullPathWithExtension(sub, ".xml");
    }
    return QFileInfo();
}

const QFileInfo XmlDocIndex::xmlPathByIdent(const QString &ident)
{
    QFileInfo re;

    if (!ident.isEmpty() && _identMap.contains(ident)) {
        const QString sub{_identMap[ident]};
        return fullPathWithExtension(sub, ".xml");
    }
    return re;
}

// so far, for the pdf path, only the extension is changed from xml to pdf.
const QFileInfo XmlDocIndex::pdfPathByUuid(const QString& uuid)
{
    if (!uuid.isEmpty() && _uuidMap.contains(uuid)) {
        const QString sub{_uuidMap[uuid]};
        return fullPathWithExtension(sub, ".pdf");
    }
    return QFileInfo();
}

// so far, for the pdf path, only the extension is changed from xml to pdf.
const QFileInfo XmlDocIndex::pdfPathByIdent(const QString& ident)
{
    QFileInfo re;

    if (!ident.isEmpty() && _identMap.contains(ident)) {
        const QString sub{_identMap[ident]};
        return fullPathWithExtension(sub, ".pdf");
    }
    return re;
}

bool XmlDocIndex::pdfOutdated(const QString& uuid)
{
    if (uuid.isEmpty()) return false;

    const QFileInfo fiPdf = pdfPathByUuid(uuid);
    const QFileInfo fiXml = xmlPathByUuid(uuid);

    if (!fiPdf.exists())
        return true;

    return fiPdf.lastModified() < fiXml.lastModified();
}

const QMultiMap<QDate, QString> &XmlDocIndex::dateMap()
{
    return _dateMap;
}

DocDigest XmlDocIndex::findDigest(const QString& year, const QString& uuid)
{
    DocDigest dd;

    JsonIndexFile jsonIndx;

    QJsonObject obj = jsonIndx.findDocObj(year, uuid);

    if (obj.isEmpty()) {
        qDebug() << "Digest is empty";
    } else {
        dd.setType(obj["docType"].toString());
        // dd.setAddressee()
        dd.setClientAddress(obj["clientAddress"].toString());
        // dd.setClientId()
        dd.setDate(QDate::fromString(obj["date"].toString(), Qt::ISODate));
        dd.setIdent(obj["ident"].toString());
        KraftDocState state;
        state.setStateFromString(obj["state"].toString());
        dd.setState(state);
        dd.setUuid(obj["uuid"].toString());
        dd.setProjectLabel(obj["prjtLabel"].toString());
        dd.setLastModified(QDateTime::fromString(obj["lastModified"].toString(), Qt::ISODate));
        dd.setWhiteboard(obj["whiteboard"].toString());
    }
    return dd;
}


bool XmlDocIndex::buildIndexFromFile()
{
    JsonIndexFile jsonIndx;

    const QStringList years = jsonIndx.years();
    int overall{0};
    QElapsedTimer timer;
    timer.start();
    for(const QString& year : years) {
        QJsonArray jsonDocs = jsonIndx.docsPerYear(year);

        for (int docIndx = 0; docIndx < jsonDocs.size(); ++docIndx) {
            const QJsonObject docObject = jsonDocs[docIndx].toObject();
            QString path{year};
            path.append("/");
            QString ident, dateStr;

            int components{0};
            if (docObject.contains("ident") && docObject["ident"].isString()) {
                ident = docObject["ident"].toString();
                components++;
            }

            if (docObject.contains("date") && docObject["date"].isString()) {
                dateStr = docObject["date"].toString();
                // Remove zeros in case there are any
                QString m = dateStr.mid(5,2);
                path.append(m);
                path.append("/");
                components++;
            }

            if (docObject.contains("uuid") && docObject["uuid"].isString()) {
                const QString uuid = docObject["uuid"].toString();
                path.append(uuid);
                components++;

                if (components == 3) {
                    // all components were found
                    // qDebug() << "INSERT path:" << path;
                    _identMap.insert(ident, path);
                    QDate d = QDate::fromString(dateStr, Qt::ISODate);
                    _dateMap.insert(d, path);
                    _uuidMap.insert(uuid, path);
                }
            }
            overall++;
        }

    }
    qDebug() << "Indexed"<< overall << "documents from indx file in" << timer.elapsed() << "msec";
    return true;
}

void XmlDocIndex::buildIndexFile()
{
    const QString dir{DefaultProvider::self()->kraftV2Dir(DefaultProvider::KraftV2Dir::XmlDocs)};

    QFileInfo fi(dir);
    if (!(fi.exists() && fi.isDir())) {
        qDebug() << "Base path is not valid:" << dir;
        return;
    }

    int cnt{0};
    KraftDoc doc;

    const QDir::Filters filter{QDir::Dirs|QDir::NoDotAndDotDot};
    QDir d(dir);
    const QFileInfoList years = d.entryInfoList(filter, QDir::Name);

    QJsonObject jsonDoc; // digest of all docs.
    QJsonObject yearsMap;

    for (const QFileInfo& yearFi : years) {
        QJsonArray docArr;

        QDir dMonth{yearFi.absoluteFilePath()};
        const QFileInfoList monthEntries = dMonth.entryInfoList(filter, QDir::Name);

        for (const QFileInfo& mFi : monthEntries) {
            QDir dDoc{mFi.absoluteFilePath()};

            const QFileInfoList docEntries = dDoc.entryInfoList(QDir::Files, QDir::NoSort);

            for (const QFileInfo& docFi : docEntries) {
                if (DocumentMan::self()->loadMetaFromFilename(docFi.absoluteFilePath(), &doc)) {
                   QJsonObject obj;
                   doc.toJsonObj(obj);
                   docArr.append(obj);
                   cnt++;
                } else {
                    qDebug() << "Unable to load meta data from file for" << docFi.absoluteFilePath();
                }

            }
        }
        yearsMap[yearFi.fileName()] = docArr;
    }
    jsonDoc[JsonIndexFile::YearsDataStr] = yearsMap;
    JsonIndexFile::writeFile(jsonDoc);

    qDebug() << "Indexed"<< cnt << "files";
}

// Add a new document to the document index which lives in memory.
// This function also updates the index file on disk
// the stored path is relative to the xml base dir, without extension because the pdf
// has lookup uses the same index
void XmlDocIndex::addEntry(KraftDoc *doc)
{
    if (!doc) return;
    // cut the extension .xml
    const QDate d = doc->date();
    const QString entry = QString("%1/%2/%3").arg(d.year(), 4, 10, QLatin1Char('0'))
            .arg(d.month(), 2, 10, QLatin1Char('0'))
            .arg(doc->uuid());

    Q_ASSERT(!entry.isEmpty());

    if (!doc->ident().isEmpty())
        _identMap.insert(doc->ident(), entry);
    if (doc->date().isValid())
        _dateMap.insert(doc->date(), entry);
    Q_ASSERT(!doc->uuid().isEmpty());
    _uuidMap.insert(doc->uuid(), entry);

    // put it in the json indexfile as well
    JsonIndexFile indexFile;
    indexFile.addDoc(doc);

}

void XmlDocIndex::updateEntry(KraftDoc *doc)
{
    // The maps do not have to be changed, because they return the path to the doc file only
    JsonIndexFile indexFile;
    indexFile.updateDoc(doc);
}
