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

#include <klocalizedstring.h>
#include <QDebug>
#include <QTreeWidget>

#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>

FilterHeader::FilterHeader( QTreeWidget *listView, QWidget *parent )
  : QWidget( parent ),
    _treeWidget(listView)
{
  QBoxLayout *topLayout = new QVBoxLayout;
  setLayout( topLayout );

  mTitleLabel = new QLabel();
  topLayout->addWidget( mTitleLabel );

  QBoxLayout *filterLayout = new QHBoxLayout;
  topLayout->addLayout( filterLayout );
  QLabel *label = new QLabel( i18n("Search:"));
  filterLayout->addWidget( label );

  mSearchLine = new QLineEdit( this );
  mSearchLine-> setClearButtonEnabled(true);
  connect( mSearchLine, SIGNAL(textChanged(QString) ),
    SLOT( slotTextChanged(QString) ) );
  filterLayout->addWidget( mSearchLine );
}

void FilterHeader::slotTextChanged( const QString& filter )
{
    if( ! _treeWidget ) {
        return;
    }
    QTreeWidgetItemIterator it(_treeWidget);
    while (*it) {
        // items without parent are root items. Never hide.
        if( (*it)->parent() ) {
            bool showIt = false;
            for(int i = 0; !showIt && i < (*it)->columnCount(); i++) {
                if( (*it)->text(i).contains(filter, Qt::CaseInsensitive)) {
                    showIt = true;
                }
            }
            (*it)->setHidden(!showIt);
        }
        ++it;
    }
}

void FilterHeader::setListView( QTreeWidget* view )
{
  _treeWidget = view;
}

void FilterHeader::clear()
{
  mSearchLine->clear();
}



