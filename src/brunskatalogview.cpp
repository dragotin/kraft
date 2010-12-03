/***************************************************************************
                          brunskatalogview.cpp
                             -------------------
    begin                : 2005-07-26
    copyright            : (C) 2005 by Klaas Freitag
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

#include <QLabel>
#include <QStringList>
#include <QList>
#include <QSplitter>
#include <QTreeWidgetItem>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <kdebug.h>
#include <klocale.h>

#include "katalogman.h"
#include "brunskatalogview.h"
#include "brunskataloglistview.h"
#include "brunsrecord.h"
#include "brunskatalog.h"


BrunsKatalogView::BrunsKatalogView()
 : KatalogView(),
 m_brunsListView(0),
 m_details(0)
{

}


BrunsKatalogView::~BrunsKatalogView()
{

}

void BrunsKatalogView::createCentralWidget(QBoxLayout *box, QWidget *w)
{
    kDebug() << "Creating new Bruns-Listview" << endl;
    QSplitter *split = new QSplitter(Qt::Vertical, w);

    m_brunsListView = new BrunsKatalogListView(split);
    box->addWidget(split); // m_brunsListView);
    m_detailLabel = new QLabel(w);

    box->addWidget(m_detailLabel);
    m_detailLabel->setText(i18n("Plant Details (Sizes, Root Forms etc.):"));

    m_details = new QTreeWidget(split);
    m_details->setColumnCount( 7 );
    QStringList h;

    h << i18n( "Matchcode" );
    h << i18n( "Form" );
    h << i18n( "Form Add" );
    h << i18n( "Wuchs" );
    h << i18n( "Root" );
    h << i18n( "Quality");
    h << i18n( "Group" );
    m_details->setHeaderLabels( h );

    box->addWidget(m_details);

    connect( m_brunsListView, SIGNAL( currentItemChanged( QTreeWidgetItem *, QTreeWidgetItem* ) ),
             this, SLOT( slPlantSelected( QTreeWidgetItem*, QTreeWidgetItem* ) ) );

    KatalogView::createCentralWidget( box, w );

}

Katalog* BrunsKatalogView::getKatalog( const QString& name )
{
    kDebug() << "GetKatalog of bruns!" << endl;
    Katalog *k = KatalogMan::self()->getKatalog( name );
    if( ! k ) {
        k = new BrunsKatalog( name );
        KatalogMan::self()->registerKatalog( k );
    }
    return k;
}


void BrunsKatalogView::slPlantSelected( QTreeWidgetItem *item, QTreeWidgetItem*)
{
    if( ! item ) return;

    m_details->clear();

    BrunsRecord *rec = static_cast<BrunsRecord*>( m_brunsListView->itemData(item) );

    if ( ! rec ) return;
    BrunsSizeList sizes = rec->getSizes();
    BrunsSizeList::iterator it;
    QList<QTreeWidgetItem*> items;

    for( it = sizes.begin(); it != sizes.end(); ++it ) {
      QStringList list = BrunsKatalog::formatQuality( (*it) );
      list.prepend( (*it).getPrimMatchcode() );
      
      items.append( new QTreeWidgetItem( list ) );
        // kDebug() << "showing new plant detail item" << endl;
    }
    m_details->addTopLevelItems( items );
}

