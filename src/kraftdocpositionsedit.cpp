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
  _myWidget = new QWidget;
  _myWidget->setAutoFillBackground(false);
  _layout = new QVBoxLayout;
  _layout->setAlignment(Qt::AlignTop);
  _layout->setSizeConstraint( QLayout::SetMinAndMaxSize );
  _layout->setContentsMargins( 0,0,0,0 );
  _layout->setSpacing(0);
  _myWidget->setLayout(_layout);
  setWidget(_myWidget);
  setWidgetResizable(true);
  _myWidget->resize(0,0);
  _myWidget->setMinimumHeight(0);
  _myWidget->setMaximumHeight(0);
  _myWidget->setContentsMargins(0, 0, 0, 0);
}

KraftViewScroll::~KraftViewScroll()
{
  // if the help widget is shown, myWidget is not owned by the scroll area
  if (widget() != _myWidget) {
    delete _myWidget;
  }
}

// Shows the help widget instead of the item list. The scroll area takes
// ownership of help. _myWidget is taken out, but not deleted.
void KraftViewScroll::showHelpWidget( QWidget *help )
{
  if (widget() == _myWidget) {
    takeWidget();
  }
  setWidget(help);
}

// Deletes the help widget and puts the item list back in place.
void KraftViewScroll::hideHelpWidget()
{
  if (widget() != _myWidget) {
    QWidget *w = takeWidget();
    if (w) {
      w->deleteLater();
    }
    setWidget(_myWidget);
    setWidgetResizable(true);
  }
}

void KraftViewScroll::addChild( QWidget *child, int index )
{
    int y1 = _myWidget->height();
    _layout->insertWidget(index, child);
    int y2 = y1+child->height();
    _myWidget->resize( child->width(), y2);
    _myWidget->setMinimumHeight(y2);
    _myWidget->setMaximumHeight(y2);
}

void KraftViewScroll::removeChild( PositionViewWidget *child )
{
  _layout->removeWidget( child ); // from the scrollview
}

void KraftViewScroll::moveChild( PositionViewWidget *child, int index)
{
  _layout->removeWidget(child);
  _layout->insertWidget(index, child);
}

int KraftViewScroll::indexOf(PositionViewWidget *child)
{
  return _layout->indexOf(child);
}

// #########################################################

KraftDocPositionsEdit::KraftDocPositionsEdit( QWidget *parent )
  : KraftDocEdit( parent )
{
  QBoxLayout *topLayout = new QVBoxLayout();
  topLayout->setContentsMargins(0, 0, 0, 0);
//TODO PORT QT5   topLayout->setSpacing( 0 ); // QDialog::spacingHint() );

  QHBoxLayout *upperHBoxLayout = new QHBoxLayout;
  //upperHBoxLayout->setFrameStyle( QFrame::Box + QFrame::Sunken );
//TODO PORT QT5   upperHBoxLayout->setMargin( QDialog::marginHint()/2 );
  topLayout->addLayout( upperHBoxLayout );

  QPushButton *button = new QPushButton( i18n("Add Item…") );
  connect( button, SIGNAL( clicked() ), SIGNAL( addPositionClicked() ) );
  button->setToolTip( i18n( "Add a normal item to the document manually." ) );
  upperHBoxLayout->addWidget(button);
  upperHBoxLayout->setSpacing( 3 );

  _discountBtn = new QPushButton( i18n("Add Discount Item") );
  connect( _discountBtn, SIGNAL( clicked() ), SIGNAL( addExtraClicked() ) );
  upperHBoxLayout->addWidget(_discountBtn);
  _discountBtn->setToolTip( i18n( "Adds an item to the document that allows discounts on other items in the document" ) );

  QWidget *spaceEater = new QWidget( );
  spaceEater->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );
  upperHBoxLayout->addWidget(spaceEater);

  _positionScroll = new KraftViewScroll( this );
  topLayout->addWidget( _positionScroll );

  setTitle( i18n( "Document Items" ) );
  setColor( "#9affa9" );
  setLayout(topLayout);
}

void KraftDocPositionsEdit::setDiscountButtonVisible( bool visible )
{
    _discountBtn->setVisible( visible );
}
