/***************************************************************************
                 lister.h  - template class for xml based items
                             -------------------
    begin                : Feb 2026
    copyright            : (C) 2026 by Klaas Freitag
    email                : opensource@freisturz.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef XMLDIRLISTER_H
#define XMLDIRLISTER_H

#include "defaultprovider.h"

#include <QSaveFile>

// =============================================================================
template <class T>
class XmlDirLister {
public:
    enum class SaveResult {
        SaveOk,
        OpenFail,
        NameFail,
        Locked,
        PartialFail,
        RemoveFail
    };

    XmlDirLister(DefaultProvider::KraftV2Dir dir)
        :_v2dir{dir}
    {

    }

    const QMap<QString, T> map() { return _map; }

    const T get(const QString& name) const {
        return _map.contains(name)?_map.value(name):T();
    }

    int loadAll(const QString& baseDir = QString()) {
        QString v2Dir{baseDir};
        if (baseDir.isEmpty()) {
            v2Dir = DefaultProvider::self()->kraftV2Dir();
        }

        QDir wdir(v2Dir);
        wdir.cd(DefaultProvider::self()->kraftV2Subdir(_v2dir));

        const QStringList names {"*.xml"};
        const QStringList entries = wdir.entryList(names);

        for (const QString& xmlFileName : entries) {
            QString const fullPathName = wdir.absoluteFilePath(xmlFileName);
            QFile file(fullPathName);
            if (!file.open(QIODevice::ReadOnly)) {
                qDebug() << "Unable to open xml document file:" << xmlFileName ;
                continue;
            }

            QDomDocument domDoc;
            const QByteArray arr = file.readAll();
            QDomDocument::ParseResult pr = domDoc.setContent(arr);
            file.close();

            if (!pr) {
                qDebug() << "Unable to set file content as xml:" << pr.errorMessage << "at line" <<pr.errorLine;
                continue;
            }

            // ---- Parsing starts here
            T obj;
            obj.parseXml(domDoc);
            _map.insert(obj.name(), obj);
        }
        return _map.count();
    }

    SaveResult save(const T& t, const QString& baseDir = QString()) {
        if (t.name().isEmpty()) {
            qDebug() << "Can not save without name";
            return SaveResult::OpenFail;
        }

        const QString xml = t.toXml();

        QString saveName{t.name()};
        QString v2Dir{baseDir};
        if (baseDir.isEmpty()) {
            v2Dir = DefaultProvider::self()->kraftV2Dir();
        }

        QDir wdir(v2Dir);
        wdir.cd(DefaultProvider::self()->kraftV2Subdir(_v2dir));

        bool re{false};
        if (!saveName.endsWith(".xml")) saveName.append(".xml");
        QSaveFile file(wdir.absoluteFilePath(saveName));
        if ( file.open( QIODevice::WriteOnly | QIODevice::Text) ) {
            re = file.write(xml.toUtf8());

            if (re) {
                re = file.commit();
            }
        }

        if (re) {
            _map.insert(t.name(), t);
            return SaveResult::SaveOk;
        }

        return SaveResult::OpenFail;
    }

    SaveResult saveAll(QMap<QString, T> map, const QString& baseDir = QString()) {
        const QList<T> list = map.values();
        SaveResult res{SaveResult::SaveOk};
        for( T t : list) {
            auto lres = save(t, baseDir);
            if (res == SaveResult::SaveOk) {
                res = lres; // do not overwrite an error
            }
        }
        _map = map;
        return res;
    }

    SaveResult remove(const T &t, const QString& baseDir = QString()) {
        const QString &name{t.name()};
        QString saveName{name};
        QString v2Dir{baseDir};
        if (baseDir.isEmpty()) {
            v2Dir = DefaultProvider::self()->kraftV2Dir();
        }

        QDir dir(v2Dir);
        dir.cd(DefaultProvider::self()->kraftV2Subdir(_v2dir));

        SaveResult re{SaveResult::RemoveFail};
        if (!saveName.endsWith(".xml")) saveName.append(".xml");

        const QString file(dir.absoluteFilePath(saveName));
        if (QFile::remove(file)) {
            re = SaveResult::SaveOk;
            _map.remove(name);
        }
        return re;
    }

private:
    static QMap<QString, T> _map;
    DefaultProvider::KraftV2Dir _v2dir;
};

template<typename T>
QMap<QString, T> XmlDirLister<T>::_map = {};


#endif // LISTER_H
