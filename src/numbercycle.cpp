/***************************************************************************
                   numbercycle.h  - document number cycles
                             -------------------
    begin                : Jan 15 2009
    copyright            : (C) 2009 by Klaas Freitag
    email                : freitag@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QMap>
#include <QDate>
#include <QDebug>
#include <QDir>
#include <QString>
#include <QDateTime>
#include <QThread>
#include <QSaveFile>

#include "numbercycle.h"
#include "stringutil.h"
#include "defaultprovider.h"

namespace {

/**
 * @brief DocType::generateDocumentIdent
 * @param docDate
 * @param addressUid
 * @param id: Current Id to be used.
 * @param dayCnt: Current day counter to be used.
 * @return
 */
QString generateDocumentIdent(const QString& templ,
                              const QString& doctype,
                              const QDate& docDate,
                              const QString& addressUid,
                              int id, int dayCnt)
{

    /*
   * The pattern may contain the following tags:
   * %y - the year of the documents date.
   * %w - the week number of the documents date
   * %d - the day number of the documents date
   * %m - the month number of the documents date
   * %M - the month number of the documents date
   * %c - the customer id from kaddressbook
   * %i - the uniq identifier from db.
   * %n - the uniq identifier that resets every day and starts from 1
   * %type - the localised doc type (offer, invoice etc.)
   * %uid  - the customer uid
   */

    // Load the template and check if there is a uniq id included.
    QString pattern{templ};
    if ( pattern.indexOf( "%i" ) == -1 && pattern.indexOf("%n") == -1) {
        qWarning() << "No %i found in identTemplate, appending it to meet law needs!";
        if (!pattern.endsWith('-'))
            pattern += QStringLiteral("-");
        pattern += QStringLiteral("%i");
    }

    QMap<QString, QString> m;

    m[ "%yyyy" ] = docDate.toString( "yyyy" );
    m[ "%yy" ] = docDate.toString( "yy" );
    m[ "%y" ] = docDate.toString( "yyyy" );

    QString h;
    h = QString("%1").arg( docDate.weekNumber(), 2, 10, QChar('0') );
    m[ "%ww" ] = h;
    m[ "%w" ] = QString::number( docDate.weekNumber( ) );

    m[ "%dd" ] = docDate.toString( "dd" );
    m[ "%d" ] = docDate.toString( "d" );

    m[ "%m" ] = QString::number( docDate.month() );

    m[ "%MM" ] = docDate.toString( "MM" );
    m[ "%M" ] = docDate.toString( "M" );

    h = QString("%1").arg(id, 6, 10, QChar('0') );
    m[ "%iiiiii" ] = h;

    h = QString("%1").arg(id, 5, 10, QChar('0') );
    m[ "%iiiii" ] = h;

    h = QString("%1").arg(id, 4, 10, QChar('0') );
    m[ "%iiii" ] = h;

    h = QString("%1").arg(id, 3, 10, QChar('0') );
    m[ "%iii" ] = h;

    h = QString("%1").arg(id, 2, 10, QChar('0') );
    m[ "%ii" ] = h;

    m[ "%i" ] = QString::number( id );

    h = QString("%1").arg(dayCnt, 6, 10, QChar('0') );
    m[ "%nnnnnn" ] = h;

    h = QString("%1").arg(dayCnt, 5, 10, QChar('0') );
    m[ "%nnnnn" ] = h;

    h = QString("%1").arg(dayCnt, 4, 10, QChar('0') );
    m[ "%nnnn" ] = h;

    h = QString("%1").arg(dayCnt, 3, 10, QChar('0') );
    m[ "%nnn" ] = h;

    h = QString("%1").arg(dayCnt, 2, 10, QChar('0') );
    m[ "%nn" ] = h;

    m[ "%n" ] = QString::number(dayCnt);

    m[ "%c" ] = addressUid;
    m[ "%type" ] = doctype;
    m[ "%uid" ] = addressUid;

    QString re = KraftString::replaceTags( pattern, m );
    // qDebug () << "Generated document ident: " << re;

    return re;
}


}

// =====================================================================================================

NumberCycle::NumberCycle()
    :_template{"%y%w-%i"}, // default
      _counter{0},
      _dbId{0}
{

}

bool NumberCycle::operator==(const NumberCycle& other) const
{
    return _name == other._name &&
            _template == other._template &&
            _counter == other._counter &&
            _dbId == other._dbId;
}

void NumberCycle::setName( const QString& n )
{
    _name = n;
}

QString NumberCycle::name() const
{
    return _name;
}

void NumberCycle::setTemplate( const QString& t )
{
    _template = t;
}

QString NumberCycle::getTemplate() const
{
    return _template;
}

void NumberCycle::setCounter( int c )
{
    _counter = c;
}

int  NumberCycle::counter() const
{
    return _counter;
}

QString NumberCycle::defaultName()
{
    return QStringLiteral( "default" );
}

// ====================================================================================

NumberCycles::NumberCycles()
{

}

NumberCycle NumberCycles::get(const QString& name)
{
    NumberCycle re;
    QMap<QString, NumberCycle> map = load();
    if (map.contains(name)) {
        re = map[name];
    }
    return re;
}

QString NumberCycle::exampleIdent( const QString& docType,
                                   const QDate& date,
                                   const QString& addressUid)
{
    int cnt = counter();
    int dayCnt{1};

    return generateDocumentIdent(getTemplate(), docType,
                                 date, addressUid,
                                 cnt+1, dayCnt);
}

QString NumberCycles::generateIdent(const QString& name, const QString& docType,
                                    const QDate& date, const QString& addressUid)
{
    int dayCnt{1};
    NumberCycle nc;
    nc = get(name);

    int newCounter = increaseLocalCounter(name);

    if (newCounter > -1) {
        return generateDocumentIdent(nc.getTemplate(), docType,
                                     date, addressUid,
                                     newCounter, dayCnt);
    }
    return QString();
}

int NumberCycles::increaseLocalCounter(const QString& ncName)
{
    NumberCycle nc;
    const int MaxAttempt{10};
    int attempt{0};
    int newCnt{-1};

    while( attempt < MaxAttempt) {
        if (tryLock()) {
            QMap<QString, NumberCycle> map = load();

            if (map.contains(ncName)) {
                nc = map[ncName];
            } else {
                nc.setName(ncName);
            }

            int cnt = nc.counter();
            nc.setCounter(cnt+1);

            SaveResult res = save(nc);
            if (res == SaveResult::SaveOk) {
                newCnt = cnt+1;
            } else {
                qDebug() << "Could not save nc file in increaseCounter";
            }
            unlock();
            return newCnt;
        }
        attempt++;
        QThread::msleep(2*attempt); // Sleep a short time before trying agian. Lock should be gone after that.
    }
    if (attempt == MaxAttempt) {
        qDebug() << "Could not lock the numbercycle file";
    }
    return -1;
}

QMap<QString, NumberCycle> NumberCycles::load()
{
    QMap<QString, NumberCycle> map;

    const QString bDir = DefaultProvider::self()->kraftV2Dir(DefaultProvider::KraftV2Dir::NumberCycles);
    Q_ASSERT(!bDir.isEmpty());
    const QDir dir(bDir);

    const QStringList names {"*.xml"};
    const QStringList entries = dir.entryList(names);

    for (const QString& xmlFileName : entries) {
        QString const fullPathName = bDir + QDir::separator() + xmlFileName;
        QFile file(fullPathName);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "Unable to open xml document file:" << xmlFileName ;
            continue;
        }

        QDomDocument domDoc;
        const QByteArray arr = file.readAll();
        QDomDocument::ParseResult pr = domDoc.setContent(arr);

        if (!pr) {
            qDebug() << "Unable to set file content as xml:" << pr.errorMessage << "at line" <<pr.errorLine;
            file.close();
            continue;
        }
        file.close();

        // ---- Parsing starts here

        QDomElement ncs = domDoc.firstChildElement("kraftNumberCycle");
        NumberCycle nc;
        QString t = KraftXml::childElemText(ncs, "name");
        nc.setName(t);
        t = KraftXml::childElemText(ncs, "lastNumber");
        nc.setCounter(t.toInt());
        t = KraftXml::childElemText(ncs, "template");
        nc.setTemplate(t);
        t = KraftXml::childElemText(ncs, "dbId");
        nc.setDbId(t.toInt());

        map.insert(nc.name(), nc);
    }
    return map;
}

bool NumberCycles::saveNCXml(const QString& name, const QString& xml, const QString& baseDir)
{
    Q_ASSERT(!name.isEmpty());

    QString saveName{name};
    QString v2Dir{baseDir};
    if (baseDir.isEmpty()) {
        v2Dir = DefaultProvider::self()->kraftV2Dir();
    }

    QDir dir(v2Dir);
    dir.cd(DefaultProvider::self()->kraftV2Subdir(DefaultProvider::KraftV2Dir::NumberCycles));

    bool re{false};
    if (!saveName.endsWith(".xml")) saveName.append(".xml");
    QSaveFile file(dir.absoluteFilePath(saveName));
    if ( file.open( QIODevice::WriteOnly | QIODevice::Text) ) {
        re = file.write(xml.toUtf8());

        if (re) {
            re = file.commit();
        }
    }
    return re;
}

NumberCycles::SaveResult NumberCycles::save(const NumberCycle& nc, const QString& baseDir)
{
    if (nc.name().isEmpty()) {
        qDebug() << "Can not save numbercylce without name";
        return SaveResult::OpenFail;
    }
    const QString kncStr{"kraftNumberCycle"};

    QDomDocument xmldoc(kncStr);
    QDomProcessingInstruction instr = xmldoc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
    xmldoc.appendChild(instr);

    QDomElement root = xmldoc.createElement(kncStr);
    root.setAttribute("schemaVersion", "1");
    xmldoc.appendChild( root );

    root.appendChild(KraftXml::textElement(xmldoc, "name", nc.name()));
    root.appendChild(KraftXml::textElement(xmldoc, "lastNumber", QString::number(nc.counter())));
    root.appendChild(KraftXml::textElement(xmldoc, "template", nc.getTemplate()));
    root.appendChild(KraftXml::textElement(xmldoc, "dbId", nc.dbId()));

    const QString xml = xmldoc.toString();

    // Save to file
    SaveResult re{SaveResult::OpenFail};
    if (saveNCXml(nc.name(), xml, baseDir)) {
        re = SaveResult::SaveOk;
    }

    return re;
}

NumberCycles::SaveResult NumberCycles::remove(const QString& name, const QString& baseDir)
{
    QString saveName{name};
    QString v2Dir{baseDir};
    if (baseDir.isEmpty()) {
        v2Dir = DefaultProvider::self()->kraftV2Dir();
    }

    QDir dir(v2Dir);
    dir.cd(DefaultProvider::self()->kraftV2Subdir(DefaultProvider::KraftV2Dir::NumberCycles));

    SaveResult re{SaveResult::RemoveFail};
    if (!saveName.endsWith(".xml")) saveName.append(".xml");

    const QString file(dir.absoluteFilePath(saveName));
    if (QFile::remove(file)) {
        re = SaveResult::SaveOk;
    }
    return re;
}

NumberCycles::SaveResult NumberCycles::saveAll(const QMap<QString, NumberCycle>& ncs, const QString& baseDir)
{
    bool error{false};
    QMap<QString, NumberCycle> existNCs = load();
    int cnt{0};
    const auto values = ncs.values();
    for (const NumberCycle& nc : values){
        if (existNCs.contains(nc.name())) {
            const NumberCycle ncOld = existNCs[nc.name()];
            if (ncOld == nc) {
                existNCs.remove(nc.name());
                continue;
            }
        }
        SaveResult re = save(nc, baseDir);
        existNCs.remove(nc.name());
        if (re == SaveResult::SaveOk){
            cnt++;
        } else {
            error = true;
        }
    }

    // if existNCs still contains names, these need to be deleted because
    // they have not been in ncs
    int rems{0};
    const QStringList keys = existNCs.keys();
    for (const auto &k : keys) {
        remove(k, baseDir);
        rems++;
    }

    qDebug() << "Saved" << cnt << "numbercycles and removed"<< rems <<"successfully";
    return error ? SaveResult::PartialFail : SaveResult::SaveOk;
}

// this lock code does not do anything at all because the local file
// is exclusive anyway.
bool NumberCycles::tryLock()
{
    bool re{true};
    qDebug() << "Try to lock numbercycles:" << re;
    return re;
}

void NumberCycles::unlock()
{
    qDebug() << "UNLock numbercycles";
}
