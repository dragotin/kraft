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

// include files for Qt
#include <qsplitter.h>
#include <qsql.h>
#include <qlabel.h>
#include <qsizepolicy.h>

// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klistbox.h>
#include <kguiitem.h>
#include <kpushbutton.h>
#include <knuminput.h>

#include "kraftdb.h"
#include "mateditor.h"
#include "matdatatable.h"
#include "matkatalog.h"
#include "unitmanager.h"


/* ********************************************************************************
 * Editor für die Materialkategorie
 * ********************************************************************************/

MatKatEditor::MatKatEditor( const QString& curChap,  QStringList chaps, QWidget *parent, const char* name )
    :KDialogBase(parent, name, true, i18n("Materialkategorie"), Ok|Cancel, Ok)
{
    QVBox *vBox = makeVBoxMainWidget();
    vBox->setSpacing(KDialog::spacingHint());

    (void) new QLabel( i18n("Kategorie des markierten Materials festlegen:"), vBox );
    m_combo = new QComboBox(vBox);
    m_combo->insertStringList(chaps);
    m_combo->setCurrentText(curChap);
}


/* ********************************************************************************
 * Materialeditor Hauptdialog mit Datentable
 * ********************************************************************************/

MatEditor::MatEditor(const QString& katName, bool takeover, QWidget *parent,
                     const char* name, bool modal, WFlags )
    : KDialogBase(parent, name, modal, i18n("Material editieren"), Close, Close),
      m_takeOver(0)

{
    m_box = makeVBoxMainWidget();

    QLabel *l = new QLabel(QString("<h1>") + i18n("Editieren von Materialkatalogen") +
                           QString("</h1>"), m_box);
    l->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Maximum );
    m_split = new QSplitter( m_box );

    /* Laden des Materialkataloges */
    m_kat = new MatKatalog(katName);
    m_kat->load();

    /* Box zur Anzeige der Katalogkapitel */
    m_chapterBox = new KListBox( m_split );
    m_chapterBox->setAcceptDrops(true);
    connect( m_chapterBox, SIGNAL(highlighted(const QString&)),
             this, SLOT(slSelectKatalog(const QString&)));

    QStringList chaps = m_kat->getKatalogChapters();

    m_chapterBox->insertStringList( chaps );

    /* Datentabelle anlegen */
    QVBox *vBox = new QVBox(m_split);
    /* Spacing ist zwischen den Widgets */
    vBox->setSpacing( 0 );

    m_dataTable = new MatDataTable(vBox);

    /* Einen Kategorie-Knopf in entsprechendem Layout hinzufügen */
    QHBox *hBox = new QHBox( vBox );
    /* Margin ist der Abstand zum Aussenrand */
    hBox->setMargin(KDialog::spacingHint());


    m_katButton = new KPushButton( i18n("Kategorie..."), hBox );
    m_katButton->setEnabled(false);

    addAmountDetail(hBox);
    hBox->setMargin(KDialog::marginHint());
    hBox->setFrameStyle(QFrame::WinPanel);


    if( takeover )
    {
        /* Takeover bedeutet, dass der Dialog vom Kalkulationsmodul aus aufgerufen
         * wurde. Da gibts dann keinen Kategorie-Button
         */
        m_katButton->hide();
    }
    else
    {
        m_takeOver->hide();
    }

    connect( m_dataTable, SIGNAL(currentChanged(int, int)),
             this, SLOT(slTableSelected(int, int)));

    connect( m_katButton, SIGNAL(clicked()),
             this, SLOT(slKatButtonClick()));

    connect( m_takeOver, SIGNAL(clicked()),
             this, SLOT(slTakeOver()));

    /* Ersten Katalog selektieren FIXME */
    kdDebug() << "Selecting first katalog " << chaps[0] << endl;
    slSelectKatalog( chaps[0] );
    m_chapterBox->setSelected( 0, true);
}

void MatEditor::addAmountDetail( QWidget *parent )
{
    QVBox *vbox = new QVBox(parent);
    vbox->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    // (void) new QLabel(i18n("Material"),vbox);
    m_matShort = new QLabel( vbox );
    m_matShort->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    // m_matShort->setFrameStyle( QFrame::Box|QFrame::Plain );
    m_matShort->setLineWidth( 0 );

    m_matShort->setMinimumHeight(2*m_matShort->fontMetrics().height()+2*m_matShort->margin());

    QHBox *hbox = new QHBox(vbox);
    // (void) new QLabel(i18n("Menge: "), hbox);
    m_amount   = new KDoubleNumInput( hbox );
    m_amount->setValue( 1.0);
    m_amount->setPrecision(3);
    m_amount->setLabel( i18n("Menge:"), AlignLeft|AlignVCenter);
    m_unit     = new QLabel(hbox);
    (void) new QLabel(i18n(" zu Kalkulation "), hbox);
    m_takeOver = new KPushButton( i18n("hinzu"), hbox );

    m_answer = new QLabel(hbox);

    QWidget *spaceEater = new QWidget(hbox);
    spaceEater->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
    m_takeOver->setEnabled(false);

}

MatEditor::~MatEditor( )
{
    if( m_kat )    delete m_kat;
}

void MatEditor::slSelectKatalog( const QString& str )
{
    m_currChapter = str;

    int chapID = m_kat->chapterID(str);
    m_dataTable->slSetCurrChapterID(chapID);
}

/* Setzt den enabled-Status für den Kategorie-Button */
void MatEditor::slTableSelected(int row, int)
{
    bool enabled = false;

    if( row > -1 )
    {
        enabled = true;
    }

    m_katButton->setEnabled( enabled );
    if( m_takeOver )
    {
        m_takeOver->setEnabled( enabled );

        QSqlRecord *rec = m_dataTable->currentRecord();
        if( rec )
        {
            m_matShort->setText( "<i>" + QString::fromUtf8(rec->value("material").toCString()) + "</i>");
            int unitID = rec->value("unitID").toInt();
            Einheit e( UnitManager::getUnit(unitID));
            m_amount->setValue(1.0);
            QString einh( e.einheit(1.0));

            m_unit->setText( " "+einh ); // .leftJustify(12, ' '));
        }

        /* Antwort-String löschen */
        if( m_answer )
            m_answer->setText( QString::null );
    }
}

void MatEditor::slKatButtonClick()
{
    if( ! (m_kat && m_dataTable) ) return;

    MatKatEditor medit( m_currChapter, m_kat->getKatalogChapters(), this );

    if( medit.exec() == QDialog::Accepted )
    {
        int id = m_kat->chapterID( medit.kategorie() );
        m_dataTable->updateCurrChapter(id);
    }

}

void MatEditor::slTakeOver()
{
    QSqlRecord *rec = m_dataTable->currentRecord();
    if( rec )
    {
        int matID = rec->value( "matID").toInt();
        emit materialSelected(matID, m_amount->value() );
    }
}

void MatEditor::slGotAnswer( const QString& ans )
{
    if( m_answer )
    {
        m_answer->setText(ans);
    }
}

/* END */

#include "mateditor.moc"
