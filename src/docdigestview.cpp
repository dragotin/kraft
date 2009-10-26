/***************************************************************************
                          docdigestview.cpp  -
                             -------------------
    begin                : Wed Mar 15 2006
    copyright            : (C) 2006 by Klaas Freitag
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
#include <QLayout>
#include <QPushButton>
#include <QToolTip>
#include <QHBoxLayout>
#include <QBoxLayout>
#include <QVBoxLayout>

#include <klocale.h>
#include <kdebug.h>
#include <kstandardaction.h>
#include <kaction.h>
#include <ktoolbar.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <kmenu.h>
#include <kcalendarsystem.h>
#include <kabc/addressbook.h>
#include <kabc/stdaddressbook.h>
#include <kabc/addresseedialog.h>
#include <kabc/addressee.h>

#include "filterheader.h"
#include "docdigestview.h"
#include "documentman.h"
#include "docguardedptr.h"
#include "kraftdoc.h"
#include "defaultprovider.h"


DocDigestView::DocDigestView( QWidget *parent )
: QWidget( parent )
{
  QVBoxLayout *box = new QVBoxLayout;
  setLayout( box );

  box->setMargin( 0 );
  box->setSpacing( 0 );

  QHBoxLayout *hbox = new QHBoxLayout;

  mNewDocButton = new QPushButton( i18n( "Create Document" ) );
  connect( mNewDocButton, SIGNAL( clicked() ), this, SIGNAL( createDocument() ) );

  hbox->addWidget( mNewDocButton );
  hbox->addStretch(1);
 
  mListView = new QTreeWidget;
  // mListView->setItemMargin( 5 );
  QPalette palette;
  palette.setColor(QPalette::AlternateBase, QColor("#e0fdd1"));
  
  mListView->setPalette(palette);
  mListView->setAlternatingRowColors( true );
  mContextMenu = new KMenu( mListView );
  // mContextMenu->insertTitle( i18n( "Document Actions" ) );
  connect( mListView, SIGNAL( contextMenuRequested( QTreeWidgetItem *, const QPoint&, int ) ),
           this, SLOT( slotRMB( QTreeWidgetItem *, const QPoint &, int ) ) );

  // mListView->header()->hide();
  mListView->setRootIsDecorated(  true );
  mListView->setSelectionMode( QAbstractItemView::SingleSelection );

  mFilterHeader = new FilterHeader( mListView );
  mFilterHeader->showCount( false );

  connect( mListView, SIGNAL( itemDoubleClicked( QTreeWidgetItem*, int ) ),
           this, SLOT( slotDocOpenRequest( QTreeWidgetItem*, int ) ) );

  connect( mListView, SIGNAL( currentChanged( QTreeWidgetItem* ) ),
           this, SLOT( slotCurrentChanged( QTreeWidgetItem* ) ) );

  hbox->addWidget( mFilterHeader );
  hbox->addSpacing( KDialog::marginHint() );

  box->addLayout( hbox );
  //box->addSpacing( KDialog::marginHint() );

  QHBoxLayout *hbox2 = new QHBoxLayout;
  hbox2->addWidget( mListView );
  hbox2->addSpacing( KDialog::marginHint() );
  box->addLayout( hbox2 );

  mListView->setColumnCount( 7 );
  QStringList cols;
  cols << i18n( "Type" );
  cols << i18n( "Client Name" );
  cols << i18n( "Last Modified" );
  cols << i18n( "Date" );
  cols << i18n( "Whiteboard" );
  cols << i18n( "Project" );
  cols << i18n( "Doc. Number" );

  mListView->setHeaderLabels( cols );
  // mListView->setSorting( 155 ); // sort only manually.
}

DocDigestView::~DocDigestView()
{

}

void DocDigestView::slotBuildView()
{
  DocumentMan *docman = DocumentMan::self();
  mListView->clear();

  QTreeWidgetItem *item = addChapter( i18n( "All Documents" ),
                                                    docman->latestDocs( 0 ) );
  mAllDocsParent = item;
  item->setIcon( 0, SmallIcon( "user-identity" ) ); // KDE 4 icon name: user-identity
  mListView->collapseItem( item );

  item = addChapter( i18n( "Documents by Time" ), DocDigestList() );
  mTimeLineParent = item;
  item->setIcon( 0, SmallIcon( "view-history" ) ); // KDE 4 icon name: view-history
  mListView->collapseItem( item );

  /* create the timeline view */
  DocDigestsTimelineList timeList = docman->docsTimelined();
  DocDigestsTimelineList::iterator it;

  int month = 0;
  int year = 0;
  QTreeWidgetItem *yearItem = 0;

  for ( it = timeList.begin(); it != timeList.end(); ++it ) {
    if ( ( *it ).year() && year != ( *it ).year() ) {
      year = ( *it ).year();

      yearItem = addChapter( QString::number( year ),  DocDigestList(), mTimeLineParent );
      mListView->collapseItem( yearItem );
    }
    month = ( *it ).month();
    const QString monthName =
      DefaultProvider::self()->locale()->calendar()->monthName( month, year ); // , KCalendarSystem::LongName);
    if ( yearItem ) {
      QTreeWidgetItem *mItem = addChapter(  monthName, ( *it ).digests(), yearItem );
      mListView->collapseItem( mItem );
    }
  }

  item = addChapter( i18n( "Latest Documents" ),  docman->latestDocs( 10 ) );
  mLatestDocsParent = item;
  item->setIcon( 0, SmallIcon( "fork" ) );
  mListView->resizeColumnToContents(0);
  
}


QTreeWidgetItem* DocDigestView::addChapter( const QString& chapter, DocDigestList list, QTreeWidgetItem *chapParent )
{
  kDebug() << "Adding docview chapter " << chapter << " with " << list.size() << " elems" << endl;

  QTreeWidgetItem *chapIt;
  if ( chapParent ) {
    chapIt = new QTreeWidgetItem( chapParent, QStringList(chapter));
  } else {
    chapIt = new QTreeWidgetItem( mListView, QStringList(chapter) );
  }
  mListView->expandItem( chapIt );

  DocDigestList::iterator it;
  for ( it = list.begin(); it != list.end(); ++it ) {
    QStringList li;
    li << (*it).type() << (*it).clientName() << ( *it).lastModified() << (*it).date()
        << ( *it ).whiteboard() << ( *it ).projectLabel() << ( *it ).ident();

    QTreeWidgetItem *item = new QTreeWidgetItem( chapIt, li );
    mDocIdDict[item] = (*it).id();

    ArchDocDigestList archDocList = ( *it ).archDocDigestList();
    ArchDocDigestList::iterator archIt;
    for ( archIt = archDocList.begin(); archIt != archDocList.end(); ++archIt ) {
      QStringList li;
      li << i18n("Archived") << QString() << (*archIt).printDateString();
      QTreeWidgetItem *archItem = new QTreeWidgetItem( item, li );
      mArchIdDict[archItem] = (*archIt);
    }
  }
  return chapIt;
}

void DocDigestView::slotRMB( QTreeWidgetItem*, const QPoint& point, int )
{
  mContextMenu->popup( point );
}

/* Called after the document was saved, thus the doc is complete.
 * The new entry should set selected.
 */
void DocDigestView::slotNewDoc( DocGuardedPtr doc )
{
  QTreeWidgetItem *parent = mLatestDocsParent;

  QTreeWidgetItem *currItem = mListView->selectedItems().first();
  QString itemID; // for docSelected signal
  if ( currItem ) mListView->setCurrentItem( currItem );

  if ( !doc ) return;

  // insert item into the "latest docs" list. That makes the latest
  // list one item longer, we're not deleting one entry
  if ( parent ) {
    QTreeWidgetItem *item = new QTreeWidgetItem( parent );
    item->setIcon( 0, SmallIcon( "knewstuff" ) );
    setupListViewItemFromDoc( doc, item );
    mListView->setCurrentItem( item );
    dbID id = doc->docID();
    if ( id.isOk() ) {
      mDocIdDict[item] = id.toString();
      itemID = id.toString();
    }
  }
  emit docSelected( itemID );

  // Insert new item into the "all documents" list
  parent = mAllDocsParent;
  if ( parent ) {
    QTreeWidgetItem *item = new QTreeWidgetItem( parent );
    setupListViewItemFromDoc( doc, item );
    dbID id = doc->docID();
    if ( id.isOk() ) {
      mDocIdDict[item] = id.toString();
    }
  }

  // FIXME: Create a new item in the "Over time"-list.
}

void DocDigestView::slotUpdateDoc( DocGuardedPtr doc )
{
  if ( !doc ) return;
  const QString docId = doc->docID().toString();

  QMapIterator<QTreeWidgetItem*, QString> it( mDocIdDict );
  while( it.hasNext() ) {
    QTreeWidgetItem* item = it.key();
    QString id = it.value();

    if ( docId == id ) {
      setupListViewItemFromDoc( doc, item );
    }
  }
}

void DocDigestView::setupListViewItemFromDoc( DocGuardedPtr doc, QTreeWidgetItem* item )
{
  item->setText( 0,  doc->docType() );

  QString clientName;
  KABC::AddressBook *adrBook =  KABC::StdAddressBook::self();
  KABC::Addressee contact;
  if( adrBook ) {
    contact = adrBook->findByUid( doc->addressUid() );
    clientName = contact.realName();
  }
  item->setText( 1,  clientName );
  item->setText( 2, doc->locale()->formatDate( doc->lastModified(), KLocale::ShortDate ) );
  item->setText( 3, doc->locale()->formatDate( doc->date(), KLocale::ShortDate ) );
  item->setText( 4, doc->whiteboard() );
  item->setText( 5, doc->projectLabel() );
  item->setText( 6, doc->ident() );
}

#if 0
void DocDigestView::slotDocViewRequest( QTreeWidgetItem *item )
{
  QString id = mDocIdDict[ item ];
  if( ! id.isEmpty() ) {
    kDebug() << "Opening document " << id;

    emit viewDocument( id );
  }
}
#endif

void DocDigestView::slotDocOpenRequest( QTreeWidgetItem *item, int )
{
  QString id = mDocIdDict[ item ];
  if( ! id.isEmpty() ) {
    kDebug() << "Opening document " << id << endl;

    emit openDocument( id );
  }

  ArchDocDigest archDoc = mArchIdDict[ item ];
  if ( archDoc.archDocId().isOk() ) {
    emit openArchivedDocument( archDoc );
  }
}

#if 0
void DocDigestView::slotOpenCurrentDoc()
{
  slotDocOpenRequest( mListView->currentItem() );
}
#endif

ArchDocDigest DocDigestView::currentArchiveDoc() const
{
  QTreeWidgetItem *current = mListView->selectedItems().first();
  if( current ) {
    return mArchIdDict[current];
  }
  return ArchDocDigest();
}

QString DocDigestView::currentDocumentId()
{
  QString res;

  QTreeWidgetItem *current = mListView->currentItem();
  if( current ) {
    res = mDocIdDict[current];
  }
  return res;
}

void DocDigestView::slotCurrentChanged( QTreeWidgetItem *item )
{
  dbID id = ( mArchIdDict[item] ).archDocId();
  QString res;
  if ( mDocIdDict.contains( item ) ) {
    emit docSelected( mDocIdDict[item] );
  } else if ( id.isOk() ) {
    emit archivedDocSelected( mArchIdDict[item] );
  }
}

KMenu* DocDigestView::contextMenu()
{
  return mContextMenu;
}

#include "docdigestview.moc"
