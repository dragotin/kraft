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

QMap<QString, QString> XmlDocIndex::_identMap = QMap<QString, QString>();
QMap<QString, QString> XmlDocIndex::_uuidMap = QMap<QString, QString>();
QMultiMap<QDate, QString> XmlDocIndex::_dateMap = QMap<QDate, QString>();
QString XmlDocIndex::_basePath = QString();
QFuture<void> XmlDocIndex::_future;

XmlDocIndex::XmlDocIndex()
{
    if (_future.isRunning()) {
        qDebug() << "===== waiting to finish indexing";
        _future.waitForFinished();
    }
}

void XmlDocIndex::setBasePath(const QString& basePath)
{
    _basePath = basePath;

    if (buildIndexFromFile()) {
        return;
    }

    if (_identMap.count() == 0) {
        QElapsedTimer timer;
        timer.start();
        _future = QtConcurrent::run([=]() {
            // Code in this block will run in another thread
            buildIndex();
        });
    }
}

const QFileInfo XmlDocIndex::xmlPathByIdent(const QString &ident)
{
    QFileInfo re;
    if (!ident.isEmpty() && _identMap.contains(ident)) {
        re.setFile(QDir(_basePath), _identMap[ident] + ".xml");
    }
    return re;
}

const QFileInfo XmlDocIndex::pathByUuid(const QString& uuid, const QString& extension)
{
    QString re;
    if (!uuid.isEmpty() && _uuidMap.contains(uuid)) {
        re = _uuidMap[uuid];
        if (!extension.isEmpty()) {
            if (extension.startsWith('.'))
                re += extension;
            else
                re = re + '.' + extension;
        }
    }
    QFileInfo fi;
    if (!re.isEmpty())
        fi.setFile(QDir(_basePath), re);
    return fi;
}

const QFileInfo XmlDocIndex::xmlPathByUuid(const QString& uuid)
{
    return pathByUuid(uuid, ".xml");
}

// so far, for the pdf path, only the extension is changed from xml to pdf.
const QFileInfo XmlDocIndex::pdfPathByUuid(const QString& uuid)
{
    if (uuid.isEmpty()) {
        return QFileInfo();
    }
    const QString dir{DefaultProvider::self()->kraftV2Dir(DefaultProvider::KraftV2Dir::PdfDocs)};

    return QFileInfo(dir, uuid+".pdf");
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

bool XmlDocIndex::buildIndexFromFile()
{
    const QFileInfo base{DefaultProvider::self()->kraftV2Dir(DefaultProvider::KraftV2Dir::Root)};
    qDebug() << "Building internal index for path" << base.absolutePath();
    Q_ASSERT(base.exists());

    if (!(base.exists() && base.isDir())) {
        qDebug() << "Base path is not valid";
        return false;
    }
    const QDir dir(base.filePath());

    QFile loadFile(dir.absoluteFilePath("kraftindx.json"));

    if (!loadFile.open(QIODevice::ReadOnly)) {
            qWarning("Couldn't open file to read.");
            return false;
        }

    QByteArray saveData = loadFile.readAll();

    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));

    const QJsonObject obj = loadDoc.object();
    const QJsonObject jsonYears = obj["yearsData"].toObject();
    const QStringList years = jsonYears.keys();

    int overall{0};
    QElapsedTimer timer;
    timer.start();
    for(const QString& year : years) {
        QJsonArray jsonDocs = obj["yearsData"][year].toArray();

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
                if (m.startsWith("0")) m.remove(QChar('0'));
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
                    _dateMap.insert(QDate::fromString(dateStr), path);
                    _uuidMap.insert(uuid, path);
                }
            }
            overall++;
        }

    }
    qDebug() << "Indexed"<< overall << "documents from indx file in" << timer.elapsed() << "msec";

    // read(loadDoc.object());
    return true;
}

void XmlDocIndex::buildIndex()
{
    qDebug() << "Building index for" << _basePath;
    Q_ASSERT(!_basePath.isEmpty());

    QFileInfo fi(_basePath);
    if (!(fi.exists() && fi.isDir())) {
        qDebug() << "Base path is not valid:" << _basePath;
        return;
    }

    int cnt{0};
    QDirIterator dirIt(_basePath, {"*.xml"}, QDir::Files, QDirIterator::Subdirectories);
    KraftDoc doc;
    while( dirIt.hasNext() ) {
        const QString xmlFile = dirIt.next();
        // qDebug() << "Indexing" << xmlFile;
        cnt++;
        if (DocumentMan::self()->loadMetaFromFilename(xmlFile, &doc)) {
            const QString mapFragment = xmlFile.left(xmlFile.length()-4);
            if (!doc.ident().isEmpty())
                _identMap.insert(doc.ident(), mapFragment);
            if (doc.date().isValid())
                _dateMap.insert(doc.date(), mapFragment);
            Q_ASSERT(!doc.uuid().isEmpty());
            _uuidMap.insert(doc.uuid(), mapFragment);
            doc.clear();
        }
    }
    qDebug() << "Indexed"<< cnt << "files";
}

void XmlDocIndex::addEntry(KraftDoc *doc, const QString& xmlFile)
{
    if (!doc) return;
    const QString mapFragment = xmlFile.left(xmlFile.length()-4);

    if (!doc->ident().isEmpty())
        _identMap.insert(doc->ident(), mapFragment);
    if (doc->date().isValid())
        _dateMap.insert(doc->date(), mapFragment);
    Q_ASSERT(!doc->uuid().isEmpty());
    _uuidMap.insert(doc->uuid(), mapFragment);
}
