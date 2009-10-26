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

#ifndef _MATDATATABLE_H
#define _MATDATATABLE_H


#include <QComboBox>
#include <q3datatable.h>
#include <q3sqleditorfactory.h>


// include files

class QSqlRecord;

/* ********************************************************************************
 * SQL Editor Factory
 */
class CustomSqlEditorFactory : public Q3SqlEditorFactory
{
    Q_OBJECT
public:
    QWidget *createEditor( QWidget *parent, const QSqlField *field );
};

/* ********************************************************************************
 * Combobox for Einheiten
 */
class EinheitPicker : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY( int einheitID READ einheitId WRITE setEinheitId )

public:
    EinheitPicker( QWidget *parent=0, const char *name=0 );
    int einheitId() const;
    void setEinheitId( int id );
};

/**
 * Datatable
 */

class MatDataTable : public Q3DataTable
{
    Q_OBJECT

public:

    MatDataTable( QWidget *parent=0, const char *name=0 );
    ~MatDataTable();

    void updateCurrChapter(int chapID);
    void paintField( QPainter * p, const QSqlField* field, const QRect & cr, bool );

public slots:
    void slBeforeInsert( QSqlRecord*);
    void slSetCurrChapterID( int id );

private:
    int m_currChapterID;
};

#endif

/* END */

