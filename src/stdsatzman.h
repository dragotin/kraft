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

#ifndef _STDSATZMAN_H
#define _STDSATZMAN_H

/*
 * Stundensatzverwaltung: Es gibt Stundensätze mit verschiedenen Namen wie
 * Meister, Geselle, Helfer. Hinter jedem Namen steht ein gewisser Wert, der
 * per Dokument angepasst werden kann.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// include files
#include <QVector>

#include "geld.h"
#include "dbids.h"


class QString;

/**
 * das Stundensatzobjekt: definiert durch id und namen
 */
class StdSatz
{
public:
    StdSatz();
    /**
     * Konstruktur basierend auf der Datenbank id
     */
    StdSatz(int id);

    StdSatz( int id, const QString& name, Geld g );

    dbID getId()      { return m_dbId; }
    QString getName() { return m_name; }
    Geld getPreis()   { return m_value; }


private:
    dbID      m_dbId;
    QString   m_name;
    Geld      m_value;

};

class StdSatzDuration : public StdSatz
{
public:
  StdSatzDuration();
  StdSatzDuration( const StdSatz&, int );

  int duration() {
    return mDuration;
  }

  void setDuration( int d ) {
    mDuration = d;
  }
private:
  int mDuration;
};


typedef QVector<StdSatz> StdSatzVector;

/**
 * der Stundensatzmanager
 */

class StdSatzMan
{
public:
  ~StdSatzMan();
  static StdSatzMan *self();

  QStringList allStdSaetze();
  StdSatz     getStdSatz( const QString& name );
  StdSatz     getStdSatz( dbID id );
private:
  StdSatzMan();
  void load();

  StdSatzVector mStdSaetze;
  static StdSatzMan *mSelf;
};

#endif

/* END */

