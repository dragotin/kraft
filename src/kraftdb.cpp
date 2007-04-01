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
  :QObject (), m_db( 0 ),  mSuccess( true )
{
  QStringList list = QSqlDatabase::drivers().grep( DB_DRIVER );
  if( list.size() == 0 ) {
    kdError() << "Database Driver could not be loaded." << endl;
    mSuccess = false ;
  }

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
    int re = checkConnect( KatalogSettings::dbServerName(), dbFile,
                           KatalogSettings::dbUser(), KatalogSettings::dbPassword() );
    if ( re == 0 ) {

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

int KraftDB::checkConnect( const QString& host, const QString& dbName,
                            const QString& user, const QString& pwd )
{
  if ( dbName.isEmpty() || !m_db ) return false;
  m_db->setHostName( host );
  m_db->setDatabaseName( dbName );
  m_db->setUserName( user );
  m_db->setPassword( pwd );

  int re = 0;

  m_db->open();
  if ( m_db->isOpenError() ) {
    kdDebug() << "ERR opening the db: " << m_db->lastError().text() <<
      ", type is " << m_db->lastError().type() << endl;
    re = m_db->lastError().type();
  }
  return re;
}

QSqlError KraftDB::lastError()
{
  if ( m_db ) {
    return m_db->lastError();
  }

  return QSqlError();
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
    kdDebug() << "Adding to wordlist <" << w << ">" << endl;
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

void KraftDB::writeWordList( const QString& listName, const QStringList& list )
{
  kdDebug() << "Saving " << list[0] << " into list " << listName << endl;
  QSqlQuery  qd;
  qd.prepare( "DELETE FROM wordLists WHERE category=:catName" );
  qd.bindValue( ":catName", listName );
  qd.exec();

  QSqlQuery qi;
  qi.prepare( "INSERT INTO wordLists (category, word) VALUES( :category, :entry )" );

  qi.bindValue( ":category", listName );
  for ( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
    qi.bindValue( ":entry", *it );
    qi.exec();
  }
}

void KraftDB::checkSchemaVersion( QWidget *parent )
{
  kdDebug() << "The country setting is " << KGlobal().locale()->country() << endl;

  if ( m_db->tables().contains( "kraftsystem" ) == 0 ) {
    if ( ! createDatabase( parent ) ) {
      kdDebug() << "Failed to create the database, returning. Thats a bad condition." << endl;
      return;
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

    emit statusMessage( i18n( "Database schema not up to date" ) );
    if( KMessageBox::warningYesNo( parent,
                                 i18n( "This Kraft database schema is not up to date "
                                       "(it is version %1 instead of the required version %2).\n"
                                       "Kraft is able to update it to the new version automatically.\n"
                                       "WARNING: MAKE SURE A GOOD BACKUP IS AVAILABLE!\n"
                                       "Do you want Kraft to update the database schema version now?")
                                   .arg(  currentVer ).arg( KRAFT_REQUIRED_SCHEMA_VERSION ),
                                 i18n("Database Schema Update") ) == KMessageBox::Yes ) {

      bool ok = true;
      while ( currentVer < KRAFT_REQUIRED_SCHEMA_VERSION ) {
        ++currentVer;
        const QString migrateFilename = QString( "%1_dbmigrate.sql" ).arg( currentVer );
        int allCmds = 0;
        int sqlc = playSqlFile( migrateFilename, allCmds );
        if ( sqlc == 0 ) {
          kdWarning() << "No (zero) commands where loaded and executed from " << migrateFilename << endl;
          ok = false;
        } else if ( allCmds != sqlc ) {
          kdDebug() << "WRN: only " << sqlc << " from " << allCmds << " sql commands "
            "were executed correctly" << endl;
          ok = false;
        } else {
          kdDebug() << "All sql commands successfull in file: " << migrateFilename << ": " << sqlc << endl;
        }
      }
      /* Now update to the required schema version */
      if ( ok ) {
        q.exec( "UPDATE kraftsystem SET dbSchemaVersion="
                + QString::number( KRAFT_REQUIRED_SCHEMA_VERSION ) );
      }
    }
  } else {
    kdDebug() << "Kraft Schema Version is ok: " << currentVer << endl;
    emit statusMessage( i18n( "Database Schema Version ok" ) );
  }
}

bool KraftDB::createDatabase( QWidget *parent )
{
  // The kraftsystem table is not there, reinit the entire db.
  bool ret = false;
  emit statusMessage( i18n( "Recreate Database" ) );
  if( KMessageBox::warningYesNo( parent,
                                 i18n( "The Kraft System Table was not found in database %1."
                                       " Do you want me to rebuild the database?\n"
                                       "WARNING: ALL YOUR DATA WILL BE DESTROYED!")
                                 .arg(  KatalogSettings::dbFile() ),
                                 i18n("Database Rebuild") ) == KMessageBox::Yes ) {
    emit statusMessage( i18n( "Creating Database..." ) );

    if ( m_db->tables().size() > 0 ) {
      QString allTables = QString( "DROP TABLE %1;" ).arg( m_db->tables().join( ", " ) );
      kdDebug() << "Erasing all tables " << allTables << endl;
      QSqlQuery q;
      q.exec( allTables );
    }

    int allCmds = 0;
    int goodCmds = playSqlFile( "create_schema.sql", allCmds );
    if ( goodCmds == allCmds ) {
      QString dbFill( "fill_schema_en.sql" );

      if ( KGlobal().locale()->country() == "de" ) {
        dbFill = "fill_schema_de.sql";
      }
      emit statusMessage( i18n( "Filling Database..." ) );
      if ( playSqlFile( dbFill, allCmds ) == 0 ) {
        kdDebug() << "Failed to fill the database" << endl;
        emit statusMessage( i18n( "Failed." ) );
      } else {
        ret = true;
        emit statusMessage( i18n( "Ready." ) );
      }
    } else if ( goodCmds > 0 && allCmds > 0 ) {
      // There were some commands failing
    } else if ( allCmds == 0 ) {
      // no commands were found
    }
  }
  return ret;
}

int KraftDB::playSqlFile( const QString& file, int& overallCount )
{
  KStandardDirs stdDirs;
  QString findFile = "kraft/dbmigrate/" + file;
  QString sqlFile = stdDirs.findResource( "data", findFile );
  int cnt = 0;

  // search in dbcreate as well.
  if ( sqlFile.isEmpty() ) {
    findFile = "kraft/dbinit/" + file;
    sqlFile = stdDirs.findResource( "data", findFile );
  }

  if ( ! sqlFile.isEmpty() ) {
    kdDebug() << "Opening migration file " << sqlFile << endl;

    QFile f( sqlFile );
    if ( !f.open( IO_ReadOnly ) ) {
      kdError() << "Could not open " << sqlFile << endl;
    } else {
      QTextStream ts( &f );
      ts.setEncoding( QTextStream::UnicodeUTF8 );

      QSqlQuery q;
      QString allSql = ts.read();
      QStringList sqlList = QStringList::split( ";", allSql );

      for ( QStringList::iterator it = sqlList.begin();
            it != sqlList.end(); ++it ) {
        QString sql = QString( "%1;" ).arg( ( *it ).stripWhiteSpace() );

        if ( sql != ";" ) /* avoid empty lines */ {
          QRegExp reg( "\\s*#\\s*message: ?(.*)\\s*\\n" );
          reg.setMinimal( true );
          int pos = reg.search( sql.lower(),  0 );
          if ( pos > -1 ) {
            QString msg = reg.cap( 1 );
            sql = sql.remove ( reg );
            kdDebug() << "Msg: >" << msg << "<" << endl;
            emit statusMessage( msg );
          }

          if ( !sql.isEmpty() ) {
            overallCount++;
            if ( q.exec( sql ) ) {
              kdDebug() << "Successfull SQL Command: " << sql << endl;
              cnt ++;
            } else {
              QSqlError err = q.lastError();
              kdDebug() << "Failed SQL Command " << sql << ": " << err.text() << endl;
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
