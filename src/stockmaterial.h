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

#include <kcontacts/addressee.h>
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

#if 0
  // currently not used.
  QString description() const;
  void setDescription( const QString& );
#endif

  double getAmountPerPack();
  void setAmountPerPack( double am );

  int getID();
  void setID( int );

  int chapter() { return m_chapter; }
  void setChapter( int c ) { m_chapter = c; }

  KContacts::Addressee getSupplier();
  void setSupplier( KContacts::Addressee *supp );

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
  void saveChapterId();

private:
  // QString m_descr;
  int     m_chapter;

  // per package:
  double  m_amount;
  int     m_unit;
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

