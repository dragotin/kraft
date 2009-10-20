#include <kdialog.h>
#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './timecalcpart.ui'
**
** Created: Fr Okt 22 17:46:28 2004
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.3   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "timecalcpart.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <q3frame.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <klineedit.h>
#include <knuminput.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>

/*
 *  Constructs a calcdetail_time as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
calcdetail_time::calcdetail_time( QWidget* parent, const char* name, bool modal, Qt::WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "calcdetail_time" );
    setSizeGripEnabled( TRUE );
    calcdetail_timeLayout = new Q3GridLayout( this, 1, 1, 11, 6, "calcdetail_timeLayout"); 
    spacer3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    calcdetail_timeLayout->addItem( spacer3, 0, 0 );

    layout6 = new Q3VBoxLayout( 0, 0, 6, "layout6"); 

    textLabel1 = new QLabel( this, "textLabel1" );
    layout6->addWidget( textLabel1 );

    textLabel2 = new QLabel( this, "textLabel2" );
    textLabel2->setAlignment( int( QLabel::WordBreak | Qt::AlignVCenter ) );
    layout6->addWidget( textLabel2 );

    frame3 = new Q3Frame( this, "frame3" );
    frame3->setFrameShape( Q3Frame::StyledPanel );
    frame3->setFrameShadow( Q3Frame::Raised );
    frame3Layout = new Q3GridLayout( frame3, 1, 1, 11, 6, "frame3Layout"); 

    layout4 = new Q3GridLayout( 0, 1, 1, 0, 6, "layout4"); 

    m_nameEdit = new KLineEdit( frame3, "m_nameEdit" );

    layout4->addWidget( m_nameEdit, 0, 1 );

    textLabel3_2_2 = new QLabel( frame3, "textLabel3_2_2" );

    layout4->addWidget( textLabel3_2_2, 2, 0 );

    textLabel3_2 = new QLabel( frame3, "textLabel3_2" );

    layout4->addWidget( textLabel3_2, 1, 0 );

    textLabel3 = new QLabel( frame3, "textLabel3" );

    layout4->addWidget( textLabel3, 0, 0 );

    layout2 = new Q3HBoxLayout( 0, 0, 6, "layout2"); 

    m_dauer = new KIntNumInput( frame3, "m_dauer" );
    m_dauer->setValue( 6 );
    m_dauer->setMinValue( 0 );
    m_dauer->setMaxValue( 10000 );
    layout2->addWidget( m_dauer );

    textLabel4 = new QLabel( frame3, "textLabel4" );
    layout2->addWidget( textLabel4 );

    layout4->addLayout( layout2, 1, 1 );

    m_stdGlobal = new QCheckBox( frame3, "m_stdGlobal" );

    layout4->addWidget( m_stdGlobal, 3, 1 );

    m_hourSets = new QComboBox( FALSE, frame3, "m_hourSets" );

    layout4->addWidget( m_hourSets, 2, 1 );

    frame3Layout->addLayout( layout4, 0, 0 );
    layout6->addWidget( frame3 );

    Layout1 = new Q3HBoxLayout( 0, 0, 6, "Layout1"); 

    buttonHelp = new QPushButton( this, "buttonHelp" );
    buttonHelp->setAutoDefault( TRUE );
    Layout1->addWidget( buttonHelp );
    Horizontal_Spacing2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( Horizontal_Spacing2 );

    buttonOk = new QPushButton( this, "buttonOk" );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    Layout1->addWidget( buttonOk );

    buttonCancel = new QPushButton( this, "buttonCancel" );
    buttonCancel->setAutoDefault( TRUE );
    Layout1->addWidget( buttonCancel );
    layout6->addLayout( Layout1 );

    calcdetail_timeLayout->addLayout( layout6, 0, 0 );
    languageChange();
    resize( QSize(466, 307).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );

    // buddies
    textLabel3->setBuddy( m_nameEdit );
}

/*
 *  Destroys the object and frees any allocated resources
 */
calcdetail_time::~calcdetail_time()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void calcdetail_time::languageChange()
{
    setCaption( tr2i18n( "Kalkulationsanteil Zeit" ) );
    textLabel1->setText( tr2i18n( "<h1>Kalkulationsanteil Zeit</h1>" ) );
    textLabel2->setText( tr2i18n( "Zeitabhängige Anteile an der Kalkulation sind vom im Dokument global eingestellten Stundensatz abhängig." ) );
    textLabel3_2_2->setText( tr2i18n( "&Stundensatz:" ) );
    textLabel3_2->setText( tr2i18n( "&Zeitdauer" ) );
    textLabel3->setText( tr2i18n( "&Bezeichnung:" ) );
    textLabel4->setText( tr2i18n( "Minuten" ) );
    m_stdGlobal->setText( tr2i18n( "Stundensatz &global anpassen" ) );
    m_stdGlobal->setAccel( QKeySequence( tr2i18n( "Alt+G" ) ) );
    buttonHelp->setText( tr2i18n( "&Help" ) );
    buttonHelp->setAccel( QKeySequence( tr2i18n( "F1" ) ) );
    buttonOk->setText( tr2i18n( "&OK" ) );
    buttonOk->setAccel( QKeySequence( QString::null ) );
    buttonCancel->setText( tr2i18n( "&Cancel" ) );
    buttonCancel->setAccel( QKeySequence( QString::null ) );
}

#include "timecalcpart.moc"
