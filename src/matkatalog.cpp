/***************************************************************************
             matkatalog  -
                             -------------------
    begin                : 2004-19-10
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

// include files for Qt
#include <qsql.h>
#include <qsqlcursor.h>

// include files for KDE
#include <klocale.h>
#include <kdebug.h>

#include "matkatalog.h"
#include "kraftdb.h"

MatKatalog::MatKatalog( const QString& name)
    : Katalog(name)
{

}

MatKatalog::MatKatalog()
    : Katalog()
{

}


int MatKatalog::load( const QString&  )
{
    int cnt = 0;
#if 0
    QSqlCursor cur("matKats");
    cur.setMode( QSqlCursor::ReadOnly );
    cur.select( "matKatName='"+name+"'");

    while ( cur.next() ) {
        m_katalogID = cur.value("matKatID").toInt();
    }
#endif
    return cnt;
}

int MatKatalog::load()
{
    return 0;
}

MatKatalog::~MatKatalog( )
{

}

/* END */

