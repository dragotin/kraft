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

class dbID;
class KProcess;

class KraftDB {

public:
    KraftDB();
    ~KraftDB();
    /** Read property of QSqlDatabase* m_db. */
    static QSqlDatabase* getDB();

    static double getMwSt(){
        return MWST;
    }

    static dbID getLastInsertID();

    static void checkInit();
    static void checkSchemaVersion();

    static QString qtDriver();

    typedef QMap<QString, QString> StringMap;
    static QStringList wordList( const QString&, StringMap replaceMap = StringMap() );

private: // Private attributes
    static void dBFileBackup( const QString& );
    static bool doInitialSetup( );
    static bool sendFileToDb( const QString& filename );
    /** The default database */
    static QSqlDatabase* m_db;
    static KProcess *mProcess;
};

#endif
