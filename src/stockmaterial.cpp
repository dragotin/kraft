/***************************************************************************
             material  -
                             -------------------
    begin                : 2004-05-05
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
#include <QString>
#include <QDate>

// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <kabc/addressee.h>

#include "stockmaterial.h"
#include "unitmanager.h"
#include "materialsaverbase.h"
#include "materialsaverdb.h"

StockMaterial::StockMaterial( ):
    CatalogTemplate(),
  m_amount( 0 ),
  m_dbid( -1 )
{

}

StockMaterial::StockMaterial( int dbid, int matChap, QString mat, int unitID,
                              double perPack, Geld pIn, Geld pOut ):
CatalogTemplate(),
    m_name(mat),
    m_chapter(matChap),
    m_amount(perPack),
    m_dbid(dbid),
    m_ePrice(pIn),
    m_vPrice(pOut)
{
    m_unit = UnitManager::self()->getUnit( unitID );
}

StockMaterial::~StockMaterial( )
{

}

MaterialSaverBase* StockMaterial::getSaver()
{
  return MaterialSaverDB::self();
}

bool StockMaterial::save()
{
  MaterialSaverBase *saver = getSaver();
  if ( saver ) {
    saver->saveTemplate( this );
    return true;
  }
  return false;
}

QString StockMaterial::name() const
{
    return m_name;
}

void StockMaterial::setName( const QString& str )
{
  m_name = str;
}

QString StockMaterial::description() const
{
    return m_descr;
}

void StockMaterial::setDescription( const QString& str )
{
  m_descr = str;
}

double StockMaterial::getAmountPerPack()
{
    return m_amount;
}

void StockMaterial::setAmountPerPack( double am )
{
    m_amount = am;
}

Einheit StockMaterial::getUnit( )
{
    return m_unit;
}

void StockMaterial::setUnit( const Einheit& e )
{
    m_unit = e;
}

int StockMaterial::getID()
{
    return m_dbid;
}

void StockMaterial::setID( int id )
{
    m_dbid = id;
}

KABC::Addressee StockMaterial::getSupplier()
{
    KABC::Addressee a;
    return a;
}

void StockMaterial::setSupplier( KABC::Addressee *a )
{
    if( a )
        m_delivererUID = a->uid();
}

Geld StockMaterial::purchPrice()
{
    return m_ePrice;
}

Geld StockMaterial::salesPrice()
{
    return m_vPrice;
}

Geld StockMaterial::unitPrice()
{
  return salesPrice();
}

void StockMaterial::setPurchPrice( Geld g )
{
    m_ePrice = g;
}

void StockMaterial::setSalesPrice( Geld g )
{
    m_vPrice = g;
}

void StockMaterial::setLastModified( QDate dt )
{
  mLastModified = dt;
}

void StockMaterial::setEnterDate( QDate dt )
{
  mEnteredDate = dt;
}

QString StockMaterial::lastModified()
{
  return dateShortFormat( mLastModified );
}

QString StockMaterial::entered()
{
  return dateShortFormat( mEnteredDate );
}


QString StockMaterial::dateShortFormat( QDate d )
{
  // return d.toString();
  return QString( "%1/%2" ).arg( d.month() ).arg( d.year() );
}

void StockMaterial::saveChapterId()
{
  MaterialSaverBase *saver = getSaver();
  if( saver ) {
    saver->saveTemplateChapter( this );
  }
}


/* END */

