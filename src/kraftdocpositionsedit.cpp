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


KraftViewScroll::KraftViewScroll( QWidget *parent ):
QScrollArea( parent )
{
  myWidget = new QWidget;
  myWidget->setAutoFillBackground(false);
  layout = new QVBoxLayout;
  layout->setAlignment(Qt::AlignTop);
  layout->setSizeConstraint( QLayout::SetMinAndMaxSize );
  layout->setContentsMargins( 0,0,0,0 );
  layout->setSpacing(0);
  myWidget->setLayout(layout);
  setWidget(myWidget);
  setWidgetResizable(true);
  myWidget->resize(0,0);
  myWidget->setMinimumHeight(0);
  myWidget->setMaximumHeight(0);
  myWidget->setContentsMargins(0, 0, 0, 0);
}

void KraftViewScroll::addChild( QWidget *child, int index )
{
    int y1 = myWidget->height();
    layout->insertWidget(index, child);
    int y2 = y1+child->height();
    myWidget->resize( child->width(), y2);
    myWidget->setMinimumHeight(y2);
    myWidget->setMaximumHeight(y2);
}

void KraftViewScroll::removeChild( PositionViewWidget *child )
{
  layout->removeWidget( child ); // from the scrollview
}

void KraftViewScroll::moveChild( PositionViewWidget *child, int index)
{
  layout->removeWidget(child);
  layout->insertWidget(index, child);
}

int KraftViewScroll::indexOf(PositionViewWidget *child)
{
  return layout->indexOf(child);
}

// #########################################################

KraftDocPositionsEdit::KraftDocPositionsEdit( QWidget *parent )
  : KraftDocEdit( parent )
{
  QBoxLayout *topLayout = new QVBoxLayout();
  topLayout->setMargin( 0 );
  topLayout->setSpacing( 0 ); // KDialog::spacingHint() );

  QHBoxLayout *upperHBoxLayout = new QHBoxLayout;
  //upperHBoxLayout->setFrameStyle( QFrame::Box + QFrame::Sunken );
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

  setTitle( i18n( "Document Items" ) );
  setColor( "#9affa9" );
  setLayout(topLayout);
}
