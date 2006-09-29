/***************************************************************************
                  ArchDoc.cpp  - an archived document.
                             -------------------
    begin                : Mit Dez 31 19:24:05 CET 2003
    copyright            : (C) 2003 by Klaas Freitag
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
#include <qdir.h>

// include files for KDE
#include <klocale.h>

// application specific includes
#include "archdoc.h"
#include "docposition.h"
#include "archdocposition.h"
#include "geld.h"

ArchDoc::ArchDoc()
{

}

ArchDoc::ArchDoc( const dbID& id )
{
  /* load archive from database */
}

ArchDoc::~ArchDoc()
{
}


QString ArchDoc::docIdentifier()
{
  QString re = docType();

  return i18n("%1 for %2 (Id %3)").arg( docType() ).arg( ident() );
}

Geld ArchDoc::nettoSum()
{
  return positions().sumPrice();
}

Geld ArchDoc::bruttoSum()
{
  Geld g = nettoSum();
  g += vatSum();
  return g;
}

Geld ArchDoc::vatSum()
{
  return Geld( nettoSum() * vat()/100.0 );
}

double ArchDoc::vat()
{
  return 16.0;
}

