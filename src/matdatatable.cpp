/***************************************************************************
             matdatatable  -
                             -------------------
    begin                : 2004-14-11
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
#include <qpainter.h>
#include <qsqlpropertymap.h>
#include <qdragobject.h>
#include <qguardedptr.h>
#include <qpopupmenu.h>

// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>


#include "matdatatable.h"
#include "unitmanager.h"
#include "kraftdb.h"

/* ********************************************************************************
 * Editor Factory
 */
QWidget* CustomSqlEditorFactory::createEditor( QWidget *parent, const QSqlField *field )
{
    if ( field->name() == "unitID" )
    {
        QWidget *editor = new EinheitPicker( parent );
        return editor;
    }

    return QSqlEditorFactory::createEditor( parent, field );
}

/* ********************************************************************************
 * Picker für die Einheit
 */

EinheitPicker::EinheitPicker( QWidget *parent, const char *name )
    : QComboBox( parent, name )
{
    insertStringList( UnitManager::allUnits());
}

int EinheitPicker::einheitId() const
{
    return UnitManager::getUnitIDSingular(currentText());
}


void EinheitPicker::setEinheitId( int einheitid )
{
    setCurrentText(UnitManager::getUnit(einheitid).einheitSingular() );
}




/* ********************************************************************************
 * Datatable fuer Material
 */

MatDataTable::MatDataTable(QWidget *parent, const char *name )
    : QDataTable(parent, name),
      m_currChapterID(-1)
{
    installEditorFactory( new CustomSqlEditorFactory() );

    QSqlPropertyMap *propMap = new QSqlPropertyMap();
    propMap->insert( "EinheitPicker", "einheitID" );
    installPropertyMap( propMap );

    addColumn( "material", i18n("Material") );
    addColumn( "unitID",   i18n("Unit") );
    addColumn( "perPack",  i18n( "Pieces per Unit" ) );
    addColumn( "priceIn",  i18n( "Price (Buy)" ) );
    addColumn( "priceOut", i18n( "Price (Sell)" ) );

    setDragEnabled(true);
    connect( this, SIGNAL(beforeInsert(QSqlRecord*)),
             this, SLOT(slBeforeInsert(QSqlRecord*)));

    if( ! KraftDB::getDB() ) return;
    setSqlCursor( new QSqlCursor("stockMat"), false );

    QStringList li;
    li << "material ASC";
    li << "perPack  ASC";
    li << "priceOut DESC";
    setSort(li);
}


MatDataTable::~MatDataTable( )
{

}

void MatDataTable::slBeforeInsert ( QSqlRecord * buf )
{
    kdDebug() << "Before Inserting" << endl;
    if( buf )
    {
        buf->setValue("matChapter", m_currChapterID );
        QString m = buf->value("material").toString();

        buf->setValue( "material", QVariant(m.utf8()) );
    }
}


void MatDataTable::paintField( QPainter * p, const QSqlField* field,
                               const QRect & cr, bool b)
{
    if ( !field )
        return;
    if ( field->name() == "unitID" )
    {
        QString text;
        text = UnitManager::getUnit( field->value().toInt() ).einheitSingular();

        p->drawText( 2,2, cr.width()-4, cr.height()-4, fieldAlignment( field ), text );
    }
    else if( field->name() == "priceIn" ||
             field->name() == "priceOut" )
    {
        double price = field->value().toDouble();
        QString str = KGlobal().locale()->formatMoney(price);
        p->drawText( 1,1, cr.width()-2, cr.height()-2, 2 /* fieldAlignment( field ) */, str );


    }
    else if( field->name() == "material" )
    {
        p->drawText( 1,1, cr.width()-2, cr.height()-2, fieldAlignment( field ), QString::fromUtf8(field->value().toString()) );

    }
    else
    {
        QDataTable::paintField( p, field, cr, b) ;
    }
}

void MatDataTable::slSetCurrChapterID( int id )
{
    m_currChapterID = id;
    kdDebug() << "Setting current chapter id " << id << endl;
    setFilter( "matChapter="+QString::number(id));
    refresh();
    adjustColumn(0);

}

/* Kategorie für das aktuelle Material setzen und updaten */
void MatDataTable::updateCurrChapter( int chapID )
{
    QSqlRecord *rec = sqlCursor()->primeUpdate();

    kdDebug() << "Setze Kategorie ChapterID=" << chapID << endl;

    if( rec )
    {
        rec->setValue("matChapter", chapID );
        sqlCursor()->update();
        refresh();
    }
}



/* END */

#include "matdatatable.moc"
