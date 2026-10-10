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
#include <QDir>
#include <QElapsedTimer>
#include <QJsonArray>
#include <QSqlError>
#include <QSqlQuery>

#include "xmlindexdb.h"
#include "kraftdoc.h"
#include "documentman.h"
#include "defaultprovider.h"
#include "jsonindexfile.h"
#include "docdigest.h"
#include "format.h"
#include "doctype.h"

const QString XmlIndexDb::ConnectionName = QStringLiteral("kraftXmlIndex");
const QString XmlIndexDb::DbFileName = QStringLiteral("kraftindx.db");
const int XmlIndexDb::SchemaVersion = 1;

QMultiMap<QDate, QString> XmlIndexDb::_dateMap = QMultiMap<QDate, QString>();

namespace {
const QString DocColumns = QStringLiteral("uuid, ident, docType, docDate, subPath, lastModified, clientAddress, prjtLabel, state, whiteboard");

bool execOrWarn(QSqlQuery &q, const QString &sql = QString())
{
    const bool ok = sql.isEmpty() ? q.exec() : q.exec(sql);
    if (!ok) {
        qWarning() << "XmlIndexDb: SQL error" << q.lastError().text() << "in" << q.lastQuery();
    }
    return ok;
}
}

XmlIndexDb::XmlIndexDb()
{
}

QSqlDatabase XmlIndexDb::db() const
{
    // returns an invalid database object if setBasePath was not called before
    return QSqlDatabase::database(ConnectionName, false);
}

void XmlIndexDb::setBasePath(const QString &basePath)
{
    Q_UNUSED(basePath); // the location is taken from the DefaultProvider, as in XmlDocIndex

    const QDir dir{DefaultProvider::self()->kraftV2Dir(DefaultProvider::KraftV2Dir::Root)};
    const QString dbFile = dir.absoluteFilePath(DbFileName);
    const bool isNew = !QFileInfo::exists(dbFile);

    if (!openDb(dbFile)) {
        return;
    }

    if (isNew) {
        QElapsedTimer timer;
        timer.start();
        if (!importJsonIndex()) {
            buildIndexFromXml();
        }
        qDebug() << "Built the index database in" << timer.elapsed() << "msec";
    }
    buildDateMap();
}

bool XmlIndexDb::openDb(const QString &dbFile)
{
    QSqlDatabase database;
    if (QSqlDatabase::contains(ConnectionName)) {
        database = QSqlDatabase::database(ConnectionName, false);
        if (database.databaseName() != dbFile) {
            // the base dir changed, ie. in tests. Reopen with the new file.
            database.close();
            database.setDatabaseName(dbFile);
            _dateMap.clear();
        }
    } else {
        database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), ConnectionName);
        database.setDatabaseName(dbFile);
    }

    if (!database.isOpen() && !database.open()) {
        qWarning() << "XmlIndexDb: Can not open index database" << dbFile << database.lastError().text();
        return false;
    }

    // The index can always be rebuilt from the xml files, so trade durability for speed:
    // WAL with synchronous=NORMAL avoids an fsync for every single index update.
    QSqlQuery q(database);
    execOrWarn(q, QStringLiteral("PRAGMA journal_mode=WAL"));
    execOrWarn(q, QStringLiteral("PRAGMA synchronous=NORMAL"));

    return createSchema();
}

bool XmlIndexDb::createSchema()
{
    QSqlQuery q(db());

    bool ok = execOrWarn(q, QStringLiteral("CREATE TABLE IF NOT EXISTS meta ("
                                           "key   TEXT PRIMARY KEY,"
                                           "value TEXT)"));
    ok = ok && execOrWarn(q, QStringLiteral("CREATE TABLE IF NOT EXISTS docindex ("
                                            "uuid          TEXT PRIMARY KEY,"
                                            "ident         TEXT,"
                                            "docType       TEXT NOT NULL,"
                                            "docDate       TEXT,"
                                            "subPath       TEXT NOT NULL,"
                                            "lastModified  TEXT,"
                                            "clientAddress TEXT,"
                                            "prjtLabel     TEXT,"
                                            "state         TEXT,"
                                            "whiteboard    TEXT)"));
    ok = ok && execOrWarn(q, QStringLiteral("CREATE INDEX IF NOT EXISTS docindex_ident ON docindex(ident)"));
    ok = ok && execOrWarn(q, QStringLiteral("CREATE INDEX IF NOT EXISTS docindex_date ON docindex(docDate)"));

    if (ok) {
        q.prepare(QStringLiteral("INSERT OR IGNORE INTO meta (key, value) VALUES ('schemaVersion', :v)"));
        q.bindValue(":v", QString::number(SchemaVersion));
        ok = execOrWarn(q);
    }
    return ok;
}

const QFileInfo XmlIndexDb::fullPathWithExtension(const QString &subPath, const QString &extension)
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

QString XmlIndexDb::subPathBy(const QString &column, const QString &value)
{
    if (value.isEmpty() || !db().isOpen())
        return QString();

    // column is never user input, only "uuid" or "ident"
    QSqlQuery q(db());
    q.prepare(QStringLiteral("SELECT subPath FROM docindex WHERE %1 = :v").arg(column));
    q.bindValue(":v", value);
    if (execOrWarn(q) && q.next()) {
        return q.value(0).toString();
    }
    return QString();
}

const QFileInfo XmlIndexDb::xmlPathByUuid(const QString &uuid)
{
    return fullPathWithExtension(subPathBy(QStringLiteral("uuid"), uuid), ".xml");
}

const QFileInfo XmlIndexDb::xmlPathByIdent(const QString &ident)
{
    return fullPathWithExtension(subPathBy(QStringLiteral("ident"), ident), ".xml");
}

// so far, for the pdf path, only the extension is changed from xml to pdf.
const QFileInfo XmlIndexDb::pdfPathByUuid(const QString &uuid)
{
    return fullPathWithExtension(subPathBy(QStringLiteral("uuid"), uuid), ".pdf");
}

const QFileInfo XmlIndexDb::pdfPathByIdent(const QString &ident)
{
    return fullPathWithExtension(subPathBy(QStringLiteral("ident"), ident), ".pdf");
}

bool XmlIndexDb::pdfOutdated(const QString &uuid)
{
    if (uuid.isEmpty())
        return false;

    const QFileInfo fiPdf = pdfPathByUuid(uuid);
    const QFileInfo fiXml = xmlPathByUuid(uuid);

    if (!fiPdf.exists())
        return true;

    return fiPdf.lastModified() < fiXml.lastModified();
}

const QMultiMap<QDate, QString> &XmlIndexDb::dateMap()
{
    return _dateMap;
}

void XmlIndexDb::buildDateMap()
{
    _dateMap.clear();
    QSqlQuery q(db());
    q.setForwardOnly(true);
    if (!execOrWarn(q, QStringLiteral("SELECT docDate, subPath FROM docindex"))) {
        return;
    }
    while (q.next()) {
        const QDate d = QDate::fromString(q.value(0).toString(), Qt::ISODate);
        if (d.isValid()) {
            _dateMap.insert(d, q.value(1).toString());
        }
    }
    qDebug() << "Date map contains" << _dateMap.size() << "documents";
}

DocDigest XmlIndexDb::findDigest(const QString &year, const QString &uuid)
{
    Q_UNUSED(year); // the uuid is unique, the year is not needed for the lookup

    // the uuid might be a sub path like 2025/05/<uuid> as stored in the dateMap
    const QString pureUuid = uuid.section('/', -1);

    QSqlQuery q(db());
    q.prepare(QStringLiteral("SELECT %1 FROM docindex WHERE uuid = :uuid").arg(DocColumns));
    q.bindValue(":uuid", pureUuid);
    if (execOrWarn(q) && q.next()) {
        DocTypes docTypes;
        return toDocDigest(q, docTypes);
    }
    qDebug() << "Digest is empty for" << uuid;
    return DocDigest();
}

QList<DocDigest> XmlIndexDb::allDigests()
{
    QList<DocDigest> re;
    if (!db().isOpen())
        return re;

    QSqlQuery q(db());
    q.setForwardOnly(true);
    // DocTypes reads all doctype files from disk on first access, so use one instance for all digests
    DocTypes docTypes;
    if (execOrWarn(q, QStringLiteral("SELECT %1 FROM docindex ORDER BY docDate DESC, lastModified DESC").arg(DocColumns))) {
        while (q.next()) {
            re.append(toDocDigest(q, docTypes));
        }
    }
    return re;
}

DocDigest XmlIndexDb::toDocDigest(const QSqlQuery &q, const DocTypes &docTypes)
{
    DocDigest dd;

    const QString t{q.value("docType").toString()};
    Q_ASSERT(!t.isEmpty());
    dd.setType(docTypes.get(t));
    dd.setClientAddress(q.value("clientAddress").toString());
    dd.setDate(QDate::fromString(q.value("docDate").toString(), Qt::ISODate));
    dd.setIdent(q.value("ident").toString());
    KraftDocState state;
    state.setStateFromString(q.value("state").toString());
    dd.setState(state);
    dd.setUuid(q.value("uuid").toString());
    dd.setProjectLabel(q.value("prjtLabel").toString());
    dd.setLastModified(QDateTime::fromString(q.value("lastModified").toString(), Qt::ISODate));
    dd.setWhiteboard(q.value("whiteboard").toString());

    return dd;
}

// Kept for interface compatibility with XmlDocIndex
DocDigest XmlIndexDb::toDocDigest(QJsonObject obj)
{
    DocDigest dd;

    if (obj.isEmpty()) {
        qDebug() << "Digest is empty";
    } else {
        const QString t{obj["docType"].toString()};
        Q_ASSERT(!t.isEmpty());
        dd.setType(t);
        dd.setClientAddress(obj["clientAddress"].toString());
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

QString XmlIndexDb::subPathForDoc(KraftDoc *doc)
{
    const QDate d = doc->date();
    return QString("%1/%2/%3").arg(d.year(), 4, 10, QLatin1Char('0')).arg(d.month(), 2, 10, QLatin1Char('0')).arg(doc->uuid());
}

bool XmlIndexDb::writeEntry(KraftDoc *doc, const QString &subPath)
{
    QJsonObject obj;
    doc->toJsonObj(obj);

    QSqlQuery q(db());
    q.prepare(QStringLiteral("INSERT OR REPLACE INTO docindex (%1) VALUES "
                             "(:uuid, :ident, :docType, :docDate, :subPath, :lastModified, :clientAddress, :prjtLabel, :state, :whiteboard)")
                  .arg(DocColumns));
    q.bindValue(":uuid", obj["uuid"].toString());
    q.bindValue(":ident", obj["ident"].toString());
    q.bindValue(":docType", obj["docType"].toString());
    q.bindValue(":docDate", obj["date"].toString());
    q.bindValue(":subPath", subPath);
    q.bindValue(":lastModified", obj["lastModified"].toString());
    q.bindValue(":clientAddress", obj["clientAddress"].toString());
    q.bindValue(":prjtLabel", obj["prjtLabel"].toString());
    q.bindValue(":state", obj["state"].toString());
    q.bindValue(":whiteboard", obj["whiteboard"].toString());

    return execOrWarn(q);
}

// Add a new document to the index database.
// The stored path is relative to the xml base dir, without extension because the pdf
// path lookup uses the same index
void XmlIndexDb::addEntry(KraftDoc *doc)
{
    if (!doc)
        return;
    Q_ASSERT(!doc->uuid().isEmpty());

    const QString entry = subPathForDoc(doc);
    if (writeEntry(doc, entry) && doc->date().isValid()) {
        _dateMap.insert(doc->date(), entry);
    }
}

void XmlIndexDb::updateEntry(KraftDoc *doc)
{
    if (!doc)
        return;

    // The sub path is not changed, because the document file remains where it is
    QString entry;
    QDate oldDate;
    QSqlQuery q(db());
    q.prepare(QStringLiteral("SELECT subPath, docDate FROM docindex WHERE uuid = :uuid"));
    q.bindValue(":uuid", doc->uuid());
    if (execOrWarn(q) && q.next()) {
        entry = q.value(0).toString();
        oldDate = QDate::fromString(q.value(1).toString(), Qt::ISODate);
    }
    const bool known = !entry.isEmpty();
    if (!known) {
        entry = subPathForDoc(doc);
    }

    if (writeEntry(doc, entry)) {
        // the date might have changed. Only look at the entries of the old date
        // instead of iterating the whole map, which is slow for bulk updates.
        if (known && oldDate.isValid()) {
            auto it = _dateMap.find(oldDate);
            while (it != _dateMap.end() && it.key() == oldDate) {
                if (it.value() == entry) {
                    _dateMap.erase(it);
                    break;
                }
                ++it;
            }
        }
        if (doc->date().isValid()) {
            _dateMap.insert(doc->date(), entry);
        }
    }
}

// Migration: fills the database from the legacy json index file if it exists.
bool XmlIndexDb::importJsonIndex()
{
    JsonIndexFile jsonIndx;
    if (!jsonIndx.indexFileInfo().exists()) {
        return false;
    }

    QSqlDatabase database = db();
    database.transaction();

    QSqlQuery q(database);
    q.prepare(QStringLiteral("INSERT OR REPLACE INTO docindex (%1) VALUES "
                             "(:uuid, :ident, :docType, :docDate, :subPath, :lastModified, :clientAddress, :prjtLabel, :state, :whiteboard)")
                  .arg(DocColumns));
    int cnt{0};

    const QStringList years = jsonIndx.years();
    for (const QString &year : years) {
        const QJsonArray jsonDocs = jsonIndx.docsPerYear(year);

        for (const QJsonValue &val : jsonDocs) {
            const QJsonObject obj = val.toObject();
            const QString uuid = obj["uuid"].toString();
            const QString dateStr = obj["date"].toString();
            if (uuid.isEmpty() || dateStr.size() < 7) {
                qDebug() << "Skipping incomplete index entry" << obj;
                continue;
            }
            const QString subPath = QString("%1/%2/%3").arg(year, dateStr.mid(5, 2), uuid);

            q.bindValue(":uuid", uuid);
            q.bindValue(":ident", obj["ident"].toString());
            q.bindValue(":docType", obj["docType"].toString());
            q.bindValue(":docDate", dateStr);
            q.bindValue(":subPath", subPath);
            q.bindValue(":lastModified", obj["lastModified"].toString());
            q.bindValue(":clientAddress", obj["clientAddress"].toString());
            q.bindValue(":prjtLabel", obj["prjtLabel"].toString());
            q.bindValue(":state", obj["state"].toString());
            q.bindValue(":whiteboard", obj["whiteboard"].toString());
            if (execOrWarn(q)) {
                cnt++;
            }
        }
    }
    database.commit();
    qDebug() << "Imported" << cnt << "documents from json index file";
    return true;
}

// Scans all xml documents and adds them to the index database
void XmlIndexDb::buildIndexFromXml()
{
    const QString dir{DefaultProvider::self()->kraftV2Dir(DefaultProvider::KraftV2Dir::XmlDocs)};

    QFileInfo fi(dir);
    if (!(fi.exists() && fi.isDir())) {
        qDebug() << "Base path is not valid:" << dir;
        return;
    }

    QSqlDatabase database = db();
    database.transaction();

    int cnt{0};
    const QDir::Filters filter{QDir::Dirs | QDir::NoDotAndDotDot};
    const QFileInfoList years = QDir(dir).entryInfoList(filter, QDir::Name);

    for (const QFileInfo &yearFi : years) {
        const QFileInfoList monthEntries = QDir(yearFi.absoluteFilePath()).entryInfoList(filter, QDir::Name);

        for (const QFileInfo &mFi : monthEntries) {
            const QFileInfoList docEntries = QDir(mFi.absoluteFilePath()).entryInfoList({"*.xml"}, QDir::Files, QDir::NoSort);

            for (const QFileInfo &docFi : docEntries) {
                KraftDoc doc;
                if (DocumentMan::self()->loadMetaFromFilename(docFi.absoluteFilePath(), &doc)) {
                    // use the real location of the file as sub path
                    const QString subPath = QString("%1/%2/%3").arg(yearFi.fileName(), mFi.fileName(), docFi.completeBaseName());
                    if (writeEntry(&doc, subPath)) {
                        cnt++;
                    }
                } else {
                    qDebug() << "Unable to load meta data from file for" << docFi.absoluteFilePath();
                }
            }
        }
    }
    database.commit();
    qDebug() << "Indexed" << cnt << "xml files";
}
