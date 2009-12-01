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

#include "dbinitdialog.h"

DbInitDialog::DbInitDialog( QWidget *parent )
  : KDialog( parent )
{
  QWidget *w = new QWidget( this );
  setMainWidget(w);

  setupUi( w );

  setCaption( i18n("Database Setup") );
  setModal( true );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  showButtonSeparator( true);

}

void DbInitDialog::slotSetStatusText( const QString& msg )
{
  mSummary->setText( msg );
}

void DbInitDialog::slotProcessedOneCommand( bool )
{
  int cnt = mOverallProgress->value();
  mOverallProgress->setValue( cnt +1 );

  cnt =mDetailProgress->value();
  mDetailProgress->setValue( cnt +1 );
}

void DbInitDialog::setOverallCount( int cnt )
{
  mOverallProgress->setMinimum(0);
  mOverallProgress->setMaximum( cnt );
  mOverallProgress->setValue( 0 );
}

void DbInitDialog::setCurrentOverallCount( int cnt )
{
  mOverallProgress->setValue( cnt );

}

void DbInitDialog::setDetailOverallCnt( int cnt )
{
  mDetailProgress->setValue( 0 );
  mDetailProgress->setMinimum( 0 );
  mDetailProgress->setMaximum( cnt );
}

void DbInitDialog::setCurrentDetailCount( int cnt )
{
  mDetailProgress->setValue( cnt );
}
