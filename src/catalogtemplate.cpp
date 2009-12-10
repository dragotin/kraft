/***************************************************************************
      catalogtemplate - template base class for catalog data
                             -------------------
    begin                : Oct 2007
    copyright            : (C) 2007 by Klaas Freitag
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
#include <klocale.h>

#include "catalogtemplate.h"

CatalogTemplate::CatalogTemplate()
  : m_calcType( Calculation )
{

}

CatalogTemplate::~CatalogTemplate()
{

}

CatalogTemplate::CalculationType CatalogTemplate::calcKind()
{
  return m_calcType;
}

void CatalogTemplate::setCalculationType( CalculationType t )
{
  m_calcType = t;
}

QString CatalogTemplate::calcKindString() const
{
  if( m_calcType == ManualPrice )
    return i18n("Manual Price");
  else if( m_calcType == Calculation )
    return i18n("Calculated");
  else if( m_calcType == AutoCalc )
    return i18n("AutoCalc");
  else return i18n( "Err: Unknown type %d").arg(m_calcType);
}


// ================================================================================

CatalogTemplateList::CatalogTemplateList()
 :QList<CatalogTemplate*>()
{

}

CatalogTemplateList::~CatalogTemplateList()
{

}

int CatalogTemplateList::compareItems( CatalogTemplate *ct1,  CatalogTemplate *ct2 )
{
  // CatalogTemplate* ct1 = static_cast<CatalogTemplate*>( i1 );
  // CatalogTemplate* ct2 = static_cast<CatalogTemplate*>( i2 );

  kDebug() << "********************************* In Sort!" << endl;

  if ( !( ct1 && ct2 ) ) return 0;

  int sortKey1 = ct1->sortKey();
  int sortKey2 = ct2->sortKey();

  if ( sortKey1 == sortKey2 ) return 0;
  if ( sortKey1 < sortKey2 ) return -1;
  return 1;

}
