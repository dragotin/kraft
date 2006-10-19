/***************************************************************************
                          kraftdb.h  -
                             -------------------
    begin                : Die Feb 3 2004
    copyright            : (C) 2004 by Klaas Freitag
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

#ifndef KRAFTDB_H
#define KRAFTDB_H

#define MWST 16.0

class QSqlDatabase;

/**
  *@author Klaas Freitag
  */
#include <qmap.h>
#include <qobject.h>

class dbID;
class KProcess;

class KraftDB : public QObject
{

  Q_OBJECT

public:
  ~KraftDB();
  /** Read property of QSqlDatabase* m_db. */
  static KraftDB *self();

  double getMwSt(){
    return MWST;
  }

  dbID getLastInsertID();

  void checkInit();
  void checkSchemaVersion();
  QSqlDatabase *getDB() { return m_db; }
  QString qtDriver();

  typedef QMap<QString, QString> StringMap;
  QStringList wordList( const QString&, StringMap replaceMap = StringMap() );
signals:
  void statusMessage( const QString& );

private: // Private attributes
  KraftDB();

  /** The default database */
  QSqlDatabase* m_db;
  KProcess *mProcess;
  static KraftDB *mSelf;
};

#endif
