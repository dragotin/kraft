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

#include <QtCore>
#include <QSqlError>
#include <QSqlDatabase>
#include <QMap>
#include <QDateTime>

#include "metaxmlparser.h"

class dbID;
class DbInitDialog;
class SetupAssistant;
/**
  *@author Klaas Freitag
  */

class SqlCommand
{
public:
  SqlCommand();
  SqlCommand( const QString&, const QString&, bool );

  QString message();
  QString command();
  bool    mayfail();

private:
  QString mSql;
  QString mMessage;
  bool    mMayFail;
};

class SqlCommandList: public QList<SqlCommand>
{
public:
  SqlCommandList();

  QList<MetaDocTypeAdd> metaAddDocTypeList() const;
  void setMetaAddDocTypeList( QList<MetaDocTypeAdd> list );
  void setNumber(int no);
  int number();
private:
  QList<MetaDocTypeAdd> _docTypeMetaList;
  int _number;
};


class KraftDB : public QObject
{
  Q_OBJECT

public:
  ~KraftDB();

  static KraftDB *self();

  dbID getLastInsertID();

  QSqlDatabase *getDB(){ return &m_db; }
  QString qtDriver();

  typedef QMap<QString, QString> StringMap;
  QStringList wordList( const QString&, StringMap replaceMap = StringMap() );
  void writeWordList( const QString&, const QStringList& );

  QString databaseName() const;

  QSqlError lastError();

  bool isSqlite();

  bool isOk() {
    return mSuccess;
  }

  bool dbConnect( const QString& driver, const QString& dbName,
                  const QString& dbUser, const QString& dbHost,
                  const QString& dbPasswd );

  /**
   * check if the database is open and contains the table kraftsystem. Still
   * the Schema version can be invalid, check currentSchemaVersion().
   */
  bool databaseExists();

  /*
   * required and current schema versions. Must be equal for a healty
   * Kraft database. If currentSchemaVersion is smaller than requiredSchemaVersion,
   * the db needs an update.
   */
  int currentSchemaVersion();
  int requiredSchemaVersion();

  void setSchemaVersion( const QString& );

  // database aware current time stamp
  QString currentTimeStamp( const QDateTime& dt = QDateTime() );
  /**
   * Euro sign encoding to work around a problem with mysql
   */
  QString mysqlEuroEncode( const QString& ) const;
  QString mysqlEuroDecode( const QString& ) const;


  QString replaceTagsInWord( const QString& w, StringMap replaceMap ) const;

  // void checkDatabaseSetup( QWidget* );

  SqlCommandList parseCommandFile( int currentVersion );
  SqlCommandList parseCommandFile( const QString& file );

  QList<MetaDocTypeAdd> parseMetaFile( int currentVersion );

  int processSqlCommands( const SqlCommandList& );

  bool checkTableExistsSqlite(const QString& name, const QStringList& lookupCols);

  KraftDB();

signals:
  void statusMessage( const QString& );
  void processedSqlCommand( bool );

private: // Private attributes
  void close();
  int checkConnect(const QString&, const QString&,
                    const QString&, const QString& , int port);

  /** The default database */
  QSqlDatabase m_db;

  QWidget *mParent;

  bool mSuccess;
  const QString EuroTag;
  QString mDatabaseDriver;
  QString mDatabaseName;
  DbInitDialog *mInitDialog;
  SetupAssistant *mSetupAssistant;
};

#endif
