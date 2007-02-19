/***************************************************************************
             stdsatzman  -
                             -------------------
    begin                : 2004-13-09
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
#include <qstringlist.h>
#include <qstring.h>
// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <kstaticdeleter.h>

#include "stdsatzman.h"
#include "kraftdb.h"
#include <qsqlquery.h>
#include <qsqlcursor.h>

StdSatz::StdSatz():
    m_dbId(0)
{

}

StdSatz::StdSatz( int id ):
    m_dbId(id)
{

}

StdSatz::StdSatz( int id, const QString& name, Geld g ):
    m_dbId(id),
    m_name(name),
    m_value(g)
{

}

/*
 * ********** Stundensatz Duration **********
 */
StdSatzDuration::StdSatzDuration()
  :mDuration( -1 )
{
}

StdSatzDuration::StdSatzDuration( const StdSatz& std, int dur )
  :StdSatz( std ),
   mDuration( dur )
{
}


/*
 * ********** Stundensatz Manager **********
 */
static KStaticDeleter<StdSatzMan> selfDeleter;
StdSatzMan* StdSatzMan::mSelf = 0;

StdSatzMan *StdSatzMan::self()
{
  if ( !mSelf ) {
    selfDeleter.setObject( mSelf, new StdSatzMan() );
  }
  return mSelf;
}

StdSatzMan::StdSatzMan( )
{
    load();
}

QStringList StdSatzMan::allStdSaetze()
{
    QStringList list;
    load();

    StdSatzValueVector::iterator it;
    for( it = mStdSaetze.begin(); it != mStdSaetze.end(); ++it )
    {
        QString n = (*it).getName();
        if( !n.isEmpty())
            list << n;
    }
    return list;

}

StdSatz  StdSatzMan::getStdSatz( const QString& name )
{
    load();
    StdSatzValueVector::iterator it;
    for( it = mStdSaetze.begin(); it != mStdSaetze.end(); ++it )
    {
        if( (*it).getName() == name ) return (*it);
    }
    return StdSatz();
}

StdSatz StdSatzMan::getStdSatz( dbID id )
{
    load();
    StdSatzValueVector::iterator it;
    for( it = mStdSaetze.begin(); it != mStdSaetze.end(); ++it )
    {
        dbID dbid = (*it).getId();
        if( dbid == id ) return (*it);
    }
    return StdSatz();
}

StdSatzMan::~StdSatzMan( )
{

}

void StdSatzMan::load()
{
  /* noetige Groesse rausfinden */
  int max = -1;

  QSqlQuery q("SELECT count(*) from stdSaetze;");
  if( q.isActive())
  {
    q.next();
    max = q.value(0).toInt();
  }
  kdDebug() << "Groesse fuer Stundensatzliste: " << max << endl;

  mStdSaetze.resize( max );


  /* Daten laden */
  QSqlCursor cur("stdSaetze");
  cur.setMode( QSqlCursor::ReadOnly );

    // Create an index that sorts from high values for einheitID down.
    // that makes at least on resize of the vector.
    QSqlIndex indx = cur.index( "sortKey" );
    // indx.setDescending ( 0, true );

    cur.select( indx );
    while( cur.next() )
    {
      int satzID = cur.value("stdSaetzeID").toInt();
      kdDebug() << "Neue StdSatz ID " << satzID << endl;
      // resize if index is to big.
      StdSatz ss( satzID, QString::fromUtf8(cur.value("name").toCString()),
                  Geld( cur.value("price").toDouble()));

      mStdSaetze.append(ss);
    }
}


/* END */

