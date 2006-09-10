/***************************************************************************
             brunskataloglistview  - template katalog listview.
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
#include <qmap.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

#include "brunskatalog.h"
#include "brunskataloglistview.h"
#include "katalogman.h"
#include "docposition.h"
#include "kataloglistview.h"


BrunsKatalogListView::BrunsKatalogListView( QWidget *w )
    : KatalogListView( w )
{
    addColumn( i18n("Bot. Name"));
    addColumn( i18n("Dt. Name"));
    addColumn( i18n("Art-Ident"));
    addColumn( i18n("Art-Match"));
}


BrunsKatalogListView::~BrunsKatalogListView()
{

}

void BrunsKatalogListView::addCatalogDisplay( const QString& katName )
{
    KatalogListView::addCatalogDisplay(katName);
    Katalog *k = KatalogMan::self()->getKatalog( katName );
    BrunsKatalog *catalog = static_cast<BrunsKatalog*>( k );
    if( ! catalog ) {
        kdDebug() << "No catalog in listview available!" << endl;
        return;
    }
    kdDebug() << "setting up chapters!" << endl;
    setupChapters();

    const QStringList chapters = catalog->getKatalogChapters();
    for ( QStringList::ConstIterator it = chapters.begin(); it != chapters.end(); ++it ) {
        QString chapter = *it;
        KListViewItem *katItem = chapterItem(chapter);
        kdDebug() << "KatItem is " << katItem << " for chapter " << chapter << endl;

        // hole alle Brunsrecords per Chapter und mach weiter....
        BrunsRecordList *records = catalog->getRecordList(chapter);

        if( records ) {
            BrunsRecordList::iterator it;
            for ( it = records->begin(); it != records->end(); ++it ) {
                BrunsRecord rec((*it) );
                KListViewItem *recItem = new KListViewItem( katItem, rec.getLtName(),
                        rec.getDtName(),
                        QString::number(rec.getArtId()),
                        rec.getArtMatch());
                m_itemMap[recItem] = rec;
            }
        }
    }
}

void BrunsKatalogListView::setupChapters()
{
  Katalog *catalog = KatalogMan::self()->getKatalog(m_catalogName);
  if( ! catalog ) {
    kdWarning() << "No catalog in setupChapters" << endl;
    return;
  }

  if( ! m_root ) {
    kdDebug() << "Creating root item!" <<  endl;
    m_root = new KListViewItem(this, catalog->getName());
    m_root->setPixmap(0, SmallIcon("gear"));
    m_root->setOpen(true);
  }

  KListViewItem *topItem = new KListViewItem( m_root, i18n( "Fruits" ) );
  topItem->setPixmap(0, getCatalogIcon());
  m_topFolderMap[ Fruits ] = topItem;

  topItem = new KListViewItem( m_root, i18n( "Azaleen and Rhododendren" ) );
  topItem->setPixmap(0, getCatalogIcon());
  m_topFolderMap[ Rhodos ] = topItem;

  topItem = new KListViewItem( m_root, i18n( "Roses" ) );
  topItem->setPixmap(0, getCatalogIcon());
  m_topFolderMap[ Roses ] = topItem;

  topItem = new KListViewItem( m_root, i18n( "Stauden" ) );
  topItem->setPixmap(0, getCatalogIcon());
  m_topFolderMap[ Stauden ] = topItem;

  topItem = new KListViewItem( m_root, i18n( "Sonstige" ) );
  topItem->setPixmap(0, getCatalogIcon());
  m_topFolderMap[ Etc ] = topItem;

  const QStringList chapters = catalog->getKatalogChapters();

  // weiterhier: sortiere chapter unter die top folder.
  for ( QStringList::ConstIterator it = chapters.begin(); it != chapters.end(); ++it ) {
    const QString chapter = *it;
    KListViewItem *topFolderItem = m_topFolderMap[ Etc ];

    if( chapter == "Aepfel" ||
        chapter == "Birnen" ||
        chapter.contains( "beeren" ) ||
        chapter.contains( "nuesse" ) ||
        chapter.contains( "Holunder" ) ||
        chapter.contains( "Pfirsiche" ) ||
        chapter.contains( "Pflaumen" ) ||
        chapter.contains( "Quitten" ) ||
        chapter.contains( "irschen" ) ) {
      topFolderItem = m_topFolderMap[ Fruits ];
    } else if( chapter.contains( "Rhododendron" ) ||
               chapter.contains( "Azale" ) ) {
      topFolderItem = m_topFolderMap[ Rhodos ];
    } else if( chapter.contains( "rose" ) ) {
      topFolderItem = m_topFolderMap[ Roses ];
    } else if( chapter.contains( "tauden" ) ||
               chapter == "Farne" ||
               chapter == "Graeser" ) {
      topFolderItem = m_topFolderMap[ Stauden ];
    } else if( chapter.contains( "Koniferen" ) ) {
      topFolderItem = m_root;
    } else if( chapter.contains( "Laubgehoelze" ) ) {
      topFolderItem = m_root;
    } else {
      // be in etc.
      kdDebug() << "Undetected catalog " << chapter << endl;
    }

    if( chapter != "0" ) {
      KListViewItem *katItem = new KListViewItem( topFolderItem, chapter );
      katItem->setPixmap( 0, getCatalogIcon() );
      m_catalogDict.insert( catalog->chapterID(chapter), katItem );
    }
  }
}

BrunsRecord BrunsKatalogListView::getRecord( QListViewItem *it )
{
  BrunsRecord re;
  if( it ) {
    return m_itemMap[it];
  }
  return re;
}


DocPosition BrunsKatalogListView::currentItemToDocPosition()
{
  DocPosition pos;

  return pos;
}
