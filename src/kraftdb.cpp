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
#include "databasesettings.h"
#include "defaultprovider.h"

SqlCommand::SqlCommand()
{

}

SqlCommand::SqlCommand(const QString& cmd, const QString& msg, bool mayfail)
    : mSql(cmd),
      mMessage(msg),
      mMayFail(mayfail)
{
    if( !mMessage.isEmpty() && !mMessage.endsWith(';') ) {
        mMessage.append(';');
    }
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

bool SqlCommand::mayfail()
{
    return mMayFail;
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
    // Attention: Before setup assistant rewrite, dbConnect() was called here.
    // Keep that in mind, maybe the auto connect to the DB now misses somewhere.
    // dbConnect();
}

bool KraftDB::dbConnect( const QString& driver, const QString& dbName,
                         const QString& dbUser, const QString& dbHost,
                         const QString& dbPasswd )
{
    mSuccess = true;

    mDatabaseDriver = driver;
    if( driver.isEmpty() ) {
        mDatabaseDriver = DatabaseSettings::self()->dbDriver().toUpper();
    }

    if( mDatabaseDriver.isEmpty() ) {
        kDebug() << "Database Driver is not specified, check katalog settings";
        mSuccess = false;
        return false;
    } else {
        kDebug() << "Using database Driver " << mDatabaseDriver;
    }

    QStringList list = QSqlDatabase::drivers();
    if( list.size() == 0 ) {
        kDebug() << "Database Drivers could not be loaded." << endl;
        mSuccess = false ;
    } else {
        if( list.indexOf( mDatabaseDriver ) == -1 ) {
            kDebug() << "Database Driver " << mDatabaseDriver << " could not be loaded!" << endl;
            mSuccess = false;
        }
    }

    if( mSuccess && m_db.isValid() ) {
        m_db.close();
    }

    if( mSuccess ) {
        m_db = QSqlDatabase::addDatabase( mDatabaseDriver );

        if ( ! m_db.isValid() || m_db.isOpenError() )
        {
            kDebug() <<  "Failed to connect to the database driver: "
                      << m_db.lastError().text() << endl;
            mSuccess = false;
        }
    }

    if ( mSuccess ) {
        int re = 0;
        if(mDatabaseDriver == "QMYSQL") {
            QString host = dbHost;
            if( host.isEmpty() ) host = DatabaseSettings::self()->dbServerName();
            QString name = dbName;
            if( name.isEmpty() ) name = DatabaseSettings::self()->dbDatabaseName();
            QString user = dbUser;
            if( user.isEmpty() ) user = DatabaseSettings::self()->dbUser();
            QString pwd = dbPasswd;
            if( pwd.isEmpty() ) pwd = DatabaseSettings::self()->dbPassword();
            kDebug() << "Try to open MySQL database " << name << endl;
            re = checkConnect( host, name , user, pwd );
        } else if(mDatabaseDriver == "QSQLITE") {
            // SqlLite only requires a valid file name which comes in as Database Name
            QString name = dbName;
            if( name.isEmpty() ) name = DatabaseSettings::self()->dbFile();
            kDebug() << "Try to open SqLite database " << name << endl;
            re = checkConnect( "", name, "", "");
        }
        if ( re == 0 ) {
            // Database successfully opened; we can now issue SQL commands.
            kDebug() << "** Database opened successfully" << endl;
        } else {
            kDebug() << "## Could not open database" << endl;
            mSuccess = false;
        }
    }
    return mSuccess;
}

KraftDB *KraftDB::self()
{
    K_GLOBAL_STATIC(KraftDB, mSelf);
    return mSelf;
}

void KraftDB::close()
{
    QString name = m_db.connectionName();
    kDebug() << "Database connection name to close: " << name;

    m_db.close();
}


int KraftDB::checkConnect( const QString& host, const QString& dbName,
                           const QString& user, const QString& pwd )
{
    // works for both mysql and sqlite if the filename for sqlite comes in
    // as parameter two
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
    if(DatabaseSettings::self()->dbDriver() == "QMYSQL")
        return DatabaseSettings::self()->dbDatabaseName();
    else if(DatabaseSettings::self()->dbDriver() == "QSQLITE")
        return DatabaseSettings::self()->dbFile();

    return "";
}

bool KraftDB::databaseExists()
{
    bool re = false;

    if(m_db.isOpen())
        re = m_db.tables().contains( "kraftsystem");

    return re;
}

void KraftDB::setSchemaVersion( const QString& versionStr )
{
    QSqlQuery q;
    q.prepare( "UPDATE kraftsystem SET dbSchemaVersion=:id" );
    q.bindValue(":id", versionStr );
    q.exec();
}

void KraftDB::wipeDatabase()
{
    emit statusMessage( i18n( "Wipe Database..." ) );
    if ( m_db.tables().size() > 0 ) {
        QString allTables = QString( "DROP TABLE %1;" ).arg( m_db.tables().join( ", " ) );
        kDebug() << "Erasing all tables " << allTables << endl;
        QSqlQuery q;
        q.exec( allTables );
    }
}

SqlCommandList KraftDB::parseCommandFile( const QString& file )
{
    QString sqlFile;
    QString env = QString::fromUtf8( qgetenv( "KRAFT_HOME" ) );
    if( !env.isEmpty() && env.right(1) != QDir::separator () ) {
        env += QDir::separator ();
    }

    QString driverPrefix = "mysql"; // Default on mysql
    if( mDatabaseDriver.toLower() == "qsqlite") {
        driverPrefix = "sqlite3";
    }

    // kDebug() << "XXXXXXXXXX: " << stdDirs.resourceDirs("data");

    if( env.isEmpty() ) {
        // Environment-Variable is empty, search in KDE paths
        QString fragment = QString("kraft/dbmigrate/%1/%2").arg(driverPrefix).arg(file );
        sqlFile = KStandardDirs::locate("data", fragment );
        kDebug() << "Searching for this fragment: " << fragment;
        // search in dbcreate as well.
        if ( sqlFile.isEmpty() ) {
            fragment = QString("kraft/dbinit/%1/%2").arg(driverPrefix).arg(file );
            kDebug() << "Also searching in " << fragment;
            sqlFile = KStandardDirs::locate( "data", fragment );
        }
    } else {
        // read from environment variable path
        QString envPath = QString( "%1/database/%2/%3").arg(env).arg(driverPrefix).arg(file);
        kDebug() << "Environment variable KRAFT_HOME set, searching for DB setup files in " << envPath;
        if( QFile::exists( envPath ) ) {
            sqlFile = envPath;
        } else if( QFile::exists( QString( "%1/database/%2/migration/%3").arg(env).arg(driverPrefix).arg(file ) ) ){
            sqlFile = QString( "%1/database/%2/migration/%3").arg(env).arg(driverPrefix).arg(file );
        }
    }

    SqlCommandList retList;

    if ( ! sqlFile.isEmpty() ) {
        kDebug() << "Opening migration file " << sqlFile << endl;

        QFile f( sqlFile );
        if ( !f.open( QIODevice::ReadOnly ) ) {
            kDebug() << "Could not open " << sqlFile << endl;
        } else {
            QTextStream ts( &f );
            ts.setCodec("UTF-8");

            QString allSql = ts.readAll(); //Not sure of this one!
            QStringList sqlList = allSql.split(";");

            QRegExp reg( "\\s*(#|--)\\s*message:? ?(.*)\\s*\\n" );
            QRegExp failreg( "\\s*(#|--)\\s*mayfail\\s*\\n" );
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

                bool mayfail = false;
                pos = failreg.indexIn( sqlFragment.toLower(), 0 );
                if( pos > -1 ) {
                    mayfail = true;
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
                        retList.append( SqlCommand( command, msg, mayfail ) );
                    }
                }
            }
        }
    } else {
        kDebug() << "ERR: Can not find sql file " << file;
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
            res = q.exec(cmd.command()) || cmd.mayfail();

            if ( res ) {
                kDebug() << "Successful SQL Command: " << cmd.command() << endl;
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

int KraftDB::requiredSchemaVersion()
{
    return KRAFT_REQUIRED_SCHEMA_VERSION;
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

QString KraftDB::currentTimeStamp( const QDateTime& dt )
{
    QString dateStr;
    if( dt.isValid() ) {
        dateStr = dt.toString(Qt::ISODate);
    } else {
        dateStr = QDateTime::currentDateTime().toString(Qt::ISODate);
    }
    return dateStr;
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
    re.sort();
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


KraftDB::~KraftDB()
{
}
