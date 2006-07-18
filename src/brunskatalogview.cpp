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
#include <qlistview.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qsplitter.h>
#include <kdebug.h>
#include <klocale.h>

#include "katalogman.h"
#include "brunskatalogview.h"
#include "brunskataloglistview.h"
#include "brunsrecord.h"
#include "brunskatalog.h"
#
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
    kdDebug() << "Creating new Bruns-Listview" << endl;
    QSplitter *split = new QSplitter(Qt::Vertical, w);
    
    m_brunsListView = new BrunsKatalogListView(split);
    box->addWidget(split); // m_brunsListView);
#if 0
    m_detailLabel = new QLabel(w);
    box->addWidget(m_detailLabel);
    m_detailLabel->setText(i18n("Plant Details (Sizes, Root Forms etc.):"));
#endif

    m_details = new KListView(split);
    m_details->addColumn( i18n( "Matchcode" ) );
    m_details->addColumn( i18n( "Form" ) );
    m_details->addColumn( i18n( "Form Add" ) );
    m_details->addColumn( i18n( "Wuchs" ) );
    m_details->addColumn( i18n( "Root" ) );
    m_details->addColumn( i18n( "Quality"));
    m_details->addColumn( i18n( "Group" ));
    // box->addWidget(m_details);

    connect( m_brunsListView, SIGNAL(selectionChanged(QListViewItem*)),
             this, SLOT(slPlantSelected(QListViewItem* )));
    
}

Katalog* BrunsKatalogView::getKatalog( const QString& name )
{
    kdDebug() << "GetKatalog of bruns!" << endl;
    Katalog *k = KatalogMan::getKatalog( name );
    if( ! k ) {
        k = new BrunsKatalog( name );
        KatalogMan::registerKatalog( k );
    }
    return k;
}


void BrunsKatalogView::slPlantSelected( QListViewItem *item)
{
    if( ! item ) return;

    m_details->clear();
    
    BrunsRecord rec = m_brunsListView->getRecord(item);

    BrunsSizeList sizes = rec.getSizes();
    BrunsSizeList::iterator it;
    for( it = sizes.begin(); it != sizes.end(); ++it ) {
      KListViewItem *guiItem = new KListViewItem(m_details, (*it).getPrimMatchcode() );
      
      const QStringList list = BrunsKatalog::formatQuality( (*it) );
      int i = 1;
      for ( QStringList::ConstIterator listIt = list.begin(); listIt != list.end(); ++listIt ) {
        guiItem->setText( i++, (*listIt) );
      }
        // kdDebug() << "showing new plant detail item" << endl;
    }

    
}

#include "brunskatalogview.moc"
