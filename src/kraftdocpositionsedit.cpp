#include "kraftdocpositionsedit.h"

#include <qhbox.h>
#include <qlayout.h>

#include <kpushbutton.h>
#include <klocale.h>
#include <kdialog.h>

#include "kraftview.h"

KraftDocPositionsEdit::KraftDocPositionsEdit( QWidget *parent )
  : KraftDocEdit( parent )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  QHBox *upperHBox = new QHBox( this );
  topLayout->addWidget( upperHBox );

  KPushButton *button = new KPushButton( i18n("Add"), upperHBox );
  connect( button, SIGNAL( clicked() ), SIGNAL( addPositionClicked() ) );

  button = new KPushButton( i18n("Extra"), upperHBox );
  connect( button, SIGNAL( clicked() ), SIGNAL( addExtraClicked() ) );

  QWidget *spaceEater = new QWidget( upperHBox );
  spaceEater->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );

  m_positionScroll = new KraftViewScroll( this );
  topLayout->addWidget( m_positionScroll );

  setTitle( i18n( "Document Positions" ) );
  setColor( "#9affa9" );
}

#include "kraftdocpositionsedit.moc"
