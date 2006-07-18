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
