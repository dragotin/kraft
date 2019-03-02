/***************************************************************************
                        dbids.h  - database id class
                             -------------------
    begin                : ?
    copyright            : (C) 2006- by Klaas Freitag
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
#ifndef DBIDS_H
#define DBIDS_H

#include <QString>
#include <QList>
#include <QHash>

/**
 * utility class that provides a simple database id object.
 * It's useful to work with dicts which do not work on base
 * types like int
 */

class dbID
{
public:
    dbID(int id):m_id(id){}
    dbID():m_id(-1){}
    int intID() const { return m_id; }

    bool operator==( const int& _u ) const
        { return m_id == _u; }
    bool operator==( const long& _u ) const
        { return m_id == _u; }
    bool operator==( const dbID& _u ) const
        { return m_id == _u.m_id; }
    bool operator!=( const dbID& _u ) const
        { return m_id != _u.m_id; }

    dbID& operator=( const QString& _u ) {
      bool ok;
      int id = _u.toInt( &ok );
      if( ok ) {
        m_id = id;
      }
      return *this;
    }

    bool operator<( dbID _id ) {
      if( m_id < _id.toInt() ) return true;
      return false;
    }

    dbID& operator=( const int _u ) {
        m_id = _u;
        return *this;
    }
    bool isOk() const
        { return m_id > -1; }
    int  toInt() { return m_id; }
    QString toString() const { return QString::number(m_id); }
private:
    int m_id;
};

typedef QHash<QString, dbID> dbIdDict;
typedef QHashIterator<QString, dbID> dbIdDictIterator;

typedef QList<dbID> DBIdList;

#endif
