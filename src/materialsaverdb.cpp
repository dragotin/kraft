/***************************************************************************
             materialsaverdb  -
                             -------------------
    begin                : 2006-12-07
    copyright            : (C) 2005 by Klaas Freitag
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

// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <k3staticdeleter.h>

#include <qdatetime.h>
#include <q3sqlcursor.h>
#include <qsqlrecord.h>

#include "kraftdb.h"
#include "kraftglobals.h"
#include "dbids.h"
#include "materialsaverdb.h"
#include "stockmaterial.h"

MaterialSaverDB* MaterialSaverDB::mSelf = 0;
static K3StaticDeleter<MaterialSaverDB> selfDeleter;

MaterialSaverDB::MaterialSaverDB( )
    : MaterialSaverBase()
{

}

MaterialSaverBase *MaterialSaverDB::self()
{
  if ( !mSelf ) {
    selfDeleter.setObject( mSelf,  new MaterialSaverDB() );
  }
  return mSelf;
}

bool MaterialSaverDB::saveTemplate( StockMaterial *mat )
{
    bool res = true;
    bool isNew = false;

    // Transaktion ?

    Q3SqlCursor cur("stockMaterial");
    QString templID = QString::number( mat->getID() );
    cur.select( "matID=" + templID);

    QSqlRecord *buffer = 0;

    if( cur.next())
    {
        kDebug() << "Updating material " << mat->getID() << endl;

        // mach update
        isNew = false;
        buffer = cur.primeUpdate();
        fillMaterialBuffer( buffer, mat, false );
        cur.update();
    }
    else
    {
        // insert
        kDebug() << "Creating new material database entry" << endl;

        isNew = true;
        buffer = cur.primeInsert();
        fillMaterialBuffer( buffer, mat, true );
        cur.insert();

        /* Jetzt die neue Template-ID selecten */
        dbID id = KraftDB::self()->getLastInsertID();
        kDebug() << "New Database ID=" << id.toInt() << endl;

        if( id.isOk() ) {
            mat->setID( id.toInt() );
            templID = id.toString();
        } else {
            kDebug() << "ERROR: Kann AUTOINC nicht ermitteln" << endl;
            res = false;
        }
    }
    return res;
}

void MaterialSaverDB::fillMaterialBuffer( QSqlRecord *rec, StockMaterial *mat, bool isNew )
{
  if( ! ( rec && mat ) ) return;
  rec->setValue( "chapterID", mat->chapter() );
  rec->setValue( "material", mat->name() );
  rec->setValue( "unitID", mat->getUnit().id() );
  rec->setValue( "perPack", mat->getAmountPerPack() );
  rec->setValue( "priceIn", mat->purchPrice().toDouble() );
  rec->setValue( "priceOut", mat->salesPrice().toDouble() );

  QDateTime dt = QDateTime::currentDateTime();
  QString dtString = dt.toString("yyyy-MM-dd hh:mm:ss" );

  if( isNew ) {
    rec->setValue( "enterDate", dtString);
  }
  rec->setValue("modifyDate", dtString );
}
