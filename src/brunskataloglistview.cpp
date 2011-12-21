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
#include <kmenu.h>

#include "brunskatalog.h"
#include "brunskataloglistview.h"
#include "katalogman.h"
#include "docposition.h"
#include "kataloglistview.h"


BrunsKatalogListView::BrunsKatalogListView( QWidget *w )
  : KatalogListView( w )
{
  setColumnCount( 4 );
  QStringList headerLabels;
  headerLabels << i18nc( "Abbrev. for botanical name", "Bot. Name" );
  headerLabels << i18nc( "Abbrev. for localized name", "Name" );
  headerLabels << i18nc( "Abbrev. for article identification number", "Art. Id." );
  headerLabels << i18nc( "Abbrev. for article Matchcode", "Art. Match.");
  setHeaderLabels( headerLabels );

  contextMenu()->addTitle( QString("Bruns ") + i18n("Catalog"));
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
    kDebug() << "No catalog in listview available!" << endl;
    return;
  }
  kDebug() << "setting up chapters!" << endl;
  setupChapters();

  const QList<CatalogChapter> chapters = catalog->getKatalogChapters();
  foreach( CatalogChapter chapter, chapters ) {

    QTreeWidgetItem *katItem = 0; // FIXME - chapterItem(chapter.name());

    // fetch all Bruns-Records for the chapter
    BrunsRecordList *records = catalog->getRecordList(chapter);

    if( records ) {
      BrunsRecord *rec;

      QListIterator<BrunsRecord*> i(*records);
      i.toBack();
      while (i.hasPrevious()) {
        rec = i.previous();
        QStringList texts;
        texts << rec->getLtName();
        texts << rec->getDtName();
        texts << QString::number( rec->getArtId() );
        texts << rec->getArtMatch();
        QTreeWidgetItem *recItem = new QTreeWidgetItem( katItem, texts );
        if ( mCheckboxes ) {
          recItem->setCheckState(0, Qt::Unchecked);
        }
        m_dataDict.insert( recItem, rec );
      }
    }
  }
}

void BrunsKatalogListView::setupChapters()
{
  Katalog *catalog = KatalogMan::self()->getKatalog(m_catalogName);
  if( ! catalog ) {
    kWarning() << "No catalog in setupChapters" << endl;
    return;
  }

  if( ! m_root ) {
    kDebug() << "Creating root item!" <<  endl;
    m_root = new QTreeWidgetItem(this, QStringList(catalog->getName()));
    m_root->setIcon(0, SmallIcon("system-run"));
    m_root->setExpanded(true);
  }

  QPixmap icon = SmallIcon( "folder-documents");

  QTreeWidgetItem *topItem = new QTreeWidgetItem( m_root, QStringList(i18n( "Fruits" )) );
  topItem->setIcon(0, icon );
  m_topFolderMap[ Fruits ] = topItem;

  topItem = new QTreeWidgetItem( m_root, QStringList( i18n( "Azalea and Rhododendrons" ) ) );
  topItem->setIcon(0, icon);
  m_topFolderMap[ Rhodos ] = topItem;

  topItem = new QTreeWidgetItem( m_root, QStringList( i18n( "Roses" ) ) );
  topItem->setIcon(0, icon);
  m_topFolderMap[ Roses ] = topItem;

  topItem = new QTreeWidgetItem( m_root, QStringList( i18nc("botanical", "Bushes" ) ) );
  topItem->setIcon(0, icon);
  m_topFolderMap[ Stauden ] = topItem;

  topItem = new QTreeWidgetItem( m_root, QStringList( i18nc( "Various plants (bot.)", "Others" ) ) );
  topItem->setIcon(0, icon);
  m_topFolderMap[ Etc ] = topItem;

  const QList<CatalogChapter> chapters = catalog->getKatalogChapters();

  // weiterhier: sortiere chapter unter die top folder.
  foreach( CatalogChapter theChapter, chapters ) {
    QTreeWidgetItem *topFolderItem = m_topFolderMap[ Etc ];
    const QString chapter = theChapter.name();

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
      kDebug() << "Undetected catalog " << chapter << endl;
    }

    if( ! chapter.isEmpty() ) {
      QTreeWidgetItem *katItem = new QTreeWidgetItem( topFolderItem, QStringList( chapter ) );
      katItem->setIcon( 0, icon );
      mChapterDict.insert( theChapter.id().toInt(), katItem );
    }
  }
}


