/***************************************************************************
                      matkatalog  - the material catalog
                             -------------------
    begin                : 2004-19-10
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
#include <QSqlQuery>

// include files for KDE
#include <QDebug>

#include "matkatalog.h"
#include "kraftdb.h"

MatKatalog::MatKatalog( const QString& name)
    : Katalog(name)
{

}

MatKatalog::MatKatalog()
    : Katalog( QLatin1String( "Material" ))
{

}

void MatKatalog::reload( dbID )
{
  mAllMaterial.clear();
  load();
}

int MatKatalog::load()
{
  Katalog::load();
  int cnt = 0;

  QSqlQuery q(QLatin1String("SELECT matID, chapterID, material, unitID, perPack, priceIn, "
              "priceOut, modifyDate, enterDate FROM stockMaterial"));
  q.exec();
  while ( q.next() ) {
    cnt++;
    int id = q.value( 0 ).toInt();
    int chapterID = q.value( 1 ).toInt();
    const QString material = q.value( 2 ).toString();
    int unitID = q.value( 3 ).toInt();
    double pPack = q.value( 4 ).toDouble();
    double priceIn = q.value( 5 ).toDouble();
    double priceOut = q.value(6 ).toDouble();
    QDate lastMod = q.value( 7 ).toDate();
    QDate entered = q.value( 8 ).toDate();

    StockMaterial *mat = new StockMaterial( id, chapterID, material, unitID,
                                            pPack, Geld( priceIn ), Geld( priceOut ) );
    mat->setEnterDate( entered );
    mat->setLastModified( lastMod );
    mAllMaterial.append( mat );

  }

  return cnt;
}

void MatKatalog::recordUsage(int id)
{
    Q_UNUSED(id); // FIXME: Implement this!
}

void MatKatalog::deleteMaterial( int id )
{
  StockMaterialListIterator it( mAllMaterial );
  int cnt = 0;

  // qDebug () << "Deleting material id=" << id;
  while( it.hasNext() ) {
    StockMaterial *mat = it.next();
    if( mat->getID() == id ) {
      break;
    }
    cnt++;
  }
  if( cnt < mAllMaterial.count() ) {
    mAllMaterial.removeAt( cnt );
  }

  // remove from database.
  QSqlQuery q;
  q.prepare( QLatin1String("DELETE FROM stockMaterial WHERE matID=:Id"));
  q.bindValue( ":Id", id );
  q.exec();
  // qDebug () << "SQL Delete Success: " << q.lastError().text();

}

StockMaterialList MatKatalog::getRecordList( const CatalogChapter& chapter )
{
  StockMaterialList list;

  int chapID = chapter.id().toInt();
  StockMaterialListIterator it( mAllMaterial );
  
  while( it.hasNext() ) {
    StockMaterial *mat = it.next();

    if ( mat->chapter() == chapID ) {
      list.append( mat );
    }
  }
  return list;

}

StockMaterial* MatKatalog::materialFromId( long id )
{
    StockMaterialListIterator it( mAllMaterial );

    while( it.hasNext() ) {
        StockMaterial *mat = it.next();

        if ( mat->getID() == id ) {
            return mat;
        }
    }
    return nullptr;
}

void MatKatalog::addNewMaterial( StockMaterial *mat )
{
  mAllMaterial.append( mat );
}


MatKatalog::~MatKatalog( )
{

}

/* END */

