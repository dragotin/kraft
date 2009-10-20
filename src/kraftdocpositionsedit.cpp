#include "kraftdocpositionsedit.h"

#include <q3hbox.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3Frame>
#include <Q3BoxLayout>

#include <kpushbutton.h>
#include <klocale.h>
#include <kdialog.h>

#include "kraftview.h"
#include <qtooltip.h>

KraftDocPositionsEdit::KraftDocPositionsEdit( QWidget *parent )
  : KraftDocEdit( parent )
{
  Q3BoxLayout *topLayout = new Q3VBoxLayout( this );
  topLayout->setMargin( 0 );
  topLayout->setSpacing( 0 ); // KDialog::spacingHint() );

  Q3HBox *upperHBox = new Q3HBox( this );
  upperHBox->setFrameStyle( Q3Frame::Box + Q3Frame::Sunken );
  upperHBox->setMargin( KDialog::marginHint()/2 );
  topLayout->addWidget( upperHBox );

  KPushButton *button = new KPushButton( i18n("Add Item..."), upperHBox );
  connect( button, SIGNAL( clicked() ), SIGNAL( addPositionClicked() ) );
  QToolTip::add( button, i18n( "Add a normal item to the document manually." ) );
  upperHBox->setSpacing( 3 );

  button = new KPushButton( i18n("Add Discount Item"), upperHBox );
  connect( button, SIGNAL( clicked() ), SIGNAL( addExtraClicked() ) );
   QToolTip::add( button, i18n( "Adds an item to the document that allows discounts on other items in the document" ) );

  button = new KPushButton( i18n("Import Items..."), upperHBox );
  connect( button, SIGNAL( clicked() ), SIGNAL( importItemsClicked() ) );
  QToolTip::add( button, i18n( "Opens a dialog where multiple items can be imported from a text file." ) );

  QWidget *spaceEater = new QWidget( upperHBox );
  spaceEater->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );

  m_positionScroll = new KraftViewScroll( this );
  topLayout->addWidget( m_positionScroll );

  setTitle( i18n( "Document Positions" ) );
  setColor( "#9affa9" );
}

#include "kraftdocpositionsedit.moc"
