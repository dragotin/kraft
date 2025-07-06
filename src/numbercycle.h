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

#ifndef NUMBERCYCLE_H
#define NUMBERCYCLE_H

#include <QString>
#include <QList>
#include <QDate>

#include "kraftcat_export.h"

class KraftDoc;
class QDateTime;

class KRAFTCAT_EXPORT NumberCycle
{
    friend class DbToXMLConverter;
    friend class NumberCycles;

public:
    NumberCycle();

    void setName( const QString& );
    QString name() const;

    void setTemplate( const QString& );
    QString getTemplate() const;

    void setCounter( int );
    int  counter() const;

    static QString defaultName();

    bool isEmpty() { return _name.isEmpty(); }

    QString exampleIdent(const QString& docType,
                         const QDate& date,
                         const QString& addressUid);

protected:
    QString dbId() const {return QString::number(_dbId);}
    void setDbId(int id) {_dbId = id;}

private:
    QString _name;
    QString _template;
    int     _counter;
    int     _dbId;
};


// FIXME: This could be a namespace rather than a "static object"

class KRAFTCAT_EXPORT NumberCycles
{    
public:
    friend class DbToXMLConverter;

    enum class SaveResult {
        SaveOk,
        OpenFail,
        NameFail,
        Locked,
        PartialFail
    };

    NumberCycles();

    static NumberCycle get(const QString& name);

    static QString generateIdent(const QString& name, const QString &docType, const QDate &date, const QString &addressUid);

    static QMap<QString, NumberCycle> load();

    static SaveResult save(const NumberCycle& ncs, const QString& baseDir = QString());
    static SaveResult saveAll(const QMap<QString, NumberCycle>& ncs);

private:
    static bool saveNCXml(const QString& name, const QString& xml, const QString& baseDir = QString());

    static int increaseLocalCounter(const QString& nc);
    static bool tryLock();
    static void unlock();
};


#endif
