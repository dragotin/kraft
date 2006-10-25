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
#include <kstaticdeleter.h>

#include <qfile.h>
#include <qsqldatabase.h>
#include <qsqlcursor.h>
#include <qsqlquery.h>
#include <qstringlist.h>
#include <qregexp.h>

#include "version.h"
#include "kraftdb.h"
#include "dbids.h"

#include "katalogsettings.h"

#define DB_DRIVER "QMYSQL3"
// #define DB_DRIVER "QSQLITE"

static KStaticDeleter<KraftDB> selfDeleter;

KraftDB* KraftDB::mSelf = 0;

KraftDB::KraftDB()
  :QObject (), m_db( 0 )
{
  m_db = QSqlDatabase::addDatabase( DB_DRIVER );
  if ( ! m_db || m_db->isOpenError() )
  {
    kdError() <<  "Failed to connect to the database driver: "
              << m_db->lastError().text() << endl;
    mSuccess = false;
  }
  QString dbFile;
  if ( mSuccess ) {
     dbFile = KatalogSettings::dbFile();
    if( dbFile.isEmpty() ) {
      kdError() << "Database name is not set!" << endl;
      // dbFile = defaultDatabaseName();
      mSuccess = false;
    }
  }
  if ( mSuccess ) {
    kdDebug() << "Try to open database " << dbFile << endl;
    if ( checkConnect( KatalogSettings::dbServerName(), dbFile,
                       KatalogSettings::dbUser(), KatalogSettings::dbPassword() ) ) {
      // Database successfully opened; we can now issue SQL commands.
      kdDebug() << "Database " << dbFile << " opened successfully" << endl;
    } else {
      kdError() << "## Could not open database file " << dbFile << endl;
      mSuccess = false;
    }
  }
}

KraftDB *KraftDB::self()
{
  if ( !mSelf ) {
    selfDeleter.setObject( mSelf, new KraftDB() );
  }
  return mSelf;
}

bool KraftDB::checkConnect( const QString& host, const QString& dbName,
                            const QString& user, const QString& pwd )
{
  if ( dbName.isEmpty() || !m_db ) return false;
  m_db->setHostName( host );
  m_db->setDatabaseName( dbName );
  m_db->setUserName( user );
  m_db->setPassword( pwd );

  m_db->open();
  bool success = true;
  if ( m_db->isOpenError() ) {
    success = false;
    kdDebug() << "ERR opening the db: " << m_db->lastError().text() << endl;
  }
  return success;
}

dbID KraftDB::getLastInsertID()
{
    if( ! m_db ) return 0;

    QSqlQuery query("SELECT LAST_INSERT_ID()");
    int id = -1;
    if( query.next() ) {
        id = query.value(0).toInt();
    }

    return dbID(id);
}

QString KraftDB::databaseName() const
{
  return KatalogSettings::dbFile();
}

QString KraftDB::defaultDatabaseName() const
{
  return QString( "Kraft" );
}

QStringList KraftDB::wordList( const QString& selector, StringMap replaceMap )
{
  QStringList re;

  if( ! m_db ) return re;

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

void KraftDB::checkSchemaVersion( QWidget *parent )
{
  if ( m_db->tables().contains( "kraftsystem" ) == 0 ) {
    // The kraftsystem table is not there, reinit the entire db.
    if( KMessageBox::warningYesNo( parent,
                                   i18n( "The Kraft System Table was not found in database %1."
                                         " Do you want me to rebuild the database?\n"
                                         "WARNING: ALL YOUR DATA WILL BE DESTROYED!").arg(  KatalogSettings::dbFile() ),
                                   i18n("Database Rebuild") ) == KMessageBox::Yes ) {
      playSqlFile( "create_db.sql" );
    }
  }

  QSqlQuery q( "SELECT dbSchemaVersion FROM kraftsystem" );
  emit statusMessage( i18n( "Checking Database Schema Version" ) );

  int currentVer = 0;
  if ( q.next() ) {
    currentVer = q.value( 0 ).toInt();
  }

  if ( currentVer < KRAFT_REQUIRED_SCHEMA_VERSION ) {
    kdDebug() << "Kraft Schema Version not sufficient: " << currentVer << endl;


    while ( currentVer < KRAFT_REQUIRED_SCHEMA_VERSION ) {
      ++currentVer;
      const QString migrateFilename = QString( "%1_dbmigrate.sql" ).arg( currentVer );
      int sqlc = playSqlFile( migrateFilename );
      kdDebug() << "Successfull sql commands in file: " << migrateFilename << ": " << sqlc << endl;
    }
    /* Now update to the required schema version */
    q.exec( "UPDATE kraftsystem SET dbSchemaVersion="
            + QString::number( KRAFT_REQUIRED_SCHEMA_VERSION ) );
  } else {
    kdDebug() << "Kraft Schema Version is ok: " << currentVer << endl;
    emit statusMessage( i18n( "Database Schema Version ok" ) );
  }
}

int KraftDB::playSqlFile( const QString& file )
{
  KStandardDirs stdDirs;
  QString findFile = "kraft/dbmigrate/" + file;
  QString sqlFile = stdDirs.findResource( "data", findFile );
  int cnt = 0;

  if ( ! sqlFile.isEmpty() ) {
    kdDebug() << "Opening migration file " << sqlFile << endl;

    QFile f( sqlFile );
    if ( !f.open( IO_ReadOnly ) ) {
      kdError() << "Could not open " << sqlFile << endl;
    } else {
      QTextStream ts( &f );
      ts.setEncoding( QTextStream::UnicodeUTF8 );

      QSqlQuery q;
      while ( !ts.atEnd() ) {
        QString sql = ts.readLine();
        if ( !sql.isEmpty() ) {
          QRegExp reg( "\\s*#\\s*message:\\s*" );
          int pos = sql.lower().find( reg );
          if ( pos > -1 ) {
            // QString msg = sql.right( sql.length()-pos );
            QString msg = sql.remove ( reg );
            kdDebug() << "Msg: >" << msg << "<" << endl;
            emit statusMessage( msg );
          } else {
            if ( q.exec( sql ) ) {
              kdDebug() << "Successfull SQL Command: " << sql << endl;
              cnt ++;
            } else {
              kdDebug() << "Failed SQL Command: " << sql << endl;
            }
          }
        }
      }
      f.close();
    }
  } else {
    kdDebug() << "No sql file found " << file << endl;
    emit statusMessage( i18n( "SQL File %1 not found" ).arg( file ) );
  }
  return cnt;
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
    // dBFileBackup( dbFile );
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

  }
}

QString KraftDB::qtDriver()
{
    return DB_DRIVER;
}

KraftDB::~KraftDB(){
}

#include "kraftdb.moc"
