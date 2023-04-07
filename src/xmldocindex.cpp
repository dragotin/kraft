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

#include "xmldocindex.h"
#include "kraftdoc.h"
#include "documentman.h"

QMap<QString, QString> XmlDocIndex::_identMap = QMap<QString, QString>();

XmlDocIndex::XmlDocIndex( const QString& basePath)
    : _basePath(basePath)
{
    if (_identMap.count() == 0) {
        buildIndex();
    }
}

const QString XmlDocIndex::pathByIdent(const QString &ident)
{
    QString re;
    if (!ident.isEmpty() && _identMap.contains(ident)) {
        re = _identMap[ident];
    }
    return re;
}

const QMap<QDate, QString> &XmlDocIndex::dateMap()
{
    return _dateMap;
}

void XmlDocIndex::buildIndex()
{
    qDebug() << "Building index for" << _basePath;
    QFileInfo fi(_basePath);
    if (!(fi.exists() && fi.isDir())) {
        qDebug() << "Base path is not valid:" << _basePath;
        return;
    }

    int cnt{0};
    QDirIterator dirIt(_basePath, {"*.xml"}, QDir::Files, QDirIterator::Subdirectories);
    while( dirIt.hasNext() ) {
        const QString xmlFile = dirIt.next();
        // qDebug() << "Indexing" << xmlFile;
        cnt++;
        KraftDoc *doc = DocumentMan::self()->loadMetaFromFilename(xmlFile);
        if (doc) {
            _identMap.insert(doc->ident(), xmlFile);
            _dateMap.insert(doc->date(), xmlFile);
            delete doc;
        }
    }
    qDebug() << "Indexed"<< cnt << "files";
}
