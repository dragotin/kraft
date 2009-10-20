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
//Added by qt3to4:
#include <Q3Frame>

// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <kdialog.h>
#include <k3listbox.h>
#include <kguiitem.h>
#include <kpushbutton.h>
#include <knuminput.h>
#include <kvbox.h>

#include "kraftdb.h"
#include "mateditor.h"
#include "matdatatable.h"
#include "matkatalog.h"
#include "unitmanager.h"
#include "kraftsettings.h"


/* ********************************************************************************
 * Editor für die Materialkategorie
 * ********************************************************************************/

MatKatEditor::MatKatEditor( const QString& curChap,  QStringList chaps, QWidget *parent )
    :KDialog(parent)
{
  KVBox *vBox = new KVBox( this );

  setCaption( i18n("Material Chapter" ) );
  setButtons( Ok|Cancel );
  setDefaultButton( Ok );
  setModal( true );

  vBox->setSpacing(KDialog::spacingHint());

  (void) new QLabel( i18n("Set Chapter of the marked Material:"), vBox );
  m_combo = new QComboBox(vBox);
  m_combo->insertStringList(chaps);
  m_combo->setCurrentText(curChap);
  setMainWidget( vBox );
}


/* ********************************************************************************
 * Materialeditor Hauptdialog mit Datentable
 * ********************************************************************************/

MatEditor::MatEditor(const QString& /* katName  */, bool takeover, QWidget *parent,
                     bool modal, Qt::WFlags )
    : KDialog(parent ),
    // , name, modal, i18n("Edit Material"), Close, Close),
      m_takeOver(0)

{
    m_box = new KVBox( this );
    setMainWidget( m_box );
    setButtons( Close );

    QLabel *l = new QLabel(QString("<h1>") + i18n("Edit Material") +
                           QString("</h1>"), m_box);
    l->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Maximum );
    m_split = new QSplitter( m_box );

    m_kat = new MatKatalog();
    m_kat->load();

    /* Box to show the chapters */
    m_chapterBox = new K3ListBox( m_split );
    m_chapterBox->setAcceptDrops(true);
    connect( m_chapterBox, SIGNAL(highlighted(const QString&)),
             this, SLOT(slSelectKatalog(const QString&)));

    QStringList chaps = m_kat->getKatalogChapters();

    m_chapterBox->insertStringList( chaps );

    /* Datentabelle anlegen */
    KVBox *vBox = new KVBox(m_split);
    /* Spacing ist zwischen den Widgets */
    vBox->setSpacing( 0 );

    m_dataTable = new MatDataTable(vBox);

    /* Einen Kategorie-Knopf in entsprechendem Layout hinzufügen */
    KHBox *hBox = new KHBox( vBox );
    /* Margin ist der Abstand zum Aussenrand */
    hBox->setMargin(KDialog::spacingHint());


    m_katButton = new KPushButton( i18n("Chapter..."), hBox );
    m_katButton->setEnabled(false);

    hBox->setMargin(KDialog::marginHint());
    hBox->setFrameStyle(Q3Frame::WinPanel);


    if( takeover )
    {
        /* Takeover bedeutet, dass der Dialog vom Kalkulationsmodul aus aufgerufen
         * wurde. Da gibts dann keinen Kategorie-Button
         */
        m_katButton->hide();
        addAmountDetail(hBox);
    }
    else
    {
      // m_takeOver->hide();
      QWidget *spaceEater = new QWidget( hBox );
      spaceEater->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
    }

    connect( m_dataTable, SIGNAL(currentChanged(int, int)),
             this, SLOT(slTableSelected(int, int)));

    connect( m_katButton, SIGNAL(clicked()),
             this, SLOT(slKatButtonClick()));

     connect( m_takeOver, SIGNAL(clicked()),
             this, SLOT(slTakeOver()));

    QString lastChap = KraftSettings::self()->lastMaterialChapter();
    kDebug() << "Selecting first katalog " << chaps[0] << endl;
    slSelectKatalog( lastChap );
    m_chapterBox->setSelected( 0, true);

    setInitialSize( KraftSettings::self()->materialCatalogSize() );
}

void MatEditor::addAmountDetail( QWidget *parent )
{
    KVBox *vbox = new KVBox(parent);
    vbox->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    // (void) new QLabel(i18n("Material"),vbox);
    m_matShort = new QLabel( vbox );
    m_matShort->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    // m_matShort->setFrameStyle( QFrame::Box|QFrame::Plain );
    m_matShort->setLineWidth( 0 );

    m_matShort->setMinimumHeight(2*m_matShort->fontMetrics().height()+2*m_matShort->margin());

    KHBox *hbox = new KHBox(vbox);
    // (void) new QLabel(i18n("Menge: "), hbox);
    m_amount   = new KDoubleNumInput( hbox );
    m_amount->setValue( 1.0);
    m_amount->setPrecision(3);
    m_amount->setLabel( i18n("Amount: "), Qt::AlignLeft|Qt::AlignVCenter);
    m_unit     = new QLabel(hbox);
    m_takeOver = new KPushButton( i18n("add"), hbox );
    (void) new QLabel(i18n(" to Calculation "), hbox);

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
            Einheit e( UnitManager::self()->getUnit(unitID));
            m_amount->setValue(1.0);
            QString einh( e.einheit(1.0));

            m_unit->setText( " "+einh ); // .leftJustified(12, ' '));
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

void MatEditor::slotClose()
{
  KraftSettings::self()->setMaterialCatalogSize( size() );

  QString chap = m_chapterBox->currentText();
  if ( !chap.isEmpty() ) {
    KraftSettings::self()->setLastMaterialChapter( chap );
  }
  KraftSettings::self()->self()->writeConfig();
  slotClose();
}

/* END */

#include "mateditor.moc"

