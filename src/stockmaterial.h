/***************************************************************************
             material  - Material for calculations
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

#ifndef _MATERIAL_H
#define _MATERIAL_H

// include files
#include <QList>

#include <kabc/addressee.h>
#include "kraftglobals.h"
#include "einheit.h"
#include "catalogtemplate.h"
/**
 *
 */
class Einheit;
class MaterialSaverBase;
class QDate;
class QDateTime;

class StockMaterial : public CatalogTemplate
{
public:
  StockMaterial();
  StockMaterial( int dbid, int matChap, QString mat, int unitID,
                 double perPack, Geld pIn, Geld pOut );
  ~StockMaterial();

  QString name() const;
  void setName( const QString& );

  QString description() const;
  void setDescription( const QString& );

  double getAmountPerPack();
  void setAmountPerPack( double am );

  Einheit getUnit();
  void setUnit(const Einheit& );

  int getID();
  void setID( int );

  int chapter() { return m_chapter; }
  void setChapter( int c ) { m_chapter = c; }

  KABC::Addressee getSupplier();
  void setSupplier( KABC::Addressee *supp );

  Geld purchPrice();
  Geld salesPrice();
  Geld unitPrice();

  void setPurchPrice( Geld );
  void setSalesPrice( Geld );

  void setLastModified( QDate );
  void setEnterDate( QDate );
  QString lastModified();
  QString entered();


  bool save();

protected:
  MaterialSaverBase* getSaver();
  QString dateShortFormat( QDate );
  void saveChapterId();

private:
  QString m_name;
  QString m_descr;
  int     m_chapter;

  // per package:
  double  m_amount;
  Einheit m_unit;
  int     m_dbid;

  // FIXME: introduce supplier list
  QString m_delivererUID;
  Geld    m_ePrice;  // price for bying
  Geld    m_vPrice;  // price for selling
  QDate   mLastModified;
  QDate   mEnteredDate;
};


class StockMaterialList : public QList<StockMaterial*>
{
public:
  StockMaterialList() : QList<StockMaterial*>() { }
};

typedef QListIterator<StockMaterial*> StockMaterialListIterator;

#endif

/* END */

