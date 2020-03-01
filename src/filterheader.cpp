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
#include <QAction>
#include <QPushButton>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>

FilterHeader::FilterHeader(QWidget *parent , QTreeWidget *listView)
  : QWidget( parent ),
    _treeWidget(listView)
{
    QBoxLayout *filterLayout = new QHBoxLayout;
    setLayout(filterLayout);
    QLabel *label = new QLabel( i18n("&Search:"));
    filterLayout->addWidget( label );

    mSearchLine = new QLineEdit( this );
    mSearchLine->setClearButtonEnabled(true);

    label->setBuddy(mSearchLine);
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
        QTreeWidgetItem *item = (*it);
        if( item->parent() ) {
            bool showIt = false;
            for(int i = 0; !showIt && i < item->columnCount(); i++) {
                if( item->text(i).contains(filter, Qt::CaseInsensitive)) {
                    showIt = true;
                }
            }

            item->setHidden(!showIt);
            if( showIt && ! filter.isEmpty() ) {
                // Make sure that all the parent items are visible too
                QTreeWidgetItem *parent = nullptr, *child = item;
                while((parent = child->parent()) != nullptr) {
                    parent->setHidden(false);
                    if( !parent->isExpanded() ) {
                        parent->setExpanded(true);
                        _openedItems[parent] = 1;
                    }
                    child = parent;
                }
            }
            if (filter.isEmpty()) {
                for( auto item : _openedItems.uniqueKeys()) {
                    item->setExpanded(false);
                }
                _openedItems.clear();
            }
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



