#include "kraftdocpositionsedit.h"

#include <QLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QBoxLayout>
#include <QToolTip>

#include <kpushbutton.h>
#include <klocale.h>
#include <kdialog.h>

#include "kraftview.h"


KraftDocPositionsEdit::KraftDocPositionsEdit( QWidget *parent )
  : KraftDocEdit( parent )
{
  QBoxLayout *topLayout = new QVBoxLayout();
  topLayout->setMargin( 0 );
  topLayout->setSpacing( 0 ); // KDialog::spacingHint() );

  QHBoxLayout *upperHBoxLayout = new QHBoxLayout;
  //upperHBoxLayout->setFrameStyle( Q3Frame::Box + Q3Frame::Sunken );
  upperHBoxLayout->setMargin( KDialog::marginHint()/2 );
  topLayout->addLayout( upperHBoxLayout );

  KPushButton *button = new KPushButton( i18n("Add Item...") );
  connect( button, SIGNAL( clicked() ), SIGNAL( addPositionClicked() ) );
  button->setToolTip( i18n( "Add a normal item to the document manually." ) );
  upperHBoxLayout->addWidget(button);
  upperHBoxLayout->setSpacing( 3 );

  button = new KPushButton( i18n("Add Discount Item") );
  connect( button, SIGNAL( clicked() ), SIGNAL( addExtraClicked() ) );
  upperHBoxLayout->addWidget(button);
  button->setToolTip( i18n( "Adds an item to the document that allows discounts on other items in the document" ) );

  button = new KPushButton( i18n("Import Items...") );
  connect( button, SIGNAL( clicked() ), SIGNAL( importItemsClicked() ) );
  upperHBoxLayout->addWidget(button);
  button->setToolTip( i18n( "Opens a dialog where multiple items can be imported from a text file." ) );

  QWidget *spaceEater = new QWidget( );
  spaceEater->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );
  upperHBoxLayout->addWidget(spaceEater);

  m_positionScroll = new KraftViewScroll( this );
  topLayout->addWidget( m_positionScroll );

  setTitle( i18n( "Document Positions" ) );
  setColor( "#9affa9" );
  setLayout(topLayout);
}

#include "kraftdocpositionsedit.moc"
