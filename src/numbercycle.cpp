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
    return QString( "default" );
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

    int newCounter = increaseCounter(name);

    if (newCounter > -1) {
        return generateDocumentIdent(nc.getTemplate(), docType,
                                     date, addressUid,
                                     newCounter, dayCnt);
    }
    return QString();
}

int NumberCycles::increaseCounter(const QString& ncName)
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
                nc.setName(ncName); // FIXME default template
            }

            int cnt = nc.counter();
            nc.setCounter(cnt+1);

            map[ncName] = nc;
            SaveResult res = save(map);
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

NumberCycles::SaveResult NumberCycles::addUpdate(const NumberCycle& nc)
{
    SaveResult res;
    const int MaxAttempt{10};
    int attempt{0};

    QMap<QString, NumberCycle> ncMap = load();

    ncMap.insert(nc.name(), nc); // insert the new numbercylce

    while( attempt < MaxAttempt) {
        if (tryLock()) {
            QMap<QString, NumberCycle> map = load();
            map[nc.name()] = nc;
            res = save(map);

            if (res == SaveResult::SaveOk) {
                qDebug() << "Successfully updated" << nc.name();
            } else {
                qDebug() << "Could not save nc file in addUpdate";
            }
            unlock();
            return res;
        }
        attempt++;
        QThread::msleep(2*attempt); // Sleep a short time before trying agian. Lock should be gone after that.
    }

    return SaveResult::Locked;
}

QMap<QString, NumberCycle> NumberCycles::load()
{
    QMap<QString, NumberCycle> map;

    const QString bDir = DefaultProvider::self()->kraftV2Dir(DefaultProvider::KraftV2Dir::NumberCycles);
    Q_ASSERT(!bDir.isEmpty());
    const QDir dir(bDir);
    const QString xmlFileName {dir.absoluteFilePath("numbercycles.xml")};

    QFile file(xmlFileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Unable to open xml document file:" << xmlFileName ;
        return map;
    }

    QDomDocument domDoc;

    const QByteArray arr = file.readAll();
    QString errMsg;
    if (!domDoc.setContent(arr, &errMsg)) {
        qDebug() << "Unable to set file content as xml:" << errMsg;
        file.close();
        return map;
    }
    file.close();

    // ---- Parsing starts here

    QDomElement ncs = domDoc.firstChildElement("kraftNumberCycles");
    QDomElement cycleElem = ncs.firstChildElement("cycle");

    while( !cycleElem.isNull()) {
        NumberCycle nc;
        QString t = KraftXml::childElemText(cycleElem, "name");
        nc.setName(t);
        t = KraftXml::childElemText(cycleElem, "lastNumber");
        nc.setCounter(t.toInt());
        t = KraftXml::childElemText(cycleElem, "template");
        nc.setTemplate(t);
        t = KraftXml::childElemText(cycleElem, "dbId");
        nc.setDbId(t.toInt());
        cycleElem = cycleElem.nextSiblingElement("cycle");

        map.insert(nc.name(), nc);
    }

    return map;
}

NumberCycles::SaveResult NumberCycles::save(const QMap<QString, NumberCycle>& ncs)
{
    int cnt{0};
    const QString kncStr{"kraftNumberCycles"};

    QDomDocument xmldoc(kncStr);
    QDomProcessingInstruction instr = xmldoc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
    xmldoc.appendChild(instr);

    QDomElement root = xmldoc.createElement(kncStr);
    root.setAttribute("schemaVersion", "1");
    xmldoc.appendChild( root );

    for (const NumberCycle& nc : ncs.values()){
        QDomElement cycle = xmldoc.createElement( "cycle" );
        root.appendChild(cycle);
        cycle.appendChild(KraftXml::textElement(xmldoc, "name", nc.name()));
        cycle.appendChild(KraftXml::textElement(xmldoc, "lastNumber", QString::number(nc.counter())));
        cycle.appendChild(KraftXml::textElement(xmldoc, "template", nc.getTemplate()));
        cycle.appendChild(KraftXml::textElement(xmldoc, "dbId", nc.dbId()));
        cnt++;
    }

    const QString& xml = xmldoc.toString();
    const QString dirStr{DefaultProvider::self()->kraftV2Dir(DefaultProvider::KraftV2Dir::NumberCycles)};
    Q_ASSERT(!dirStr.isEmpty());
    QDir dir(dirStr);

    const QString fd{dir.absoluteFilePath("numbercycles.xml")};
    QFileInfo fi{fd};
    SaveResult res{SaveResult::SaveOk};
    bool re{false};

    QSaveFile file(fd);
    if ( file.open( QIODevice::WriteOnly | QIODevice::Text) ) {
        re = file.write(xml.toUtf8());

        if (re) {
            re = file.commit();
        }
    }
    if (re) {
        res = SaveResult::SaveOk;
        qDebug() << "Saved" << cnt << "numbercycles successfully";
    } else {
        res = SaveResult::OpenFail;
    }
    return res;

}

//
bool NumberCycles::tryLock()
{
    bool re{true};
    // Consider case that file does not exist at all.

    qDebug() << "Try to lock numbercycles:" << re;
    return re;
}

void NumberCycles::unlock()
{
    qDebug() << "UNLock numbercycles";
}
