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
  Einheit( int id, const QString&, const QString&, const QString&, const QString&, const QString& );
  ~Einheit();

  QString einheitSingular() const { return m_einheitSingular; }
  QString einheitSingularLong() const { return m_einheitSingularLong; }
  QString einheitPlural() const { return m_einheitPlural; }
  QString einheitPluralLong() const { return m_einheitPluralLong; }
  QString ec20() const { return m_ec20; }

  QString einheit( int anz ) const;
  QString einheit( double anz ) const;


  int     id() { return m_dbId; }

private:
  int m_dbId;
  QString m_einheitSingular;
  QString m_einheitPlural;
  QString m_einheitSingularLong;
  QString m_einheitPluralLong;
  QString m_ec20;
};

#endif
