/***************************************************************************
             templatesaverdb  -
                             -------------------
    begin                : 2005-20-00
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
#include <qsqlcursor.h>
#include <qsqlrecord.h>

#include "kraftdb.h"
#include "kraftglobals.h"
#include "dbids.h"
#include "templatesaverdb.h"
#include "calcpart.h"
#include "floskeltemplate.h"
#include "zeitcalcpart.h"
#include "fixcalcpart.h"
#include "materialcalcpart.h"
#include "stockmaterial.h"



bool CalculationsSaverDB::saveFixCalcPart( FixCalcPart *cp, dbID parentID )
{
    bool result = true;
    QSqlCursor cur( mTableFixCalc );

    int cpId = cp->getDbID().toInt();
    kdDebug() << "CalcFix calcpart-ID is " << cpId << endl;
    if( cpId < 0 ) { // no db entry yet => INSERT
        if( !cp->isToDelete() ) {
            QSqlRecord *buffer = cur.primeInsert();
            fillFixCalcBuffer( buffer, cp );
            buffer->setValue( "TemplID", parentID.toInt() );
            cur.insert();

            dbID id = KraftDB::self()->getLastInsertID();
            kdDebug() << "Setting db-ID " << id.toString() << endl;
            cp->setDbID(id);
        } else {
            kdDebug() << "new element, but set to delete" << endl;
        }
    } else {
        if( cp->isToDelete() ) {
            kdDebug() << "deleting fix calc part " << cpId << endl;
            // delete this calcpart.
            QSqlCursor delcur( mTableFixCalc );
            delcur.select( "FCalcID="+QString::number(cpId) );
            if ( delcur.next() ) {
                delcur.primeDelete();
                int cnt = delcur.del();
                kdDebug() << "Amount of deleted entries: " << cnt << endl;
            }
        } else {
            // der Datensatz ist bereits in der Datenbank => UPDATE
            cur.select( "FCalcID=" + QString::number( cpId ) );
            if( cur.next() ) {
                QSqlRecord *buffer = cur.primeUpdate();
                buffer->setValue( "modDate", "systimestamp" );
                fillFixCalcBuffer( buffer, cp );
                cur.update();
            } else {
                kdError() << "Can not select FCalcID, corrupt data!" << endl;
            }
        }
    }

    return result;
}

void CalculationsSaverDB::fillFixCalcBuffer( QSqlRecord *buffer, FixCalcPart *cp )
{
    if( ! (buffer && cp )) return;
    buffer->setValue( "name", cp->getName() );
    buffer->setValue( "amount", cp->getMenge() );

    buffer->setValue( "price", cp->unitPreis().toDouble() );

    buffer->setValue( "percent", cp->getProzentPlus() );
    buffer->setValue( "modDate", "systimestamp" );
}

bool CalculationsSaverDB::saveMaterialCalcPart( MaterialCalcPart *cp, dbID parentID )
{
    bool result = true;
    if( !cp ) return result;

    QSqlCursor cur( mTableMatCalc );

    int cpId = cp->getDbID().toInt();
    kdDebug() << "Saving material calcpart id=" << cpId << endl;

    if( cpId < 0 ) { // kein Eintrag in db bis jetzt => INSERT
        QSqlRecord *buffer = cur.primeInsert();
        fillMatCalcBuffer( buffer, cp );
        buffer->setValue( "TemplID", parentID.toInt() );
        cur.insert();

        dbID id = KraftDB::self()->getLastInsertID();
        cp->setDbID(id);
    } else {
        // calcpart-ID ist bereits belegt, UPDATE
        cur.select( "MCalcID=" + QString::number( cpId ) );
        if( cur.next() ) {
            QSqlRecord *buffer = cur.primeUpdate();
            buffer->setValue( "modDate", "systimestamp" );
            fillMatCalcBuffer( buffer, cp );
            cur.update();
        } else {
            kdError() << "Can not select MCalcID, corrupt data!" << endl;
        }
    }

    // nun die Materialliste sichern
    StockMaterialList matList = cp->getCalcMaterialList();
    StockMaterialListIterator it( matList );

    StockMaterial *mat;

    while( (mat = it.current()) != 0 )
    {
        storeMaterialDetail( cp, mat );
        ++it;
    }
    return result;
}

void CalculationsSaverDB::storeMaterialDetail( MaterialCalcPart *cp, StockMaterial *mat )
{
    if( ! (cp && mat) ) return;
    kdDebug() << "storing material calcpart detail for material " << mat->name() << endl;

    /* create temporar dbcalcpart and fill the current material list */
    QSqlCursor cur( mTableMatDetailCalc );
    QString selStr = QString("CalcID=%1 AND materialID=%2" ).arg(cp->getDbID().toInt()).arg(mat->getID());
    kdDebug() << "Material details for calcID " << cp->getDbID().toString() << endl;

    cur.select(selStr);
    MaterialCalcPart dbPart("MatCalcPartonDB", 0 );
    while( cur.next() )
    {
      double amount = cur.value("amount").toDouble();
      int matID     = cur.value("materialID").toInt();
      dbPart.addMaterial(amount, matID);
    }

    /* Now start to compare the DB and the temp calc part */
    double newAmount = cp->getCalcAmount(mat);
    double origAmount = dbPart.getCalcAmount(mat);

    kdDebug() << "The new Value is " << newAmount << " and the orig is " << origAmount << endl;
    QSqlCursor modifyCur( mTableMatDetailCalc );

    if( origAmount > -1.0  ) {
        // Es gibt schon einen DS fuer dieses Material, schauen, ob die Anzahl
        // des Materials stimmt, wenn nicht, updaten.
        if( origAmount != newAmount ) {
            modifyCur.select(selStr);
            if( modifyCur.next() ) {
                QSqlRecord *upRec = modifyCur.primeUpdate();
                if( upRec ) {
                    upRec->setValue("amount", newAmount);
                    modifyCur.update();
                }
            }
            // muss geupdatet werden
        } else {
            // die Anzahl ist gleichgeblieben, nix zu tun.
        }
    } else {
        // nix gefunden, datensatz muss eingefuegt werden.
        QSqlRecord *insRec = modifyCur.primeInsert();
        insRec->setValue("amount", newAmount);
        insRec->setValue("CalcID", cp->getDbID().toInt());
        insRec->setValue("materialID", mat->getID());

        modifyCur.insert();
        dbID id = KraftDB::self()->getLastInsertID();
        if( id.isOk() ) {
            cp->setDbID(id);
        } else {
            kdDebug() << "ERROR: Keine gueltige DB-ID bei Anlage des Material CalcPart!" << endl;
        }
    }
}


void CalculationsSaverDB::fillMatCalcBuffer( QSqlRecord *buffer, MaterialCalcPart *cp )
{
    if( !(buffer && cp)) return;

    buffer->setValue("name", cp->getName());
    buffer->setValue("percent", cp->getProzentPlus() );

}

CalculationsSaverDB::CalculationsSaverDB( )
  : CalculationsSaverBase(),
    mTableTimeCalc( "CalcTime" ),
    mTableFixCalc( "CalcFixed" ),
    mTableMatCalc( "CalcMaterials" ),
    mTableMatDetailCalc( "CalcMaterialDetails" )
{

}

CalculationsSaverDB::CalculationsSaverDB( TargetType tt )
  : CalculationsSaverBase( tt ),
    mTableTimeCalc( "CalcTime" ),
    mTableFixCalc( "CalcFixed" ),
    mTableMatCalc( "CalcMaterials" ),
    mTableMatDetailCalc( "CalcMaterialDetails" )
{
  if ( tt == Document ) {
    mTableTimeCalc = "DocCalcTime";
    mTableFixCalc = "DocCalcFixed";
    mTableMatCalc = "DocCalcMaterials";
    mTableMatDetailCalc = "DocCalcMaterialDetails";
  }
}

bool CalculationsSaverDB::saveCalculations( CalcPartList parts, dbID parentID )
{
  bool res = true;

  CalcPart *cp;
  for ( cp = parts.first(); cp && res; cp = parts.next() )
  {
    if( cp->isDirty() )
    {
      if( cp->getType() == KALKPART_TIME ) {
        res = saveTimeCalcPart( static_cast<ZeitCalcPart*>(cp), parentID );
        Q_ASSERT( res );
      } else if( cp->getType() == KALKPART_FIX ) {
        res = saveFixCalcPart( static_cast<FixCalcPart*>(cp), parentID );
        Q_ASSERT( res );
      } else if( cp->getType() == KALKPART_MATERIAL ) {
        res = saveMaterialCalcPart( static_cast<MaterialCalcPart*>(cp), parentID );
        Q_ASSERT( res );
      } else {
        kdDebug() << "ERROR: Unbekannter Kalkulations-Anteil-Typ!" << endl;
      }
    }
  }

  return res;
}

bool CalculationsSaverDB::saveTimeCalcPart( ZeitCalcPart *cp, dbID parentId )
{
    bool result = true;
    if( !cp ) return result;

    QSqlCursor cur( mTableTimeCalc );

    int cpId = cp->getDbID().toInt();

    if( cpId < 0 ) { // kein Eintrag in db bis jetzt => INSERT
        if( ! cp->isToDelete() ) {
            QSqlRecord *buffer = cur.primeInsert();
            fillZeitCalcBuffer( buffer, cp );
            buffer->setValue( "TemplID", parentId.toInt() );
            cur.insert();

            dbID id = KraftDB::self()->getLastInsertID();
            cp->setDbID(id);
        } else {
            kdDebug() << "delete flag is set -> skip saving." << endl;
        }
    } else {
        if( cp->isToDelete() ) {
            // delete this calcpart.
            QSqlCursor delcur( mTableTimeCalc );
            delcur.select( "TCalcID="+cpId );
            if ( delcur.next() ) {
                delcur.primeDelete();
                delcur.del();
            }
        } else {
	    // Update needed, record is already in the database
            cur.select( "TCalcID=" + QString::number( cpId ) );
            if( cur.next() ) {
                QSqlRecord *buffer = cur.primeUpdate();
                buffer->setValue( "modDate", "systimestamp" );
                fillZeitCalcBuffer( buffer, cp );
                cur.update();
            } else {
                kdError() << "Unable to select TCalcID, corrupt data!" << endl;
            }
        }
    }

    return result;
}

void CalculationsSaverDB::fillZeitCalcBuffer( QSqlRecord *buffer, ZeitCalcPart *cp )
{
    if( ! (buffer && cp )) return;
    buffer->setValue( "name",    cp->getName() );
    buffer->setValue( "minutes", cp->getMinuten() );
    buffer->setValue( "percent", cp->getProzentPlus() );

    StdSatz std = cp->getStundensatz();
    buffer->setValue( "stdHourSet", std.getId().toInt() );

    buffer->setValue( "allowGlobal", cp->globalStdSetAllowed() ? 1 : 0 );
}


/* =========================================================================== */

TemplateSaverDB::TemplateSaverDB( )
    : TemplateSaverBase()
{

}


TemplateSaverDB::~TemplateSaverDB( )
{

}

bool TemplateSaverDB::saveTemplate( FloskelTemplate *tmpl )
{
    bool res = true;
    bool isNew = false;

    // Transaktion ?

    QSqlCursor cur("Catalog");
    QString templID = QString::number(tmpl->getTemplID());
    cur.select( "TemplID=" + templID);

    QSqlRecord *buffer = 0;

    if( cur.next())
    {
        kdDebug() << "Updating template " << tmpl->getTemplID() << endl;

        // mach update
        isNew = false;
        buffer = cur.primeUpdate();
        fillTemplateBuffer( buffer, tmpl, false );
        buffer->setValue( "modifyDatum", "systimestamp" );
        cur.update();
    }
    else
    {
        // insert
        kdDebug() << "Creating new database entry" << endl;

        isNew = true;
        buffer = cur.primeInsert();
        fillTemplateBuffer( buffer, tmpl, true );
        cur.insert();

        /* Jetzt die neue Template-ID selecten */
        dbID id = KraftDB::self()->getLastInsertID();
        kdDebug() << "New Database ID=" << id.toInt() << endl;

        if( id.isOk() ) {
            tmpl->setTemplID(id.toInt() );
            templID = id.toString();
        } else {
            kdDebug() << "ERROR: Kann AUTOINC nicht ermitteln" << endl;
            res = false;
        }
    }

    if( res )
    {
        /* Nun die einzelnen Calcparts speichern */
        CalcPartList parts = tmpl->getCalcPartsList();
        CalculationsSaverDB calculationSaver;
        res = calculationSaver.saveCalculations( parts, tmpl->getTemplID() );
    }
    return res;

}

void TemplateSaverDB::fillTemplateBuffer( QSqlRecord *buffer, FloskelTemplate *tmpl, bool isNew )
{
    buffer->setValue( "chapterID", tmpl->getChapterID());
    buffer->setValue( "unitID", tmpl->einheit().id());
    buffer->setValue( "Floskel", tmpl->getText().utf8() );
    buffer->setValue( "Gewinn", tmpl->getGewinn() );
    buffer->setValue( "zeitbeitrag", tmpl->hasTimeslice() );

    /* neue templates kriegen ein Eintragsdatum */
    QDateTime dt = QDateTime::currentDateTime();
    QString dtString = dt.toString("yyyy-MM-dd hh:mm:ss" );

    if( isNew ) {
        buffer->setValue( "enterDatum", dtString);
    }
    buffer->setValue("modifyDatum", dtString );

    int ctype = 2;  // Calculation type Calculation
    if( tmpl->calcKind() == ManualPrice )
    {
        ctype = 1;
    }
    buffer->setValue( "Preisart", ctype );
    buffer->setValue( "EPreis", tmpl->manualPrice().toDouble() );
}

/* END */

