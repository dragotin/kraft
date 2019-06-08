/***************************************************************************
                          einheit.h  -
                             -------------------
    begin                : Don Jan 1 2004
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

#ifndef EINHEIT_H
#define EINHEIT_H
#include <QVector>
#include <QString>

#include "kraftcat_export.h"
/**
  *@author Klaas Freitag
  */

class KRAFTCAT_EXPORT Einheit
{
public:
  typedef QList<Einheit> List;

  Einheit();
  Einheit( int id );
  Einheit( const QString& ); // Einheit with arbitrary text.
  Einheit( int id, const QString&, const QString&, const QString&, const QString& );
  ~Einheit();

  QString einheitSingular() { return m_einheitSingular; }
  QString einheitSingularLong() { return m_einheitSingularLong; }
  QString einheitPlural() { return m_einheitPlural; }
  QString einheitPluralLong() { return m_einheitPluralLong; }

  QString einheit( int anz );
  QString einheit( double anz );

  int     id() { return m_dbId; }

private:
  int m_dbId;
  QString m_einheitSingular;
  QString m_einheitPlural;
  QString m_einheitSingularLong;
  QString m_einheitPluralLong;
};

#endif
