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
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlTableModel>

// include files for KDE
#include <QDebug>

#include "kraftdb.h"
#include "kraftglobals.h"
#include "dbids.h"
#include "templatesaverdb.h"
#include "calcpart.h"
#include "floskeltemplate.h"
#include "timecalcpart.h"
#include "fixcalcpart.h"
#include "materialcalcpart.h"
#include "stockmaterial.h"


bool CalculationsSaverDB::saveFixCalcPart( FixCalcPart *cp, dbID parentID )
{
    bool result = true;
    QSqlTableModel model;
    model.setTable(mTableFixCalc);
    int cpId = cp->getDbID().toInt();
    model.setFilter("FCalcID=" + QString::number( cpId ));
    model.select();
    // qDebug () << "CalcFix calcpart-ID is " << cpId << endl;
    if( cpId < 0 ) { // no db entry yet => INSERT
        if( !cp->isToDelete() ) {
            QSqlRecord buffer = model.record();
            fillFixCalcBuffer( &buffer, cp );
            buffer.setValue( "TemplID", parentID.toInt() );
            model.insertRecord(-1, buffer);
            model.submitAll();

            dbID id = KraftDB::self()->getLastInsertID();
            // qDebug () << "Setting db-ID " << id.toString() << endl;
            cp->setDbID(id);
        } else {
            // qDebug () << "new element, but set to delete" << endl;
        }
    } else {
        if( cp->isToDelete() ) {
            // qDebug () << "deleting fix calc part " << cpId << endl;
            // delete this calcpart.
            if ( model.rowCount() > 0 ) {
                int cnt = model.rowCount();
                model.removeRows(0, cnt);
                model.submitAll();
                // qDebug () << "Amount of deleted entries: " << cnt << endl;
            }
        } else {
            // der Datensatz ist bereits in der Datenbank => UPDATE
            if( model.rowCount() > 0 ) {
                QSqlRecord buffer = model.record(0);
                buffer.setValue( "modDate", KraftDB::self()->currentTimeStamp() );
                fillFixCalcBuffer(& buffer, cp );
                model.setRecord(0, buffer);
                model.submitAll();
            } else {
                qCritical() << "Can not select FCalcID, corrupt data!" << endl;
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
    buffer->setValue( "modDate", KraftDB::self()->currentTimeStamp() );
}

bool CalculationsSaverDB::saveMaterialCalcPart( MaterialCalcPart *cp, dbID parentID )
{
  bool result = true;
  if( !cp ) return result;

  QSqlTableModel model;
  model.setTable( mTableMatCalc );
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  int cpId = cp->getDbID().toInt();
  model.setFilter("MCalcID=" + QString::number( cpId ));
  model.select();
  // qDebug () << "Saving material calcpart id=" << cpId << endl;

  if( cpId < 0 ) { // no entry in database yet, need to insert
    QSqlRecord buffer = model.record();
    fillMatCalcBuffer( &buffer, cp );
    buffer.setValue( "TemplID", parentID.toInt() );
    model.insertRecord(-1, buffer);
    model.submitAll();

    dbID id = KraftDB::self()->getLastInsertID();
    cp->setDbID(id);
  } else {
    // there is an db entry, update needed
    if(cp->isToDelete()) {
      // This calcpart must be deleted
      if( model.rowCount() > 0) {
        model.removeRow(0);
        model.submitAll();
      }
    } else {
      // don't delete, update!
      if( model.rowCount() > 0) {
        QSqlRecord buffer = model.record(0);
        buffer.setValue( "modDate", KraftDB::self()->currentTimeStamp() );
        fillMatCalcBuffer( &buffer, cp );
        model.setRecord(0, buffer);
        model.submitAll();
      } else {
        qCritical() << "Can not select MCalcID, corrupt data!" << endl;
      }
    }
  }

return result;
}

void CalculationsSaverDB::fillMatCalcBuffer( QSqlRecord *buffer, MaterialCalcPart *cp )
{
    if( !(buffer && cp)) return;

    buffer->setValue("materialID", cp->getMaterial()->getID());
    buffer->setValue("amount", cp->getCalcAmount());
    buffer->setValue("TemplID", cp->getTemplID().toInt());
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

  CalcPartListIterator it( parts );

  while( it.hasNext()) {
    CalcPart *cp = it.next();
    if( cp->isDirty() )
    {
      if( cp->getType() == KALKPART_TIME ) {
        res = saveTimeCalcPart( static_cast<TimeCalcPart*>(cp), parentID );
        Q_ASSERT( res );
      } else if( cp->getType() == KALKPART_FIX ) {
        res = saveFixCalcPart( static_cast<FixCalcPart*>(cp), parentID );
        Q_ASSERT( res );
      } else if( cp->getType() == KALKPART_MATERIAL ) {
        res = saveMaterialCalcPart( static_cast<MaterialCalcPart*>(cp), parentID );
        Q_ASSERT( res );
      } else {
        // qDebug () << "ERROR: Unbekannter Kalkulations-Anteil-Typ!" << endl;
      }
    }
  }

  return res;
}

bool CalculationsSaverDB::saveTimeCalcPart( TimeCalcPart *cp, dbID parentId )
{
    bool result = true;
    if( !cp ) return result;

    int cpId = cp->getDbID().toInt();

    QSqlTableModel model;
    model.setTable( mTableTimeCalc );
    model.setFilter( "TCalcID="+QString::number(cpId) );
    model.select();

    // qDebug () << "Models last error: " << model.lastError() << model.rowCount();

    if( cpId < 0 )
    { // no entry in db yet => INSERT
        if( ! cp->isToDelete() ) {
            QSqlRecord buffer = model.record();
            fillTimeCalcBuffer( &buffer, cp );
            buffer.setValue( "TemplID", parentId.toInt() );
            model.insertRecord(-1, buffer);

            dbID id = KraftDB::self()->getLastInsertID();
            cp->setDbID(id);
        } else {
            // qDebug () << "delete flag is set -> skip saving." << endl;
        }
    }

    else
    {
        if( cp->isToDelete() ) {
            // delete this calcpart.
            if ( model.rowCount() > 0 ) {
                model.removeRow(0);
                model.submitAll();
            }
        }
        else {
	    // Update needed, record is already in the database
            if( model.rowCount() > 0 ) {
                QSqlRecord buffer = model.record(0);
                buffer.setValue( "modDate", KraftDB::self()->currentTimeStamp() );
                fillTimeCalcBuffer( &buffer, cp );
                model.setRecord(0, buffer);
                model.submitAll();
            } else {
                qCritical() << "Unable to select TCalcID, corrupt data!" << endl;
            }
        }
    }

    return result;
}

void CalculationsSaverDB::fillTimeCalcBuffer( QSqlRecord *buffer, TimeCalcPart *cp )
{
    if( ! (buffer && cp )) return;
    buffer->setValue( "name",    cp->getName() );
    buffer->setValue( "minutes", cp->duration() );
    buffer->setValue( "timeUnit", cp->timeUnitIndex());
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

    // Transaktion ?

    QSqlTableModel model;
    model.setEditStrategy(QSqlTableModel::OnManualSubmit);
    model.setTable("Catalog");
    QString templID = QString::number(tmpl->getTemplID());
    model.setFilter("TemplID=" + templID);
    model.select();

    QSqlRecord buffer;
    if( model.rowCount() > 0)
    {
        // qDebug () << "Updating template " << tmpl->getTemplID() << endl;

        // mach update
        buffer = model.record(0);
        fillTemplateBuffer( &buffer, tmpl, false );
        buffer.setValue( "modifyDatum", KraftDB::self()->currentTimeStamp() );
        model.setRecord(0, buffer);
        model.submitAll();
    }
    else
    {
        // insert
        // qDebug () << "Creating new database entry" << endl;

        buffer = model.record();
        fillTemplateBuffer( &buffer, tmpl, true );
        model.insertRecord(-1, buffer);
        model.submitAll();

        /* Jetzt die neue Template-ID selecten */
        dbID id = KraftDB::self()->getLastInsertID();
        // qDebug () << "New Database ID=" << id.toInt() << endl;

        if( id.isOk() ) {
            tmpl->setTemplID(id.toInt() );
            templID = id.toString();
        } else {
            // qDebug () << "ERROR: Kann AUTOINC nicht ermitteln" << endl;
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
    buffer->setValue( "chapterID", tmpl->chapterId().toInt() );
    buffer->setValue( "unitID", tmpl->unit().id());
    buffer->setValue( "Floskel", tmpl->getText() );
    buffer->setValue( "Gewinn", tmpl->getBenefit() );
    buffer->setValue( "zeitbeitrag", tmpl->hasTimeslice() );


    QDateTime dt = QDateTime::currentDateTime();
    QString dtString = KraftDB::self()->currentTimeStamp(dt);
    if( isNew ) {
        buffer->setValue( "enterDatum", dtString);
        tmpl->setEnterDate( dt );
    }
    buffer->setValue("modifyDatum", dtString );
    tmpl->setModifyDate( dt );

    int ctype = 2;  // Calculation type Calculation
    if( tmpl->calcKind() == CatalogTemplate::ManualPrice )
    {
        ctype = 1;
    }
    buffer->setValue( "Preisart", ctype );
    buffer->setValue( "EPreis", tmpl->manualPrice().toDouble() );
}

void TemplateSaverDB::saveTemplateChapter( FloskelTemplate* tmpl )
{
  if( tmpl ) {
    dbID id = tmpl->getTemplID();
    dbID chapId = tmpl->chapterId();

    QSqlQuery qUpdate;
    // qDebug () << "Updating Chapter to chapter id " << chapId.toInt() << " of id " << id.toString();
    QString sql = "UPDATE Catalog SET chapterID=:chap WHERE TemplID=:id";
    qUpdate.prepare( sql );
    qUpdate.bindValue( ":chap", chapId.toInt() );
    qUpdate.bindValue( ":id", id.toInt() );

    qUpdate.exec();
    // qDebug () << "setting template chapter sql: " << qUpdate.lastError().text();
  }
}

/* END */

