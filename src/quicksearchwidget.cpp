/*
    This file is part of KAddressBook.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "quicksearchwidget.h"

#include <QtCore/QTimer>
#include <QtGui/QKeyEvent>
#include <QtGui/QVBoxLayout>

#include <klineedit.h>
#include <klocale.h>

QuickSearchWidget::QuickSearchWidget( QWidget *parent )
  : QWidget( parent )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setMargin( 0 );

  mEdit = new KLineEdit;
  mEdit->setClickMessage( i18nc( "Search contacts in list", "Search" ) );
  mEdit->setClearButtonShown( true );

  mEdit->installEventFilter( this );

  layout->addWidget( mEdit );

  mTimer = new QTimer( this );

  connect( mEdit, SIGNAL( textChanged( const QString& ) ), SLOT( resetTimer() ) );
  connect( mTimer, SIGNAL( timeout() ), SLOT( delayedTextChanged() ) );
}


QuickSearchWidget::~QuickSearchWidget()
{
}

QSize QuickSearchWidget::sizeHint() const
{
  const QSize size = mEdit->sizeHint();
  return QSize( 200, size.height() );
}

void QuickSearchWidget::resetTimer()
{
  mTimer->stop();
  mTimer->start( 500 );
}

void QuickSearchWidget::delayedTextChanged()
{
  mTimer->stop();
  emit filterStringChanged( mEdit->text() );
}

void QuickSearchWidget::keyPressEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Down ) {
    event->accept();
    delayedTextChanged();
    emit arrowDownKeyPressed();
    return;
  }

  QWidget::keyPressEvent( event );
}

