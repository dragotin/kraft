/***************************************************************************
             mateditor  -
                             -------------------
    begin                : 2004-25-10
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

#ifndef _MATEDITOR_H
#define _MATEDITOR_H

#include <qdatatable.h>
#include <qvbox.h>
#include <qcombobox.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kdialogbase.h>

// include files

/**
 *
 */
class QLabel;
class KDoubleNumInput;
class QSplitter;
class QWidget;
class KListBox;
class MatKatalog;
class QSqlCursor;
class KPushButton;
class MatDataTable;

/* ********************************************************************************
 * Editor für die Materialkategorie
 * ********************************************************************************/

class MatKatEditor : public KDialogBase
{
    Q_OBJECT

public:
    MatKatEditor( const QString& curChap,  QStringList chaps, QWidget *parent, const char* name=0 );
    QString kategorie() const { return m_combo->currentText(); }

private:
    QComboBox *m_combo;
};


/* ********************************************************************************
 * Materialeditor Hauptdialog mit Datentable
 * ********************************************************************************/

class MatEditor : public KDialogBase
{
    Q_OBJECT

public:
    MatEditor(const QString& katName, bool takeover = false, QWidget * parent = 0,
              const char * name = 0, bool modal = FALSE, WFlags f = 0 );
    ~MatEditor();

signals:
    void materialSelected(int, double);

public slots:
    virtual void slSelectKatalog( const QString& str );
    virtual void slTableSelected(int row, int);
    void slKatButtonClick();
    void slTakeOver();
    void slGotAnswer( const QString& );

private:
    virtual void addAmountDetail(QWidget*);

    QSplitter  *m_split;
    KListBox   *m_chapterBox;

    MatKatalog *m_kat;
    MatDataTable *m_dataTable;
    QVBox      *m_box;

    KPushButton *m_katButton;
    KPushButton *m_takeOver;
    QString      m_currChapter;

    QLabel      *m_matShort;
    QLabel      *m_unit;
    QLabel      *m_answer;
    KDoubleNumInput *m_amount;
};

#endif

/* END */

