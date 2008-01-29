/***************************************************************************
        positionviewwidget - inherited class for doc position views.
                             -------------------
    begin                : 2006-02-20
    copyright            : (C) 2006 by Klaas Freitag
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

#include <qlabel.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qcolor.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <kglobal.h>
#include <klocale.h>
#include <knuminput.h>
#include <ktextedit.h>
#include <kpopupmenu.h>
#include <kiconloader.h>

#include "docposition.h"
#include "positionviewwidget.h"
#include "unitmanager.h"
#include "geld.h"
#include "kraftsettings.h"
#include "defaultprovider.h"

PositionViewWidget::PositionViewWidget()
 : positionWidget(),
 mModified( false ),
 m_skipModifiedSignal( false ),
 mToDelete(false),
 mOrdNumber(0),
 mExecPopup( new KPopupMenu( this ) ) ,
 mState( Active ),
 mKind( Normal )
{
  m_sbUnitPrice->setMinValue( 0 );
  m_sbUnitPrice->setMaxValue( 99999.99 );
  m_sbUnitPrice->setPrecision( 2 );

  pbExec->setToggleButton( false );
  connect( m_sbAmount, SIGNAL( valueChanged( double )),
             this, SLOT( slotRefreshPrice( ) ) );
  connect( m_sbUnitPrice, SIGNAL( valueChanged( double )),
             this, SLOT( slotRefreshPrice( ) ) );
  connect( pbExec, SIGNAL( pressed() ),
             this,  SLOT( slotExecButtonPressed() ) );

  /* modified signals */
  connect( m_cbUnit,      SIGNAL( activated(int) ), this,      SLOT( slotModified() ) );
  // connect( m_teFloskel,   SIGNAL( textChanged() ), this,       SLOT( slotModified() ) );
  // teFloskel is already connected in ui file
  connect( m_sbAmount,    SIGNAL( valueChanged(double)), this, SLOT( slotModified() ) );
  connect( m_sbUnitPrice, SIGNAL( valueChanged(double)), this, SLOT( slotModified() ) );

  mExecPopup->insertTitle( i18n("Position Actions") );
  mStateSubmenu = new QPopupMenu;
  mStateSubmenu->insertItem( i18n( "Normal" ), this, SIGNAL( positionStateNormal() ) );
  mStateSubmenu->insertItem( SmallIconSet( "alternative" ),
                            i18n( "Alternative" ), this, SIGNAL( positionStateAlternative() ) );
  mStateSubmenu->insertItem( SmallIconSet( "demand" ),
                            i18n( "On Demand" ), this, SIGNAL( positionStateDemand() ) );
  mExecPopup->insertItem( i18n( "Position Kind" ), mStateSubmenu );

  mExecPopup->insertSeparator();

  mExecPopup->insertItem(  SmallIconSet("up"),
                           i18n("Move Up"),         this, SIGNAL( moveUp() ) );
  mExecPopup->insertItem(  SmallIconSet("down"),
                           i18n("Move Down"),       this, SIGNAL( moveDown() ) );
  mLockId = mExecPopup->insertItem(  SmallIconSet("encrypted"),
                           i18n("Lock Position"),   this, SIGNAL( lockPosition() ) );
  mUnlockId = mExecPopup->insertItem(  SmallIconSet("decrypted"),
                           i18n("Unlock Position"), this, SIGNAL( unlockPosition() ) );
  mDeleteId = mExecPopup->insertItem(  SmallIconSet("remove"),
                           i18n("Delete Position"), this, SIGNAL( deletePosition() ) );


  connect( this, SIGNAL( positionStateNormal() ), this, SLOT( slotSetPositionNormal() ) );
  connect( this, SIGNAL( positionStateAlternative() ), this, SLOT( slotSetPositionAlternative() ) );
  connect( this, SIGNAL( positionStateDemand() ), this, SLOT( slotSetPositionDemand() ) );


  connect( this, SIGNAL( lockPosition() ),   this, SLOT( slotLockPosition() ) );
  connect( this, SIGNAL( unlockPosition() ), this, SLOT( slotUnlockPosition() ) );

  connect( mExecPopup, SIGNAL( aboutToShow() ), this, SLOT( slotMenuAboutToShow() ) );
  connect( mExecPopup, SIGNAL( aboutToHide() ), this, SLOT( slotMenuAboutToHide() ) );

  mExecPopup->setItemEnabled( mUnlockId, false );
  lStatus->setPixmap( QPixmap() );
  lKind->setPixmap( QPixmap() );
}

void PositionViewWidget::setDocPosition( DocPositionBase *dp, KLocale* loc )
{
  if( ! dp ) return;
  mLocale = loc;

  if( dp->type() == DocPositionBase::Position ) {
    DocPosition *pos = static_cast<DocPosition*>(dp);
    m_skipModifiedSignal = true;
    // m_labelPosition->setText( QString("%1.").arg( mPositionPtr->position() ) );

    m_teFloskel->setText( pos->text() );

    m_sbAmount->setValue( pos->amount() );
    m_cbUnit->setCurrentText( pos->unit().einheitSingular() );
    m_sbUnitPrice->setValue( pos->unitPrice().toDouble() );

    const QString currSymbol = mLocale->currencySymbol();
    m_sbUnitPrice->setPrefix( currSymbol + " " );

    lStatus->hide();
    lKind->hide();

    AttributeMap amap = dp->attributes();
    if ( amap.contains( DocPosition::Kind ) ) {
      Attribute kind = amap[DocPosition::Kind];
      const QString kindStr = kind.value().toString();
      if ( kindStr == kindString( Alternative ) ) {
        slotSetPositionAlternative();
      } else if ( kindStr == kindString( Demand ) ) {
        slotSetPositionDemand();
      } else {
        kdDebug() << "Unknown position kind!" << endl;
      }
    }
    mPositionPtr = dp;
    slotSetOverallPrice( currentPrice() );

    m_skipModifiedSignal = false;
  }
}

void PositionViewWidget::slotExecButtonPressed()
{
  kdDebug() << "Opening Context Menu over exec button" << endl;

  // set bg-color
  mExecPopup->popup( QWidget::mapToGlobal( pbExec->pos() ) );

}

void PositionViewWidget::slotMenuAboutToShow()
{
  // setPaletteBackground( QColor( "blue" ) );
  setBackgroundMode( Qt::PaletteMid );
}

void PositionViewWidget::slotMenuAboutToHide()
{
  kdDebug() << "Set normal again" << endl;
  setBackgroundMode( Qt::PaletteBackground );
}

void PositionViewWidget::slotLockPosition( )
{
  slotSetState( Locked );
}

void PositionViewWidget::slotUnlockPosition( )
{
  slotSetState( Active );
}

void PositionViewWidget::slotEnableKindMenu( bool s )
{
  mStateSubmenu->setEnabled( s );
}

QString PositionViewWidget::stateString( const State& state ) const
{
  QString str;

  if( state == Active ) {
    str = i18n( "Active" );
  } else if( state == New ) {
    str = i18n( "New" );
  } else if( state == Deleted ) {
    str = i18n( "Deleted" );
  } else if( state == Locked ) {
    str = i18n( "Locked" );
  } else {
    str = i18n( "Unknown" );
  }
  return str;
}

void PositionViewWidget::slotSetState( State state )
{
  mState = state;
  kdDebug() << "Setting new widget state " << stateString( state ) << endl;
  if( state == Active ) {
    mExecPopup->setItemEnabled( mLockId, true);
    mExecPopup->setItemEnabled( mUnlockId, false );

    lStatus->hide();
    lStatus->setPixmap( QPixmap() );
    mToDelete = false;
    slotSetEnabled( true );
  } else if( state == New ) {
    lStatus->setPixmap( SmallIcon( "filenew" ) );
    lStatus->show();
  } else if( state == Deleted ) {
    lStatus->setPixmap( SmallIcon( "remove" ) );
    lStatus->show();
    mToDelete = true;
    slotSetEnabled( false );
  } else if( state == Locked ) {
    mExecPopup->setItemEnabled( mLockId, false );
    mExecPopup->setItemEnabled( mUnlockId, true );
    slotSetEnabled( false );
    lStatus->setPixmap( SmallIcon( "encrypted" ) );
    lStatus->show();
  }
}

void PositionViewWidget::setOrdNumber( int o )
{
  mOrdNumber = o;
  m_labelPosition->setText( QString("%1.").arg( mOrdNumber ) );
}

void PositionViewWidget::slotSetEnabled( bool doit )
{
  if( !doit ) {
    m_sbAmount->setEnabled( false );
    m_sbUnitPrice->setEnabled( false );
    m_labelPosition->setEnabled( false );
    m_teFloskel->setEnabled( false );
    m_sumLabel->setEnabled( false );
    m_cbUnit->setEnabled( false );
  } else {
    m_sbAmount->setEnabled( true );
    m_sbUnitPrice->setEnabled( true );
    m_labelPosition->setEnabled( true );
    m_teFloskel->setEnabled( true );
    m_sumLabel->setEnabled( true );
    m_cbUnit->setEnabled( true );
  }
}

Geld PositionViewWidget::currentPrice()
{
  Geld sum;
  if ( mKind == Normal ) {
    double amount = m_sbAmount->value();
    Geld g( m_sbUnitPrice->value() );
    sum = g * amount;
  }
  return sum;
}

Geld PositionViewWidget::unitPrice()
{
  Geld p(  m_sbUnitPrice->value() );
  return p;
}

void PositionViewWidget::slotRefreshPrice()
{
  const Geld sum = currentPrice();
  slotSetOverallPrice( sum );
  emit priceChanged( sum );
}

void PositionViewWidget::slotSetOverallPrice( Geld g )
{
  if ( mPositionPtr )
    m_sumLabel->setText( g.toString( mLocale ) );
}

void PositionViewWidget::slotModified()
{
  // if( mModified ) return;
    if( m_skipModifiedSignal ) return;
    kdDebug() << "Modified Position!" << endl;
    QColor c( "red" );
    m_labelPosition->setPaletteForegroundColor( c );
    mModified = true;
    emit positionModified();
}

PositionViewWidget::~PositionViewWidget()
{
}

PositionViewWidgetList::PositionViewWidgetList()
  : QPtrList<PositionViewWidget>()
{
  setAutoDelete( true );
}

PositionViewWidget* PositionViewWidgetList::widgetFromPosition( DocPositionGuardedPtr ptr)
{
  PositionViewWidget *pvw = 0;

  for( pvw = first(); pvw; pvw = next() ) {
    if( pvw->position() == ptr ) return pvw;
  }
  return 0;
}

Geld PositionViewWidgetList::nettoPrice()
{
  PositionViewWidget *pvw = 0;
  Geld res;

  for( pvw = first(); pvw; pvw = next() ) {
    res += pvw->currentPrice();
  }
  return res;
}

void PositionViewWidget::slotSetPositionNormal()
{
  lKind->hide();
  lKind->setPixmap( QPixmap() );
  mKind = Normal;

  cleanKindString();
  slotRefreshPrice();
  emit positionModified();
}

void PositionViewWidget::cleanKindString()
{
  QString current = m_teFloskel->text();
  bool touched = false;

  if ( current.startsWith( kindLabel( Alternative ) ) ) {
    current.remove( 0, QString( kindLabel( Alternative ) ).length() );
    touched = true;
  } else if ( current.startsWith( kindLabel( Demand ) ) ) {
    current.remove( 0, QString( kindLabel( Demand ) ).length() );
    touched = true;
  }

  if ( touched ) {
    m_teFloskel->setText( current );
  }
}

void PositionViewWidget::slotSetPositionAlternative()
{
  lKind->show();
  QToolTip::add( lKind, i18n( "This is an alternative position."
                              " Use the position toolbox to change." ) );
  lKind->setPixmap( SmallIcon( "alternative" ) );
  mKind = Alternative;
  slotRefreshPrice();

  cleanKindString();

  m_teFloskel->insertAt( kindLabel( Alternative ), 0, 0 );

  emit positionModified();
}

void PositionViewWidget::slotSetPositionDemand()
{
  lKind->show();
  QToolTip::add( lKind, i18n( "This is a as required position. "
                              "Use the position toolbox to change." ) );
  lKind->setPixmap( SmallIcon( "demand" ) );
  mKind = Demand;
  slotRefreshPrice();

  cleanKindString();
  m_teFloskel->insertAt( kindLabel( Demand ), 0, 0 );

  emit positionModified();
}

// The technical label
QString PositionViewWidget::kindString( Kind k ) const
{
  Kind kind = k;

  if ( kind == Invalid ) kind = mKind;

  if ( kind == Normal )      return QString::fromLatin1( "Normal" );
  if ( kind == Demand )      return QString::fromLatin1( "Demand" );
  if ( kind == Alternative ) return QString::fromLatin1( "Alternative" );

  return QString::fromLatin1( "unknown" );
}

// The label that is prepended to a positions text
QString PositionViewWidget::kindLabel( Kind k ) const
{
  Kind kind = k;

  if ( kind == Invalid ) kind = mKind;

  QString re;
  if ( kind == Normal ) {
    re = KraftSettings::self()->normalLabel();
    if ( re.isEmpty() ) re = i18n( "Normal" );
  }
  if ( kind == Demand ) {
    re = KraftSettings::self()->demandLabel();
    if ( re.isEmpty() ) re = i18n( "Demand" );
  }
  if ( kind == Alternative ) {
    re = KraftSettings::self()->alternativeLabel();
    if ( re.isEmpty() ) re = i18n( "Alternative" );
  }

  if ( ! re.endsWith( ": " ) ) {
    re += QString::fromLatin1( ": " );
  }
  return re;
}
#include "positionviewwidget.moc"

