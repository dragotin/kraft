/***************************************************************************
      dbinitdialog.h  - Dialog to init and update the database schema
                             -------------------
    begin                : Tue Nov 24 2009
    copyright            : (C) 2009 by Klaas Freitag
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
#include <QtGui>

#include <kdebug.h>
#include "dbinitdialog.h"

DbInitDialog::DbInitDialog( QWidget *parent )
  : KDialog( parent )
{
  QWidget *w = new QWidget( this );
  setMainWidget(w);

  setupUi( w );

  setCaption( i18n("Database Setup") );
  setModal( true );
  setButtons( User1 | Close );
  setDefaultButton( User1 );
  setButtonText( User1, i18n("Start"));
  setButtonToolTip(User1,i18n("Start the Database Operation"));
  showButtonSeparator( true);

}

void DbInitDialog::slotSetStatusText( const QString& msg )
{
  mSummary->setText( msg );
}

void DbInitDialog::slotSetInstructionText( const QString& text )
{
  mInfoText->setText( text );
}

void DbInitDialog::slotProcessedOneCommand( bool )
{
  int cnt = mOverallProgress->value();
  setCurrentOverallCount( cnt +1 );

  cnt =mDetailProgress->value();
  setCurrentDetailCount( cnt+1 );
}

void DbInitDialog::setOverallCount( int cnt )
{
  mOverallProgress->setMinimum(0);
  mOverallProgress->setMaximum( cnt );
  mOverallProgress->setValue( 0 );
  setCounterText( mOverallStatus, 0, cnt );
}

void DbInitDialog::setCurrentOverallCount( int cnt )
{
  mOverallProgress->setValue( cnt );
  setCounterText( mOverallStatus, cnt, mOverallProgress->maximum() );
}

void DbInitDialog::setCounterText( QLabel* label, int current, int max )
{
  if( !label ) return;
  label->setText( i18n("%1/%2").arg(current).arg(max) );
}

void DbInitDialog::setDetailOverallCnt( int cnt )
{
  mDetailProgress->setValue( 0 );
  mDetailProgress->setMinimum( 0 );
  mDetailProgress->setMaximum( cnt );
  setCounterText( mDetailStatus, 0, cnt );
}

void DbInitDialog::setCurrentDetailCount( int cnt )
{
  mDetailProgress->setValue( cnt );
  setCounterText( mDetailStatus, cnt, mDetailProgress->maximum());
}
