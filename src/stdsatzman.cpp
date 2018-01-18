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
#include <QStringList>
#include <QString>
#include <QSqlQuery>
#include <QDebug>
#include <QGlobalStatic>

// include files for KDE

#include "stdsatzman.h"
#include "kraftdb.h"

Q_GLOBAL_STATIC(StdSatzMan, mSelf)

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

StdSatzMan *StdSatzMan::self()
{
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

    StdSatzVector::iterator it;
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
    StdSatzVector::iterator it;
    for( it = mStdSaetze.begin(); it != mStdSaetze.end(); ++it )
    {
        if( (*it).getName() == name ) return (*it);
    }
    return StdSatz();
}

StdSatz StdSatzMan::getStdSatz( dbID id )
{
    load();
    StdSatzVector::iterator it;
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
  // qDebug () << "Groesse fuer Stundensatzliste: " << max << endl;

  mStdSaetze.resize( max );


  /* Daten laden */
  q.prepare("SELECT stdSaetzeID, name, price FROM stdSaetze ORDER BY sortKey");
  q.exec();
  while( q.next() )
  {
    int satzID = q.value(0).toInt();
    // qDebug () << "Neue StdSatz ID " << satzID << endl;
    // resize if index is to big.
    StdSatz ss( satzID, q.value(1).toString(),
                Geld( q.value(2).toDouble()));
    mStdSaetze.append(ss);
  }
}


/* END */

