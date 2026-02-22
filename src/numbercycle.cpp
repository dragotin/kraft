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

const QString NumberCycle::toXml() const
{
    if (name().isEmpty()) {
        qDebug() << "Can not save numbercylce without name";
        return QString();
    }
    const QString kncStr{"kraftNumberCycle"};

    QDomDocument xmldoc(kncStr);
    QDomProcessingInstruction instr = xmldoc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
    xmldoc.appendChild(instr);

    QDomElement root = xmldoc.createElement(kncStr);
    root.setAttribute("schemaVersion", "1");
    xmldoc.appendChild( root );

    root.appendChild(KraftXml::textElement(xmldoc, "name", name()));
    root.appendChild(KraftXml::textElement(xmldoc, "lastNumber", QString::number(counter())));
    root.appendChild(KraftXml::textElement(xmldoc, "template", getTemplate()));
    root.appendChild(KraftXml::textElement(xmldoc, "dbId", dbId()));

    const QString xml = xmldoc.toString();

    return xml;
}

void NumberCycle::parseXml(QDomDocument &domDoc)
{
    QDomElement ncs = domDoc.firstChildElement("kraftNumberCycle");
    QString t = KraftXml::childElemText(ncs, "name");
    setName(t);
    t = KraftXml::childElemText(ncs, "lastNumber");
    setCounter(t.toInt());
    t = KraftXml::childElemText(ncs, "template");
    setTemplate(t);
    t = KraftXml::childElemText(ncs, "dbId");
    setDbId(t.toInt());

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

// ====================================================================================


// ====================================================================================
NumberCycles::NumberCycles()
    : Lister<NumberCycle>(DefaultProvider::KraftV2Dir::NumberCycles)
{

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
    NumberCycles ncs;
    ncs.loadAll();

    const int MaxAttempt{10};
    int attempt{0};
    int newCnt{-1};

    while( attempt < MaxAttempt) {
        if (tryLock()) {
            nc = ncs.get(ncName);

            int cnt = nc.counter();
            nc.setCounter(cnt+1);

            SaveResult res = ncs.save(nc);
            if (res == SaveResult::SaveOk) {
                newCnt = cnt+1;
            } else {
                qDebug() << "Could not save nc file in increaseCounter";
            }
            unlock();
            return newCnt;
        }
        attempt++;
        QThread::msleep(2*attempt); // Sleep a short time before trying again. Lock should be gone after that.
    }
    if (attempt == MaxAttempt) {
        qDebug() << "Could not lock the numbercycle file";
    }
    return -1;
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
