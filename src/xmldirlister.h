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
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QSaveFile>


template<typename T>
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

    using Map = QMap<QString, T>;

    XmlDirLister() = default;

    T get(const QString& key) const {
        ensureLoaded();
        auto it = _map.find(key);
        if (it != _map.end()) {
            return it.value();
        }
        return {};
    }

    void insert(QString key, T value) {
        ensureLoaded();
        _map[std::move(key)] = std::move(value);
    }

    void remove(const QString& key) {
        ensureLoaded();
        _map.remove(key);
    }

    Map map() const {
        ensureLoaded();
        return _map;
    }

    SaveResult save(const T& t, const QString& baseDir = QString()) {
        if (t.name().isEmpty()) {
            qDebug() << "Can not save without name";
            return SaveResult::NameFail;
        }

        const QString xml = t.toXml();
        QString saveName{t.name()};
        if (!saveName.endsWith(".xml")) {
            saveName.append(".xml");
        }
        const QDir wdir = v2Path(baseDir);

        bool re = false;
        QSaveFile file(wdir.absoluteFilePath(saveName));
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            re = file.write(xml.toUtf8());
            if (re) {
                re = file.commit();
            }
        }

        if (re) {
            ensureLoaded();
            _map.insert(t.name(), t);
            return SaveResult::SaveOk;
        }
        return SaveResult::OpenFail;
    }

    SaveResult saveAll(Map newMap, const QString& baseDir = QString()) {
        ensureLoaded();
        SaveResult res{SaveResult::SaveOk};

        // Delete files for items no longer present in the new map
        for (auto it = _map.cbegin(); it != _map.cend(); ++it) {
            if (!newMap.contains(it.key())) {
                QString fname = it.key();
                if (!fname.endsWith(".xml")) {
                    fname.append(".xml");
                }
                if (!QFile::remove(v2Path(baseDir).absoluteFilePath(fname))) {
                    if (res == SaveResult::SaveOk) {
                        res = SaveResult::RemoveFail;
                    }
                }
            }
        }

        // Write modified items to disk
        for (const T& t : std::as_const(newMap)) {
            if (t.modified()) {
                const SaveResult lres = save(t, baseDir);
                if (res == SaveResult::SaveOk) {
                    res = lres;
                }
            }
        }

        _map = std::move(newMap);
        return res;
    }

protected:
    static DefaultProvider::KraftV2Dir v2SubDir(); // must be specialized per T

private:
    static QDir v2Path(const QString& baseDir = QString()) {
        QString v2Dir{baseDir};
        if (baseDir.isEmpty()) {
            v2Dir = DefaultProvider::self()->kraftV2Dir();
        }
        QDir wdir(v2Dir);
        wdir.cd(DefaultProvider::self()->kraftV2Subdir(v2SubDir()));
        return wdir;
    }

    static Map load() {
        const QDir wdir = v2Path();
        const QStringList entries = wdir.entryList({"*.xml"});
        Map map;
        for (const QString& xmlFileName : entries) {
            QFile file(wdir.absoluteFilePath(xmlFileName));
            if (!file.open(QIODevice::ReadOnly)) {
                qDebug() << "Unable to open xml document file:" << xmlFileName;
                continue;
            }
            QDomDocument domDoc;
            const QByteArray arr = file.readAll();
            const QDomDocument::ParseResult pr = domDoc.setContent(arr);
            file.close();
            if (!pr) {
                qDebug() << "Unable to set file content as xml:" << pr.errorMessage << "at line" << pr.errorLine;
                continue;
            }
            T obj;
            obj.parseXml(domDoc);
            if (obj.isEmpty()) {
                qDebug() << "Can not parse XML file for numbercycle" << file.fileName();
                continue;
            }
            map.insert(obj.name(), obj);
        }
        return map;
    }

    void ensureLoaded() const {
        if (!_loaded) {
            _map = load();
            _loaded = true;
        }
    }

    mutable Map  _map;
    mutable bool _loaded{false};
};


#endif // LISTER_H
