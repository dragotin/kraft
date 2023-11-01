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

    if (_identMap.count() == 0) {
        QElapsedTimer timer;
        timer.start();
        _future = QtConcurrent::run([=]() {
            // Code in this block will run in another thread
            buildIndex();
        });

        // qDebug() << "Indexing took" << timer.elapsed() << "msec";
    }
}

const QFileInfo XmlDocIndex::xmlPathByIdent(const QString &ident)
{
    QFileInfo re;
    if (!ident.isEmpty() && _identMap.contains(ident)) {
        re = QFileInfo{_identMap[ident] + ".xml"};
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
    return QFileInfo{re};
}

const QFileInfo XmlDocIndex::xmlPathByUuid(const QString& uuid)
{
    return pathByUuid(uuid, ".xml");
}

// so far, for the pdf path, only the extension is changed from xml to pdf.
const QFileInfo XmlDocIndex::pdfPathByUuid(const QString& uuid)
{
    return pathByUuid(uuid, ".pdf");
}

const QMultiMap<QDate, QString> &XmlDocIndex::dateMap()
{
    return _dateMap;
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
