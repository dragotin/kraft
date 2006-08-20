/***************************************************************************
       docoverviewwidget  - A widget to show an overview at top of the
                    document editor window
                             -------------------
    begin                : 2006-08-12
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

// include files
#include "docoverviewwidget.h"
#include "kraftdoc.h"

#include <qlabel.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qgrid.h>
#include <qvbox.h>

#include <kdialog.h>

#include "geld.h"

/**
 *
 */

DocOverviewWidget::DocOverviewWidget( QWidget *parent )
 :QVBox( parent )
{
  setMargin( 3 ); // KDialog::marginHint() );
  // setFrameStyle( Box + Plain ); // HLine + Sunken );

  setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Maximum );
  // the VBox contains first a HBox with all the information and a detail header.

  QFrame *f = new QFrame( this );
  f->setFrameStyle( HLine + Sunken );
  f->setLineWidth( 1 );

  QHBox *hbox = new QHBox( this );
#if 0
  hbox->setFrameStyle( QFrame::Panel | QFrame::Raised );
  hbox->setLineWidth( 2 );
#endif
  hbox->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Maximum );

  QButtonGroup *bg = new QButtonGroup( 1,  Horizontal, /* i18n( "Navigation" ) */ hbox );
  {
    bg->setInsideMargin( 2 );
    bg->setInsideSpacing( 2 );
    bg->setExclusive( true );

    connect( bg, SIGNAL( clicked( int ) ), SIGNAL( switchToPage( int ) ) );
    QPushButton *pb1 = new QPushButton( i18n( "Header" ),  bg );
    pb1->setToggleButton( true );
    QPushButton *pb2 = new QPushButton( i18n( "Positions" ),  bg );
    pb2->setToggleButton( true );
    QPushButton *pb3 = new QPushButton( i18n( "Footer" ),  bg );
    pb3->setToggleButton( true );
    mButtonGroup = bg;
  }

  mDocShort = new QLabel( hbox );

  // The following vbox rules the space above the numbers:
  QVBox *vb = new QVBox( hbox );
  //QLabel *l = new QLabel( i18n( "Numbers:" ), vb );
  QGrid *grid = new QGrid( 2, Qt::Horizontal,  vb );
  grid->setSpacing( 3 );

  ( void ) new QLabel( i18n( "Netto:" ), grid );
  mNettoSum = new QLabel( i18n( "0 EUR" ), grid );
  mNettoSum->setAlignment( Qt::AlignRight );
  mVatLabel = new QLabel( i18n( "+ VAT (%1%%):" ).arg( 16 ), grid );
  mVat = new QLabel( i18n( "0 EUR" ), grid );
  mVat->setAlignment( Qt::AlignRight );

  ( void ) new QLabel( i18n( "<b>Brutto:</b>" ), grid );
  mBrutto = new QLabel( i18n( "0 EUR" ), grid );
  mBrutto->setAlignment( Qt::AlignRight );
  QWidget *w = new QWidget( vb );
  w->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::MinimumExpanding );


}

void DocOverviewWidget::slotSetSums( Geld netto, double vat )
{
  mNettoSum->setText( netto.toString() );

  mVatLabel->setText( i18n( "+ VAT (%1%):" ).arg( vat ) );
  Geld vatSum = netto * vat / 100;
  mVat->setText( vatSum.toString () );
  vatSum += netto;
  mBrutto->setText( QString( "<b>%1</b>" ).arg( vatSum.toString() ) );

}


void DocOverviewWidget::slotSelectPageButton( int id )
{
  mButtonGroup->setButton( id );
  emit switchToPage( id );
}


DocOverviewWidget::~DocOverviewWidget()
{

}

void DocOverviewWidget::setDocPtr( DocGuardedPtr doc )
{
  if ( doc != mDoc ) {
    mDoc = doc;
  }
}

#include "docoverviewwidget.moc"
/* END */

