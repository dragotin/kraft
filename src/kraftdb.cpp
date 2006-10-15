/***************************************************************************
                          KraftDB.cpp  -
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
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <qfile.h>
#include <qsqldatabase.h>
#include <qsqlcursor.h>
#include <qsqlquery.h>
#include <qstringlist.h>

#include "version.h"
#include "kraftdb.h"
#include "dbids.h"

#include "katalogsettings.h"

#define DB_DRIVER "QMYSQL3"
// #define DB_DRIVER "QSQLITE"

QSqlDatabase* KraftDB::m_db = 0;

KraftDB::KraftDB(){

}

QSqlDatabase* KraftDB::getDB()
{
    if( m_db == 0 )
    {
        m_db = QSqlDatabase::addDatabase( DB_DRIVER );
        if ( ! m_db || m_db->isOpenError() )
        {
            kdError() <<  "Failed to connect to the database driver: " << m_db->lastError().text() << endl;
            return 0;
        }
        const QString dbFile = KatalogSettings::dbFile();
        if( dbFile.isEmpty() ) {
            kdError() << "Database name is not set!" << endl;
            QSqlDatabase::removeDatabase( m_db );
            m_db = 0;
        } else {
            kdDebug() << "Try to open database " << dbFile << endl;
            m_db->setDatabaseName( dbFile );
            m_db->setUserName( KatalogSettings::dbUser() );
            m_db->setPassword( KatalogSettings::dbPassword() );
            m_db->setHostName( KatalogSettings::dbServerName() );

            if ( m_db->open() ) {
                // Database successfully opened; we can now issue SQL commands.
                kdDebug() << "Database file " << dbFile << " opened successfully" << endl;
            } else {
                m_db = 0;
                kdError() << "## Could not open database file " << dbFile << endl;
            }
        }
    }
    return m_db;
}

dbID KraftDB::getLastInsertID()
{
    if( ! KraftDB::getDB() ) return 0;

    QSqlQuery query("SELECT LAST_INSERT_ID()");
    int id = -1;
    if( query.next() ) {
        id = query.value(0).toInt();
    }

    return dbID(id);
}

QStringList KraftDB::wordList( const QString& selector, StringMap replaceMap )
{
  QStringList re;

  if( ! KraftDB::getDB() ) return re;

  QSqlCursor cur( "wordLists" ); // Specify the table/view name
  // cur.setMode( QSqlCursor::ReadOnly );
  cur.select( QString( "category='%1'" ).arg( selector ) );
  while ( cur.next() ) {
    QString w = cur.value( "word" ).toString();

    StringMap::Iterator it;
    for ( it = replaceMap.begin(); it != replaceMap.end(); ++it ) {
      const QString key = it.key().utf8();
      const QString rep = it.data().utf8();
      w.replace( key, rep );
    }
    re << w;
  }
  return re;
}

void KraftDB::checkSchemaVersion()
{
  getDB();

  QSqlQuery q( "SELECT dbschemaversion FROM kraftsystem" );

  int currentVer = 0;
  if ( q.next() ) {
    currentVer = q.value( 0 ).toInt();
  }

  if ( currentVer < KRAFT_REQUIRED_SCHEMA_VERSION ) {
    kdDebug() << "Kraft Schema Version not sufficient: " << currentVer << endl;

    KStandardDirs stdDirs;

    while ( currentVer <= KRAFT_REQUIRED_SCHEMA_VERSION ) {
      ++currentVer;
      const QString migrateFilename = QString( "%1_dbmigrate.sql" ).arg( currentVer );
      QString findFile = "kraft/dbmigrate/" + migrateFilename;
      QString sqlFile = stdDirs.findResource( "data", findFile );
      if ( ! sqlFile.isEmpty() ) {
        kdDebug() << "Opening migration file " << sqlFile << endl;

        QFile f( sqlFile );
        if ( !f.open( IO_ReadOnly ) ) {
          kdError() << "Could not open " << sqlFile << endl;
        } else {
          QTextStream ts( &f );
          ts.setEncoding(QTextStream::UnicodeUTF8);

          while ( !ts.atEnd() ) {
            QString sql = ts.read();
            if ( !sql.isEmpty() ) {
              if ( sql.lower().startsWith( "message:" ) ) {
                QString msg = sql.left( sql.length()-8 );
                kdDebug() << "Msg: " << msg << endl;
              }
              else if ( q.exec( sql ) ) {
                kdDebug() << "Successfull SQL Command: " << sql << endl;
              } else {
                kdDebug() << "Failed SQL Command: " << sql << endl;
              }
            }
          }
          f.close();
        }
      }
    }
  } else {
    kdDebug() << "Kraft Schema Version is ok: " << currentVer << endl;
  }

}


// not yet used.
void KraftDB::checkInit()
{
  kdDebug() << "** Database init **" << endl;
  if( m_db ) {
    kdError() << "global db handle is not zero, can not init!" << endl;
  }

        // The database is not yet open. Thus we can move the file away
  QString dbFile = KatalogSettings::dbFile();
  kdDebug() << "Database file is " << dbFile << endl;
  if( ! dbFile.isEmpty() ) {
            // backup this file
    dBFileBackup( dbFile );
  } else {
    QString dbName = KatalogSettings::defaultDbName();
    QString dbPath = KatalogSettings::dbPath();
    if( dbPath.isEmpty() ) {
      KStandardDirs stdDirs;
      dbPath = stdDirs.saveLocation( "data" );
    }

    QString dbFile = dbPath + dbName;
    kdDebug() << "Database file: " << dbFile << endl;
    KatalogSettings::setDbFile( dbFile );

    if( doInitialSetup() ) {
      kdDebug() << "Initial setup successfull" << endl;
    }
  }
}

QString KraftDB::qtDriver()
{
    return DB_DRIVER;
}

bool KraftDB::doInitialSetup()
{
    bool result = true;
#if 0
    KStandardDirs stdDirs;
    QString schemaFile = stdDirs.findResource( "data", "create_schema_sqlite.sql" );
    kdDebug() << "Loading create file from " << schemaFile << endl;

    result = sendFileToDb( schemaFile );
#endif
    KMessageBox::sorry( 0, i18n("The database is not yet initialised. Please call the "
                             "script that creates the database." ),
                             i18n("Initialisation") );
    return result;
}

bool KraftDB::sendFileToDb( const QString& )
{
    bool result = true;

    return result;
}

void KraftDB::dBFileBackup( const QString& filename )
{
    kdDebug() << "Backup of file " << filename << endl;
}

KraftDB::~KraftDB(){
}
