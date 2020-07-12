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
#include <QDebug>

#include <QFile>
#include <QSqlQuery>
#include <QStringList>
#include <QRegExp>
#include <QTextStream>
#include <QSqlError>
#include <QDir>
#include <QDebug>
#include <QDomDocument>
#include <QDomElement>

#include "version.h"
#include "kraftdb.h"
#include "doctype.h"
#include "dbids.h"
#include "defaultprovider.h"

Q_GLOBAL_STATIC(KraftDB, mSelf)


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
    :QList<SqlCommand>(),
      _number(0)
{

}

void SqlCommandList::setNumber(int no)
{
    _number = no;
}

int SqlCommandList::number()
{
    return _number;
}

void SqlCommandList::setMetaAddDocTypeList( QList<MetaDocTypeAdd> list )
{
    _docTypeMetaList = list;
}

QList<MetaDocTypeAdd> SqlCommandList::metaAddDocTypeList() const
{
    return _docTypeMetaList;
}
// ==========================================================================

KraftDB::KraftDB()
    :QObject (), mParent(nullptr),
      mSuccess( true ),
      EuroTag( QString::fromLatin1( "%EURO" ) ),
      mInitDialog(nullptr)
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
    mDatabaseName = dbName;

    if( mDatabaseDriver.isEmpty() ) {
        // qDebug () << "Database Driver is not specified, check katalog settings";
        mSuccess = false;
        return false;
    } else {
        // qDebug () << "Using database Driver " << mDatabaseDriver;
    }

    QStringList list = QSqlDatabase::drivers();
    if( list.size() == 0 ) {
        // qDebug () << "Database Drivers could not be loaded." << endl;
        mSuccess = false ;
    } else {
        if( list.indexOf( mDatabaseDriver ) == -1 ) {
            // qDebug () << "Database Driver " << mDatabaseDriver << " could not be loaded!" << endl;
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
            qDebug() <<  "Failed to connect to the database driver: "
                      << m_db.lastError().text() << endl;
            mSuccess = false;
        }
    }

    if ( mSuccess ) {
        int re = 0;
        if(mDatabaseDriver == "QMYSQL") {
            int port = -1; // use the default port so far
            // FIXME: get port from user interface
            // qDebug () << "Try to open MySQL database " << name << endl;
            re = checkConnect( dbHost, dbName , dbUser, dbPasswd, port);
        } else if(mDatabaseDriver == "QSQLITE") {
            // SqlLite only requires a valid file name which comes in as Database Name
            // qDebug () << "Try to open SqLite database " << name << endl;
            re = checkConnect( QString(), dbName, QString(), QString(), -1);
        }
        if ( re == 0 ) {
            // Database successfully opened; we can now issue SQL commands.
            // qDebug () << "** Database opened successfully" << endl;
        } else {
            // qDebug () << "## Could not open database" << endl;
            mSuccess = false;
        }
    }
    return mSuccess;
}

KraftDB *KraftDB::self()
{
    return mSelf;
}

void KraftDB::close()
{
    QString name = m_db.connectionName();
    // qDebug () << "Database connection name to close: " << name;

    m_db.close();
}

bool KraftDB::isSqlite()
{
    const QString dbDriver = qtDriver().toUpper();

    return (dbDriver.startsWith("QSQLITE"));
}

int KraftDB::checkConnect( const QString& host, const QString& dbName,
                           const QString& user, const QString& pwd, int port )
{
    mDatabaseName = dbName;
    // works for both mysql and sqlite if the filename for sqlite comes in
    // as parameter two
    if ( dbName.isEmpty() || !(m_db.isValid()) ) return false;
    m_db.setHostName( host );
    m_db.setDatabaseName( dbName );
    m_db.setUserName( user );
    m_db.setPassword( pwd );
    if( port > -1 ) {
        m_db.setPort(port);
    }
    int re = 0;

    m_db.open();
    if ( m_db.isOpenError() ) {
         qDebug () << "ERR opening the db: " << m_db.lastError().text() <<
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
        // qDebug () << "############# FATAL ERROR: Unknown database driver " << mDatabaseDriver;
    }
    int id = -1;

    if( query.next() ) {
        id = query.value(0).toInt();
    } else {
        // qDebug () << "############# FATAL ERROR: Query for last insert id is invalid!";
    }
    // qDebug () << "Last Insert ID: " << id;
    return dbID(id);
}

QString KraftDB::databaseName() const
{
    return mDatabaseName;
}

bool KraftDB::databaseExists()
{
    bool re = false;

    if(!m_db.isOpen()) {
        m_db.open();
    }
    if(m_db.isOpen()) {
        const QStringList t = m_db.tables();
        re = t.contains( "kraftsystem");
    }
    return re;
}

void KraftDB::setSchemaVersion( const QString& versionStr )
{
    QSqlQuery q;
    q.prepare( "UPDATE kraftsystem SET dbSchemaVersion=:id" );
    q.bindValue(":id", versionStr );
    q.exec();
}

SqlCommandList KraftDB::parseCommandFile( int currentVersion )
{
    SqlCommandList list;
    const QString& file = QString("%1_dbmigrate.sql").arg(currentVersion);
    list = parseCommandFile(file);
    list.setMetaAddDocTypeList( parseMetaFile(currentVersion) );
    list.setNumber(currentVersion);
    return list;
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

    // qDebug() << "XXXXXXXXXX: " << stdDirs.resourceDirs("data");

    if( env.isEmpty() ) {
        // Environment-Variable is empty, search in KDE paths
        QString fragment = QString("kraft/dbmigrate/%1/%2").arg(driverPrefix).arg(file );
        sqlFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, fragment );
        // qDebug () << "Searching for this fragment: " << fragment;
        // search in dbcreate as well.
        if ( sqlFile.isEmpty() ) {
            fragment = QString("kraft/dbinit/%1/%2").arg(driverPrefix).arg(file );
            // qDebug () << "Also searching in " << fragment;
            sqlFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, fragment );
        }
    } else {
        // read from environment variable path
        QString envPath = QString( "%1/database/%2/%3").arg(env).arg(driverPrefix).arg(file);
        // qDebug () << "Environment variable KRAFT_HOME set, searching for DB setup files in " << envPath;
        if( QFile::exists( envPath ) ) {
            sqlFile = envPath;
        } else if( QFile::exists( QString( "%1/database/%2/migration/%3").arg(env).arg(driverPrefix).arg(file ) ) ){
            sqlFile = QString( "%1/database/%2/migration/%3").arg(env).arg(driverPrefix).arg(file );
        }
    }

    SqlCommandList retList;

    if ( ! sqlFile.isEmpty() ) {
        // qDebug () << "Opening migration file " << sqlFile << endl;

        QFile f( sqlFile );
        if ( !f.exists() ) {
            qDebug() << "FATAL: File" << sqlFile << "does not exist!";
        }
        if ( !f.open( QIODevice::ReadOnly ) ) {
            qDebug () << "FATAL: Could not open " << sqlFile << endl;
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
                    // qDebug() << "SQL-Commands-Parser: Msg: >" << msg << "<" << endl;
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
                        // qDebug() << "Found newline in <" << sqlFragment << ">:" << newLinePos;
                        if(newLinePos > 0) {
                            sqlFragment = sqlFragment.remove( 0, 1+sqlFragment.indexOf('\n') );
                        } else {
                            sqlFragment = QString();
                        }
                        // qDebug() << "Left over SQL Fragment:" << sqlFragment;
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
                    if( !command.isEmpty() ) {
                        retList.append( SqlCommand( command, msg, mayfail ) );
                    }
                }
            }
        }
    } else {
        qDebug () << "ERR: Can not find sql file " << file;
    }



    return retList;
}

QList<MetaDocTypeAdd> KraftDB::parseMetaFile( int currentVersion )
{
    const QString fileName = QString("%1_meta.xml").arg(currentVersion);

    QString env = QString::fromUtf8( qgetenv( "KRAFT_HOME" ) );
    if( !env.isEmpty() && env.right(1) != QDir::separator () ) {
        env += QDir::separator ();
    }
    QString xmlFile;
    if( !env.isEmpty() ) {
        xmlFile = env + QLatin1String("database/meta/") + fileName;
    } else {
        const QString fragment = QString("kraft/meta/") + fileName;
        xmlFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, fragment );
    }
    QFile f( xmlFile );
    MetaXMLParser parser;
    if( f.exists() ) {
        if ( !f.open( QIODevice::ReadOnly ) ) {
            qDebug () << "FATAL: Could not open " << xmlFile << endl;
        } else {
            QTextStream ts( &f );
            ts.setCodec("UTF-8");
            parser.parse( &f );
        }
    } else {
        qDebug() << "XML Metafile" << xmlFile << "does not exist!";
    }

    return parser.metaDocTypeAddList();
}

int KraftDB::processSqlCommands( const SqlCommandList& commands )
{
    int cnt = 0;

    // first do the doctype definitions
    QList<MetaDocTypeAdd> newDocTypes = commands.metaAddDocTypeList();

    // loop over all doctypes first, later loop again to create the followers.
    // The followers might reference each other and thus must exist.
    for( auto newDocType : newDocTypes ) {
       const QString name = newDocType.name();
       DocType type(name, true);

       for( QString attr : newDocType._attribs.keys() ) {
           type.setAttribute(attr, newDocType._attribs[attr]);
       }
       type.save();
    }

    // now loop again to process the followers
    for( auto newDocType : newDocTypes ) {
        const QString name = newDocType.name();
        if( newDocType._follower.count() > 0 ) {
            DocType type(name, true);
            type.setAllFollowers(newDocType._follower);
            type.save();
        }
    }

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
                // qDebug () << "Successful SQL Command: " << cmd.command() << endl;
                cnt ++;
            } else {
                QSqlError err = q.lastError();
                res = false;
                qDebug () << "###### Failed SQL Command " << cmd.command() << ": " << err.text() << endl;
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
        // qDebug () << "PP: " << keys;
        for ( QStringList::Iterator dtIt = keys.begin(); dtIt != keys.end(); ++dtIt ) {
            QString repKey = *dtIt;
            re.replace( repKey, replaceMap[repKey] );
        }
    }

    // qDebug () << "Adding to wordlist <" << re << ">";

    return re;
}

void KraftDB::writeWordList( const QString& listName, const QStringList& list )
{
    // qDebug () << "Saving " << list[0] << " into list " << listName << endl;
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

bool KraftDB::checkTableExistsSqlite(const QString& name, const QStringList& lookupCols)
{
    const QString query = QLatin1Literal("PRAGMA table_info(")+name+(")");
    QSqlQuery q(query);
    QStringList cols = lookupCols;

    q.exec();
    QSqlError err = q.lastError();
    if( err.isValid() ) {
        qDebug() << "Error: " << err.text();
    }

    while( q.next() ) {
        const QString colName = q.value(1).toString();
        qDebug() << "checking colum" << colName;
        cols.removeAll(colName);
    }
    return cols.isEmpty();
}

KraftDB::~KraftDB()
{
}

