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
#include <QDebug>
#include <kcontacts/addressee.h>

#include "stockmaterial.h"
#include "unitmanager.h"
#include "materialsaverbase.h"
#include "materialsaverdb.h"
#include "defaultprovider.h"
#include "format.h"
#include "kraftsettings.h"

StockMaterial::StockMaterial( ):
    CatalogTemplate(),
  m_amount( 0 ),
  m_dbid( -1 )
{

}

StockMaterial::StockMaterial( int dbid, int matChap, QString mat, int unitID,
                              double perPack, Geld pIn, Geld pOut ):
    CatalogTemplate(),
    m_chapter(matChap),
    m_amount(perPack),
    m_dbid(dbid),
    m_ePrice(pIn),
    m_vPrice(pOut)
{
    this->setUnitId(unitID);
    this->setText(mat);
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

#if 0
QString StockMaterial::description() const
{
    return m_descr;
}

void StockMaterial::setDescription( const QString& str )
{
  m_descr = str;
}
#endif

double StockMaterial::getAmountPerPack()
{
    return m_amount;
}

void StockMaterial::setAmountPerPack( double am )
{
    m_amount = am;
}

int StockMaterial::getID()
{
    return m_dbid;
}

void StockMaterial::setID( int id )
{
    m_dbid = id;
}

KContacts::Addressee StockMaterial::getSupplier()
{
    KContacts::Addressee a;
    return a;
}

void StockMaterial::setSupplier( KContacts::Addressee *a )
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
  return salesPrice() / m_amount;
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
  return Format::toDateString( mLastModified, KraftSettings::self()->dateFormat());
}

QString StockMaterial::entered()
{
  return Format::toDateString( mEnteredDate, KraftSettings::self()->dateFormat() );
}

void StockMaterial::saveChapterId()
{
  MaterialSaverBase *saver = getSaver();
  if( saver ) {
    saver->saveTemplateChapter( this );
  }
}


/* END */

