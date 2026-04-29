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
#include <QDomDocument>
#include <QSaveFile>

#include "defaultprovider.h"
#include "kraftcat_export.h"
#include "xmldirlister.h"

class KraftDoc;
class QDateTime;

class KRAFTCAT_EXPORT NumberCycle
{
    friend class DbToXMLConverter;
    friend class NumberCycles;

public:
    NumberCycle();

    bool operator==(const NumberCycle& other) const;

    void setName( const QString& );
    QString name() const;

    void setTemplate( const QString& );
    QString getTemplate() const;

    void setCounter( int );
    int  counter() const;

    static QString defaultName();

    void parseXml(QDomDocument &domDoc);
    const QString toXml() const;

    bool isEmpty() const { return _name.isEmpty(); }
    bool modified() const { return _isModified; }

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
    bool    _isModified;
    int     _dbId;
};

// ################################################################################

class KRAFTCAT_EXPORT NumberCycles
        : public XmlDirLister<NumberCycle>
{    
public:
    friend class DbToXMLConverter;

    NumberCycles();

    QString generateIdent(const QString& name, const QString &docType, const QDate &date, const QString &addressUid);

private:

    int increaseLocalCounter(const QString& nc);
    bool tryLock();
    void unlock();
};


#endif
