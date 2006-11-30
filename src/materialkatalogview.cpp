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
#include "materialkatalogview.h"
#include "materialkataloglistview.h"
#include "stockmaterial.h"
#include "matkatalog.h"
#
MaterialKatalogView::MaterialKatalogView()
 : KatalogView(),
 m_materialListView(0),
 m_details(0)
{
}


MaterialKatalogView::~MaterialKatalogView()
{
}

void MaterialKatalogView::createCentralWidget( QBoxLayout *box, QWidget *w )
{
    m_materialListView = new MaterialKatalogListView( w );
    box->addWidget( m_materialListView );
}

Katalog* MaterialKatalogView::getKatalog( const QString& name )
{
    kdDebug() << "GetKatalog of material!" << endl;
    Katalog *k = KatalogMan::self()->getKatalog( name );
    if( ! k ) {
        k = new MatKatalog( name );
        KatalogMan::self()->registerKatalog( k );
    }
    return k;
}

#include "materialkatalogview.moc"
