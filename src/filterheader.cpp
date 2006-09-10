/***************************************************************************
                            filterheader.cpp
                             -------------------
    copyright            : (C) 2005 by Cornelius Schumacher
                           (C) 2005 by Klaas Freitag
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

#include "filterheader.h"

#include <klistview.h>
#include <klocale.h>
#include <kdialog.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kiconloader.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>

CountingSearchLine::CountingSearchLine( QWidget *parent, KListView *listView )
  : KListViewSearchLine( parent, listView )
{
}

void CountingSearchLine::updateSearch( const QString &s )
{
  KListViewSearchLine::updateSearch( s );

  emit searchCountChanged();
}

int CountingSearchLine::searchCount()
{
  int count = 0;
  QListViewItem *item;
  for( item = listView()->firstChild(); item; item = item->nextSibling() ) {
    if ( item->isVisible() ) ++count;
  }
  return count;
}


FilterHeader::FilterHeader( KListView *listView, QWidget *parent )
  : QWidget( parent ), mListView( listView ), mItemNameNone( i18n("No Items") ),
    mItemNameOne( i18n("1 Item") ),
    mItemNameMultiple( i18n("%1 of %2 Items") )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( KDialog::marginHint() );

  mTitleLabel = new QLabel( this );
  topLayout->addWidget( mTitleLabel );

  QBoxLayout *filterLayout = new QHBoxLayout( topLayout );

  QLabel *label = new QLabel( i18n("Search:"), this );
  filterLayout->addWidget( label );

  mSearchLine = new CountingSearchLine( this, listView );
  connect( mSearchLine, SIGNAL( searchCountChanged() ),
    SLOT( setTitleLabel() ) );
  filterLayout->addWidget( mSearchLine );

  QPushButton *removeButton = new QPushButton( this );
  removeButton->setIconSet( KApplication::reverseLayout() ?
    SmallIconSet("locationbar_erase") : SmallIconSet( "clear_left" ) );
  filterLayout->addWidget( removeButton );
  connect( removeButton, SIGNAL( clicked() ), SLOT( clear() ) );

  setTabOrder( mSearchLine, listView );

  setTitleLabel();
}

void FilterHeader::setItemName( const QString &none, const QString &one,
      const QString &multiple )
{
  mItemNameNone = none;
  mItemNameOne = one;
  mItemNameMultiple = multiple;

  setTitleLabel();
}

void FilterHeader::setListView( KListView* view )
{
  mSearchLine->setListView( view );
}

void FilterHeader::clear()
{
  mSearchLine->setText( QString::null );

  setTitleLabel();
}

void FilterHeader::setTitleLabel()
{
  int total = 0;

  if ( mListView ) total = mListView->childCount();

  QString txt;

  if ( total == 0 ) txt = mItemNameNone;
  else {
    int current = mSearchLine->searchCount();

    if ( total == 1 && current == 1 ) txt = mItemNameOne;
    else {
      txt = mItemNameMultiple.arg( current ).arg( total );
    }
  }

  mTitleLabel->setText( "<b>" + txt + "</b>" );
}

void FilterHeader::showCount( bool show )
{
  if ( show ) mTitleLabel->show();
  else mTitleLabel->hide();
}

#include "filterheader.moc"
