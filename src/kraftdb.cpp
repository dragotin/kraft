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
#include <k3staticdeleter.h>

#include <QFile>
#include <QSqlQuery>
#include <QStringList>
#include <QRegExp>
#include <QTextStream>
#include <QSqlError>
#include <QDir>

#include "version.h"
#include "kraftdb.h"
#include "dbids.h"
#include "dbinitdialog.h"
#include "katalogsettings.h"
#include "defaultprovider.h"

SqlCommand::SqlCommand()
{

}

SqlCommand::SqlCommand( const QString& cmd, const QString& msg)
{
  mMessage = msg;
  if( !mMessage.isEmpty() && !mMessage.endsWith(';') ) {
    mMessage.append(';');
  }
  mSql = cmd;
  if( !mSql.isEmpty() && !mSql.endsWith(';') ) {
    mSql.append(';');
  }
}

QString SqlCommand::message()
{
  return mMessage;
}

QString SqlCommand::command()
{
  return mSql;
}

// ============================

SqlCommandList::SqlCommandList()
  :QList<SqlCommand>()
{

}

// ==========================================================================

static K3StaticDeleter<KraftDB> selfDeleter;

KraftDB* KraftDB::mSelf = 0;

KraftDB::KraftDB()
  :QObject (), mSuccess( true ),
   EuroTag( QString::fromLatin1( "%EURO" ) ),
   mInitDialog(0)
{
  mDatabaseDriver =KatalogSettings::self()->dbDriver().toUpper();
  if( mDatabaseDriver.isEmpty() ) {
    kError() << "Database Driver is not specified, check katalog settings";
    mSuccess = false;
  } else {
    kDebug() << "Using database Driver " << mDatabaseDriver;
  }

  QStringList list = QSqlDatabase::drivers();
  if( list.size() == 0 ) {
    kError() << "Database Drivers could not be loaded." << endl;
    mSuccess = false ;
  } else {
    if( list.indexOf( mDatabaseDriver ) == -1 ) {
      kError() << "Database Driver could not be loaded!" << endl;
      mSuccess = false;
    }
  }

  m_db = QSqlDatabase::addDatabase( mDatabaseDriver );

  if ( ! m_db.isValid() || m_db.isOpenError() )
  {
    kError() <<  "Failed to connect to the database driver: "
              << m_db.lastError().text() << endl;
    mSuccess = false;
  }

  QString dbFile;
  if ( mSuccess ) {
     dbFile = KatalogSettings::self()->dbFile();
    if( dbFile.isEmpty() ) {
      kError() << "Database name is not set!" << endl;
      // dbFile = defaultDatabaseName();
      mSuccess = false;
    }
  }

  if ( mSuccess ) {
    kDebug() << "Try to open database " << dbFile << endl;
    int re = checkConnect( KatalogSettings::self()->dbServerName(), dbFile,
                           KatalogSettings::self()->dbUser(), KatalogSettings::self()->dbPassword() );
    if ( re == 0 ) {

      // Database successfully opened; we can now issue SQL commands.
      kDebug() << "Database " << dbFile << " opened successfully" << endl;
    } else {
      kError() << "## Could not open database file " << dbFile << endl;
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
  if ( dbName.isEmpty() || !(m_db.isValid()) ) return false;
  m_db.setHostName( host );
  m_db.setDatabaseName( dbName );
  m_db.setUserName( user );
  m_db.setPassword( pwd );

  int re = 0;

  m_db.open();
  if ( m_db.isOpenError() ) {
    kDebug() << "ERR opening the db: " << m_db.lastError().text() <<
      ", type is " << m_db.lastError().type() << endl;
    re = m_db.lastError().type();
  }
  return re;
}

QSqlError KraftDB::lastError()
{
  return m_db.lastError();
}

dbID KraftDB::getLastInsertID()
{
    if(! ( m_db.isValid()) ) return 0;

    QSqlQuery query;
    if( mDatabaseDriver.toLower() == "qmysql" ) {
      query.prepare("SELECT LAST_INSERT_ID()");
      query.exec();
    } else if( mDatabaseDriver.toLower() == "qsqlite") {
      query.prepare( "SELECT last_insert_rowid()");
      query.exec();
    } else {
      kDebug() << "############# FATAL ERROR: Unknown database driver " << mDatabaseDriver;
    }
    int id = -1;

    if( query.next() ) {
      id = query.value(0).toInt();
    } else {
      kDebug() << "############# FATAL ERROR: Query for last insert id is invalid!";
    }
    kDebug() << "Last Insert ID: " << id;
    return dbID(id);
}

QString KraftDB::databaseName() const
{
  return KatalogSettings::self()->dbFile();
}

QString KraftDB::defaultDatabaseName() const
{
  return QString( "Kraft" );
}

QStringList KraftDB::wordList( const QString& selector, StringMap replaceMap )
{
  QStringList re;
  QSqlQuery query;

  query.prepare("SELECT category, word FROM wordLists WHERE category=:cat");
  query.bindValue(":cat", selector);
  query.exec();
  while ( query.next() ) {
    re << replaceTagsInWord( query.value(1).toString(), replaceMap );
  }
  return re;
}

QString KraftDB::replaceTagsInWord( const QString& w, StringMap replaceMap ) const
{
  QString re( w );

  QMap<int, QStringList> reMap;
  StringMap::Iterator it;
  for ( it = replaceMap.begin(); it != replaceMap.end(); ++it ) {
    reMap[it.key().length()] << it.key();
  }

  QMap<int, QStringList>::Iterator reIt;
  for ( reIt = reMap.end(); reIt != reMap.begin(); ) {
    --reIt;
    QStringList keys = reIt.value();
    kDebug() << "PP: " << keys;
    for ( QStringList::Iterator dtIt = keys.begin(); dtIt != keys.end(); ++dtIt ) {
      QString repKey = *dtIt;
      re.replace( repKey, replaceMap[repKey] );
    }
  }

  kDebug() << "Adding to wordlist <" << re << ">";

  return re;
}

void KraftDB::writeWordList( const QString& listName, const QStringList& list )
{
  kDebug() << "Saving " << list[0] << " into list " << listName << endl;
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

void KraftDB::createInitDialog( QWidget *parent )
{
  if( mInitDialog ) return;
  mInitDialog = new DbInitDialog( parent );
  mInitDialog->setModal( true );

  connect( this, SIGNAL(statusMessage( const QString& )),
           mInitDialog, SLOT( slotSetStatusText( const QString& )));
  connect( this, SIGNAL(processedSqlCommand( bool )),
           mInitDialog, SLOT( slotProcessedOneCommand( bool )));
}

bool KraftDB::checkSchemaVersion( QWidget *parent )
{
  kDebug() << "The country setting is " << DefaultProvider::self()->locale()->country() << endl;

  bool reinit = false;
  if ( m_db.tables().contains( "kraftsystem" ) == 0 ) {
    reinit = true;
    createInitDialog( parent );
    mInitDialog->show();

    if ( ! createDatabase( parent ) ) {
      kDebug() << "Failed to create the database, returning. Thats a bad condition." << endl;
      return false;
    }
  }

  QSqlQuery q( "SELECT dbSchemaVersion FROM kraftsystem" );
  emit statusMessage( i18n( "Checking Database Schema Version" ) );

  int currentVer = 0;
  if ( q.next() ) {
    currentVer = q.value( 0 ).toInt();
  }

  bool ok = true;

  if ( currentVer < KRAFT_REQUIRED_SCHEMA_VERSION ) {
    createInitDialog( parent );
    if( !reinit ) mInitDialog->show();

    kDebug() << "Kraft Schema Version not sufficient: " << currentVer << endl;

    emit statusMessage( i18n( "Database schema not up to date" ) );
    if( reinit || KMessageBox::warningYesNo( parent,
                                             i18n( "This Kraft database schema is not up to date "
                                                   "(it is version %1 instead of the required version %2).\n"
                                                   "Kraft is able to update it to the new version automatically.\n"
                                                   "WARNING: MAKE SURE A GOOD BACKUP IS AVAILABLE!\n"
                                                   "Do you want Kraft to update the database schema version now?")
                                             .arg(  currentVer ).arg( KRAFT_REQUIRED_SCHEMA_VERSION ),
                                             i18n("Database Schema Update") ) == KMessageBox::Yes ) {

      int overallCmdCount = 0;
      QList<SqlCommandList> commandLists;

      while ( currentVer < KRAFT_REQUIRED_SCHEMA_VERSION ) {
        const QString migrateFilename = QString( "%1_dbmigrate.sql" ).arg( currentVer );
        SqlCommandList cmds = parseCommandFile( migrateFilename );
        overallCmdCount += cmds.count();
        commandLists << cmds;
        ++currentVer;
      }
      mInitDialog->setOverallCount( overallCmdCount );

      int doneOverallCmds =  0;
      foreach( SqlCommandList cmds, commandLists ) {
        mInitDialog->setDetailOverallCnt( cmds.count() );
        int goodCmds = processSqlCommands( cmds );
        doneOverallCmds += goodCmds;
        if( goodCmds != cmds.count() ) {
          kDebug() << "Only performned " << goodCmds << " out of " << cmds.count();
        }
      }

      if( doneOverallCmds != overallCmdCount ) {
        kDebug() << "WRN: only " << doneOverallCmds << " from " << overallCmdCount << " sql commands "
            "were executed correctly" << endl;
        ok = false;

        KMessageBox::sorry( parent, i18n( "The update of the database failed, only "
                                          "%1 of %2 commands succeeded. It is not "
                                          "recommended to continue.\nPlease check the "
                                          "database consistency.\n" ).arg( doneOverallCmds ).arg( overallCmdCount ),
                            i18n( "Database Schema Update Error" ) );


      } else {
        kDebug() << "All sql commands successfully performned";
        /* Now update to the required schema version */
        q.exec( "UPDATE kraftsystem SET dbSchemaVersion="
                + QString::number( KRAFT_REQUIRED_SCHEMA_VERSION ) );
      }
    }
  } else {
    kDebug() << "Kraft Schema Version is ok: " << currentVer << endl;
    emit statusMessage( i18n( "Database Schema Version ok" ) );
  }

  return ok;
}

bool KraftDB::createDatabase( QWidget *parent )
{
  // The kraftsystem table is not there, reinit the entire db.
  bool ret = true;
  emit statusMessage( i18n( "Recreate Database" ) );
  if( KMessageBox::warningYesNo( parent,
                                 i18n( "The Kraft System Table was not found in database %1."
                                       " Do you want me to rebuild the database?\n"
                                       "WARNING: ALL YOUR DATA WILL BE DESTROYED!")
                                 .arg(  KatalogSettings::self()->dbFile() ),
                                 i18n("Database Rebuild") ) == KMessageBox::Yes ) {

    emit statusMessage( i18n( "Creating Database..." ) );

    if ( m_db.tables().size() > 0 ) {
      QString allTables = QString( "DROP TABLE %1;" ).arg( m_db.tables().join( ", " ) );
      kDebug() << "Erasing all tables " << allTables << endl;
      QSqlQuery q;
      q.exec( allTables );
    }

    emit statusMessage( i18n( "Parse Commands..." ) );

    SqlCommandList createCommands = parseCommandFile( "create_schema.sql");

    QString dbFill( "fill_schema_en.sql" );

    if ( DefaultProvider::self()->locale()->country() == "de" ) {
      dbFill = "fill_schema_de.sql";
    }
    SqlCommandList fillCommands = parseCommandFile( dbFill );

    int overallCnt = createCommands.count() + fillCommands.count();
    mInitDialog->setOverallCount( overallCnt );

    emit statusMessage( i18n( "Process create commands..." ) );
    mInitDialog->setDetailOverallCnt( createCommands.count() );
    int creates = processSqlCommands( createCommands );
    if( createCommands.count() != creates ) {
      kDebug() << "####### Some create commands failed";
      ret = false;
    }

    if( ret ) {
      emit statusMessage( i18n( "Process fill commands..." ) );
      mInitDialog->setDetailOverallCnt( fillCommands.count() );
      int fills = processSqlCommands( fillCommands );
      if( fillCommands.count() != fills ) {
        kDebug() << "####### Some fill commands failed";
        ret = false;
      }
    }
  }
  return ret;
}

SqlCommandList KraftDB::parseCommandFile( const QString& file )
{
  QString sqlFile;
  QString env( getenv( "KRAFT_DB_FILES" ) );
  if( !env.isEmpty() && env.right(1) != QDir::separator () ) {
    env += QDir::separator ();
  }

  QString driverPrefix = "mysql"; // Default on mysql
  if( mDatabaseDriver.toLower() == "qsqlite") {
    driverPrefix = "sqlite3";
  }

  if( env.isEmpty() ) {
    // Environment-Variable is empty, search in KDE paths
    QString fragment = QString("dbmigrate/%1/%2").arg(driverPrefix).arg(file );
    sqlFile = KStandardDirs::locate("appdata", fragment );
    kDebug() << "Searching for this fragment: " << fragment;
    // search in dbcreate as well.
    if ( sqlFile.isEmpty() ) {
      sqlFile = KStandardDirs::locate( "appdata", QString("dbinit/%1/%2").arg(driverPrefix).arg(file ) );
    }
  } else {
    // read from environment variable path
    QString envPath = QString( "%1/%2/%3").arg(env).arg(driverPrefix).arg(file);
    if( QFile::exists( envPath ) ) {
      sqlFile = envPath;
    } else if( QFile::exists( QString( "%1/%2/migration/%3").arg(env).arg(driverPrefix).arg(file ) ) ){
      sqlFile = QString( "%1/%2/migration/%3").arg(env).arg(driverPrefix).arg(file );
    }
  }

  SqlCommandList retList;

  if ( ! sqlFile.isEmpty() ) {
    kDebug() << "Opening migration file " << sqlFile << endl;

    QFile f( sqlFile );
    if ( !f.open( QIODevice::ReadOnly ) ) {
      kError() << "Could not open " << sqlFile << endl;
    } else {
      QTextStream ts( &f );
      ts.setCodec("UTF-8");

      QString allSql = ts.readAll(); //Not sure of this one!
      QStringList sqlList = allSql.split(";");

      QRegExp reg( "\\s*(#|--)\\s*message:? ?(.*)\\s*\\n" );
      reg.setMinimal( true );

      QListIterator<QString> it(sqlList);

      while( it.hasNext() ) {
        QString msg, command;

        QString sqlFragment = it.next().trimmed();

        int pos = reg.indexIn( sqlFragment.toLower(),  0 );
        if ( pos > -1 ) {
          msg = reg.cap( 2 );
          kDebug() << "SQL-Commands-Parser: Msg: >" << msg << "<" << endl;
        }

        bool clean = false;
        while( ! clean ) {
          if(  sqlFragment.startsWith("#") || sqlFragment.startsWith("--") ) {
            // remove the comment line.
            int newLinePos = sqlFragment.indexOf('\n');
            kDebug() << "Found newline in <" << sqlFragment << ">:" << newLinePos;
            if(newLinePos > 0) {
              sqlFragment = sqlFragment.remove( 0, 1+sqlFragment.indexOf('\n') );
            } else {
              sqlFragment = QString();
            }
            kDebug() << "Left over SQL Fragment:" << sqlFragment;
          } else {
            clean = true;
          }
        }

        if( ! sqlFragment.isEmpty() ) {
          if( sqlFragment.startsWith( "CREATE TRIGGER", Qt::CaseInsensitive )) {
            // Triggers contain a ; which scares the parser. In case of triggers we pull
            // the next item in the list which should be the END; keyword.
            command = sqlFragment + ";";
            if( it.hasNext())
              command += it.next();
          } else {
            // ordinary command, we take it as it is.
            command = sqlFragment;
          }
          if( !( command.isEmpty() && msg.isEmpty() ) ) {
            retList.append( SqlCommand( command, msg ) );
          }
        }
      }
    }
  }
  return retList;
}

int KraftDB::processSqlCommands( const SqlCommandList& commands )
{
  int cnt = 0;

  foreach( SqlCommand cmd, commands ) {
    if( !cmd.message().isEmpty() ) {
      emit statusMessage( cmd.message() );
    }

    if( !cmd.command().isEmpty() ) {
      bool res = true;
      QSqlQuery q;
      q.clear();
      if ( q.exec(cmd.command()) ) {
        kDebug() << "Successfull SQL Command: " << cmd.command() << endl;
        cnt ++;
      } else {
        QSqlError err = q.lastError();
        res = false;
        kDebug() << "###### Failed SQL Command " << cmd.command() << ": " << err.text() << endl;
      }
      q.clear();
      emit processedSqlCommand( res );

    }
  }
  return cnt;
}

// not yet used.
void KraftDB::checkInit()
{
  kDebug() << "** Database init **" << endl;
  if( !( m_db.isValid()) ) {
    kError() << "global db handle is not zero, can not init!" << endl;
  }

        // The database is not yet open. Thus we can move the file away
  QString dbFile = KatalogSettings::self()->dbFile();
  kDebug() << "Database file is " << dbFile << endl;
  if( ! dbFile.isEmpty() ) {
            // backup this file
    // dBFileBackup( dbFile );
  } else {
    QString dbName = KatalogSettings::self()->defaultDbName();
    QString dbPath = KatalogSettings::self()->dbPath();
    if( dbPath.isEmpty() ) {
      KStandardDirs stdDirs;
      dbPath = stdDirs.saveLocation( "data" );
    }

    QString dbFile = dbPath + dbName;
    kDebug() << "Database file: " << dbFile << endl;
    KatalogSettings::self()->setDbFile( dbFile );

  }
}

int KraftDB::currentSchemaVersion()
{
  QSqlQuery query;

  query.exec("SELECT dbschemaversion FROM kraftsystem"); //We'll retrieve every record

  int re = -1;
  if ( query.next() ) {
    re = query.value(0).toInt();
  }
  return re;
}

QString KraftDB::qtDriver()
{
    return mDatabaseDriver;
}

QString KraftDB::mysqlEuroEncode( const QString& str ) const
{
  QChar euro( 0x20ac );
  QString restr( str );
  return restr.replace( euro, EuroTag );
}

QString KraftDB::mysqlEuroDecode( const QString& str ) const
{
  QChar euro( 0x20ac );
  QString restr( str );
  return restr.replace( EuroTag, euro );
}

KraftDB::~KraftDB()
{
}

#include "kraftdb.moc"
