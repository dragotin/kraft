#include "kraftdocpositionsedit.h"

#include <qhbox.h>
#include <qlayout.h>

#include <kpushbutton.h>
#include <klocale.h>
#include <kdialog.h>

#include "kraftview.h"
#include <qtooltip.h>

KraftDocPositionsEdit::KraftDocPositionsEdit( QWidget *parent )
  : KraftDocEdit( parent )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setMargin( 0 );
  topLayout->setSpacing( 0 ); // KDialog::spacingHint() );

  QHBox *upperHBox = new QHBox( this );
  upperHBox->setFrameStyle( QFrame::Box + QFrame::Sunken );
  upperHBox->setMargin( KDialog::marginHint()/2 );
  topLayout->addWidget( upperHBox );

  KPushButton *button = new KPushButton( i18n("Add Item..."), upperHBox );
  connect( button, SIGNAL( clicked() ), SIGNAL( addPositionClicked() ) );
  QToolTip::add( button, i18n( "Add a normal item to the document manually." ) );
  upperHBox->setSpacing( 3 );
  button = new KPushButton( i18n("Add Discount Item"), upperHBox );
  connect( button, SIGNAL( clicked() ), SIGNAL( addExtraClicked() ) );
  QToolTip::add( button, i18n( "Adds an item to the document that allows discounts on other items in the document" ) );

  QWidget *spaceEater = new QWidget( upperHBox );
  spaceEater->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );

  m_positionScroll = new KraftViewScroll( this );
  topLayout->addWidget( m_positionScroll );

  setTitle( i18n( "Document Positions" ) );
  setColor( "#9affa9" );
}

#include "kraftdocpositionsedit.moc"
