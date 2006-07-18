#ifndef DBIDS_H
#define DBIDS_H

#include <qstring.h>
#include <qdict.h>
/**
 * utility class that provides a simple database id object.
 * It's usefull to work with dicts which do not work on base
 * types like int
 */

class dbID
{
public:
    dbID(int id):m_id(id){};
    dbID():m_id(-1){};
    int intID() const { return m_id; }

    bool operator==( const int& _u ) const
        { return m_id == _u; }
    bool operator==( const long& _u ) const
        { return m_id == _u; }
    bool operator==( const dbID& _u ) const
        { return m_id == _u.m_id; }
    dbID& operator=( const dbID& _u ) {
        m_id = _u.m_id;
        return *this;
    }
    dbID& operator=( const QString& _u ) {
      bool ok;
      int id = _u.toInt( &ok );
      if( ok ) {
        m_id = id;
      } 
      return *this; 
    }

    dbID& operator=( const int _u ) {
        m_id = _u;
        return *this;
    }
    bool isOk()
        { return m_id > -1; }
    int  toInt() { return m_id; }
    QString toString() { return QString::number(m_id); }
private:
    int m_id;
};

typedef QDict<dbID> dbIdDict;
typedef QValueList<dbID> DBIdList;

#endif
