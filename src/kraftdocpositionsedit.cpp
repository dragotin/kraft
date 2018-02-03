/***************************************************************************
         kraftdocpositionsedit.cpp - Doc item editor widget
                             -------------------
    begin                :
    copyright            : (C) 2003 by Klaas Freitag
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

#include "kraftdocpositionsedit.h"

#include <QLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QBoxLayout>
#include <QToolTip>

#include <QPushButton>
#include <QLocale>
#include <QDialog>

#include <KLocalizedString>

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
//TODO PORT QT5   topLayout->setSpacing( 0 ); // QDialog::spacingHint() );

  QHBoxLayout *upperHBoxLayout = new QHBoxLayout;
  //upperHBoxLayout->setFrameStyle( QFrame::Box + QFrame::Sunken );
//TODO PORT QT5   upperHBoxLayout->setMargin( QDialog::marginHint()/2 );
  topLayout->addLayout( upperHBoxLayout );

  QPushButton *button = new QPushButton( i18n("Add Item...") );
  connect( button, SIGNAL( clicked() ), SIGNAL( addPositionClicked() ) );
  button->setToolTip( i18n( "Add a normal item to the document manually." ) );
  upperHBoxLayout->addWidget(button);
  upperHBoxLayout->setSpacing( 3 );

  m_discountBtn = new QPushButton( i18n("Add Discount Item") );
  connect( m_discountBtn, SIGNAL( clicked() ), SIGNAL( addExtraClicked() ) );
  upperHBoxLayout->addWidget(m_discountBtn);
  m_discountBtn->setToolTip( i18n( "Adds an item to the document that allows discounts on other items in the document" ) );

#if 0 // commented, rarely used feature.
  button = new QPushButton( i18n("Import Items...") );
  connect( button, SIGNAL( clicked() ), SIGNAL( importItemsClicked() ) );
  upperHBoxLayout->addWidget(button);
  button->setToolTip( i18n( "Opens a dialog where multiple items can be imported from a text file." ) );
#endif
  QWidget *spaceEater = new QWidget( );
  spaceEater->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );
  upperHBoxLayout->addWidget(spaceEater);

  m_positionScroll = new KraftViewScroll( this );
  topLayout->addWidget( m_positionScroll );

  setTitle( i18n( "Document Items" ) );
  setColor( "#9affa9" );
  setLayout(topLayout);
}

void KraftDocPositionsEdit::setDiscountButtonVisible( bool visible )
{
    m_discountBtn->setVisible( visible );
}
