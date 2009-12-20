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
#include <kpushbutton.h>

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

KraftDB::KraftDB()
  :QObject (), mParent(0),
  mSuccess( true ),
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
  K_GLOBAL_STATIC(KraftDB, mSelf);
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

void KraftDB::createInitDialog( )
{
  if( mInitDialog ) return;
  mInitDialog = new DbInitDialog( mParent );
  mInitDialog->setModal( true );

  connect( this, SIGNAL(statusMessage( const QString& )),
           mInitDialog, SLOT( slotSetStatusText( const QString& )));
  connect( this, SIGNAL(processedSqlCommand( bool )),
           mInitDialog, SLOT( slotProcessedOneCommand( bool )));
}

void KraftDB::checkDatabaseSetup( QWidget *parent )
{
 bool reinit = false;
 mParent = parent;
 // check if the kraftsystems table exists, otherwise recreate everything
  if ( m_db.tables().contains( "kraftsystem" ) == 0 ) {
    reinit = true;
    createInitDialog();
    connect( mInitDialog, SIGNAL(user1Clicked()), this, SLOT(slotCreateDatabase()));

    emit statusMessage( i18n( "Recreate Database" ) );

    mInitDialog->slotSetInstructionText( i18n( "<p>The database <br/><tt>%1</tt><br/> is either empty or broken."
                                         "To setup the database from scratch press the Start button!</p>"
                                         "<p><b>WARNING:</b> ALL YOUR KRAFT DATA WILL BE DESTROYED!</p>")
                                         .arg( KatalogSettings::self()->dbFile() ) );
    mInitDialog->show();
   }

  if( ! reinit ) {
    // in case of reinit we wait until the user clicks slotStartSchemaUpdate ok
    checkSchemaVersion();
  }
}

void KraftDB::checkSchemaVersion( )
{
  kDebug() << "The country setting is " << DefaultProvider::self()->locale()->country() << endl;

  emit statusMessage( i18n( "Checking Database Schema Version" ) );

  int currentVer = currentSchemaVersion();

  bool interactive = (mInitDialog == 0); // if the dialog already exists, we do not wait for click on 'Start'

  if ( currentVer < KRAFT_REQUIRED_SCHEMA_VERSION ) {

    kDebug() << "Kraft Schema Version not sufficient: " << currentVer << endl;
    if( interactive ) {
      createInitDialog( );
      connect( mInitDialog, SIGNAL(user1Clicked()), this, SLOT(slotStartSchemaUpdate()));

      mInitDialog->show();
    }
    emit statusMessage( i18n( "Database schema not up to date" ) );
    mInitDialog->slotSetInstructionText( i18n( "This Kraft database schema is not up to date "
                                               "(it is version %1 instead of the required version %2).\n"
                                               "Kraft is able to update it to the new version automatically.\n"
                                               "WARNING: MAKE SURE A GOOD BACKUP IS AVAILABLE!\n"
                                               "Do you want Kraft to update the database schema version now?")
                                         .arg(  currentVer ).arg( KRAFT_REQUIRED_SCHEMA_VERSION ) );

    if( !interactive ) {
      slotStartSchemaUpdate();
    }
  } else {
    kDebug() << "Database Version is sufficient!";
  }
}


void KraftDB::slotStartSchemaUpdate()
{
  int overallCmdCount = 0;
  QList<SqlCommandList> commandLists;
  int currentVer = currentSchemaVersion();
  if( currentVer == -1 ) currentVer = 1; // set to initial version

  while ( currentVer < KRAFT_REQUIRED_SCHEMA_VERSION ) {
    ++currentVer;
    const QString migrateFilename = QString( "%1_dbmigrate.sql" ).arg( currentVer );
    kDebug() << "######### Reading " << migrateFilename;
    SqlCommandList cmds = parseCommandFile( migrateFilename );
    overallCmdCount += cmds.count();
    commandLists << cmds;
  }
  mInitDialog->setOverallCount( overallCmdCount );

  int doneOverallCmds =  0;
  foreach( SqlCommandList cmds, commandLists ) {
    mInitDialog->setDetailOverallCnt( cmds.count() );
    int goodCmds = processSqlCommands( cmds );
    doneOverallCmds += goodCmds;
    if( goodCmds != cmds.count() ) {
      kDebug() << "Only performned " << goodCmds << " out of " << cmds.count();
    } else {
      kDebug() << "Performed well!";
    }
  }

  if( doneOverallCmds != overallCmdCount ) {
    kDebug() << "WRN: only " << doneOverallCmds << " from " << overallCmdCount << " sql commands "
        "were executed correctly" << endl;

    mInitDialog->slotSetInstructionText( i18n( "The update of the database failed, only "
                                               "%1 of %2 commands succeeded. It is not "
                                               "recommended to continue.\nPlease check the "
                                               "database consistency.\n" ).arg( doneOverallCmds ).arg( overallCmdCount ));
  } else {
    kDebug() << "All sql commands successfully performned";
    /* Now update to the required schema version */
    QSqlQuery q;
    q.prepare( "UPDATE kraftsystem SET dbSchemaVersion=:id" );
    q.bindValue(":id", QString::number( KRAFT_REQUIRED_SCHEMA_VERSION ) );
    q.exec();
    mInitDialog->button( KDialog::User1 )->setEnabled(false);
  }
}


void KraftDB::slotCreateDatabase()
{
  emit statusMessage( i18n( "Creating Database..." ) );
  bool ret = true;
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
  if( ret ) {
    checkSchemaVersion();
  }
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
          // kDebug() << "SQL-Commands-Parser: Msg: >" << msg << "<" << endl;
        }

        bool clean = false;
        while( ! clean ) {
          if(  sqlFragment.startsWith("#") || sqlFragment.startsWith("--") ) {
            // remove the comment line.
            int newLinePos = sqlFragment.indexOf('\n');
            // kDebug() << "Found newline in <" << sqlFragment << ">:" << newLinePos;
            if(newLinePos > 0) {
              sqlFragment = sqlFragment.remove( 0, 1+sqlFragment.indexOf('\n') );
            } else {
              sqlFragment = QString();
            }
            // kDebug() << "Left over SQL Fragment:" << sqlFragment;
          } else {
            clean = true;
          }
        }

        if( !sqlFragment.isEmpty() ) {

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
