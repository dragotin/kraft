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
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qheader.h>
#include <qpopupmenu.h>
#include <qtooltip.h>

#include <klistview.h>
#include <klocale.h>
#include <kdebug.h>
#include <kstdaction.h>
#include <kaction.h>
#include <ktoolbar.h>
#include <kdialog.h>
#include <kiconloader.h>
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
#include <qlistview.h>
#include "defaultprovider.h"


DocDigestView::DocDigestView( QWidget *parent )
: QWidget( parent )
{
  QWidget *w = new QWidget(parent);

  QBoxLayout *box = new QVBoxLayout( w );
  box->setMargin( 0 );
  box->setSpacing( 0 );
  QBoxLayout *hbox = new QHBoxLayout( w );

  mNewDocButton = new QPushButton( i18n( "Create Document" ), w );
  connect( mNewDocButton, SIGNAL( clicked() ), this, SIGNAL( createDocument() ) );

  hbox->addWidget( mNewDocButton );

  hbox->addStretch(1);
  mListView = new KListView( w );
  mListView->setItemMargin( 5 );
  mListView->setAlternateBackground( QColor( "#dffdd0" ) );
  mContextMenu = new QPopupMenu( mListView );
  connect( mListView, SIGNAL( contextMenuRequested( QListViewItem *, const QPoint&, int ) ),
           this, SLOT( slotRMB( QListViewItem *, const QPoint &, int ) ) );

  // mListView->header()->hide();
  mListView->setRootIsDecorated(  true );
  mListView->setSelectionMode( QListView::Single );

  mFilterHeader = new FilterHeader( mListView, w );
  mFilterHeader->showCount( false );

  connect( mListView, SIGNAL( executed( QListViewItem* ) ),
           this, SLOT( slotDocOpenRequest( QListViewItem* ) ) );

  connect( mListView, SIGNAL( currentChanged( QListViewItem* ) ),
           this, SLOT( slotCurrentChanged( QListViewItem* ) ) );

  hbox->addWidget( mFilterHeader );
  hbox->addSpacing( KDialog::marginHint() );

  box->addLayout( hbox );
  box->addSpacing( KDialog::marginHint() );

  QBoxLayout *hbox2 = new QHBoxLayout( w );
  hbox2->addWidget( mListView );
  hbox2->addSpacing( KDialog::marginHint() );
  box->addLayout( hbox2 );

  mListView->addColumn( i18n( "Type" ) );
  mListView->addColumn( i18n( "Client Name" ) );
  mListView->addColumn( i18n( "Last Modified" ) );
  mListView->addColumn( i18n( "Date" ) );
  mListView->addColumn( i18n( "Whiteboard" ) );
  mListView->addColumn( i18n( "Doc. Number" ) );

  mListView->setSorting( 155 ); // sort only manually.
}

DocDigestView::~DocDigestView()
{

}

void DocDigestView::slotBuildView()
{
  DocumentMan *docman = DocumentMan::self();
  mListView->clear();

  KListViewItem *item = addChapter( i18n( "All Documents" ),
                                                    docman->latestDocs( 0 ) );
  mAllDocsParent = item;
  item->setPixmap( 0, SmallIcon( "identity" ) ); // KDE 4 icon name: user-identity
  item->setOpen( false );

  item = addChapter( i18n( "Documents by Time" ),
                                     DocDigestList() );
  mTimeLineParent = item;
  item->setPixmap( 0, SmallIcon( "history" ) ); // KDE 4 icon name: view-history
  item->setOpen( false );

  /* create the timeline view */
  DocDigestsTimelineList timeList = docman->docsTimelined();
  DocDigestsTimelineList::iterator it;

  int month = 0;
  int year = 0;
  KListViewItem *yearItem = 0;

  for ( it = timeList.begin(); it != timeList.end(); ++it ) {
    if ( ( *it ).year() && year != ( *it ).year() ) {
      year = ( *it ).year();

      yearItem = addChapter( QString::number( year ),  DocDigestList(), mTimeLineParent );
      yearItem->setOpen( false );
    }
    month = ( *it ).month();
    const QString monthName =
      DefaultProvider::self()->locale()->calendar()->monthName( month, year ); // , KCalendarSystem::LongName);
    KListViewItem *mItem = addChapter(  monthName, ( *it ).digests(), yearItem );
    mItem->setOpen( false );
  }

  item = addChapter( i18n( "Latest Documents" ),  docman->latestDocs( 10 ) );
  mLatestDocsParent = item;
  item->setPixmap( 0, SmallIcon( "fork" ) );
}


KListViewItem* DocDigestView::addChapter( const QString& chapter, DocDigestList list, KListViewItem *chapParent )
{
  kdDebug() << "Adding docview chapter " << chapter << " with " << list.size() << " elems" << endl;

  KListViewItem *chapIt;
  if ( chapParent ) {
    chapIt = new KListViewItem( chapParent, chapter );
  } else {
    chapIt = new KListViewItem( mListView, chapter );
  }
  chapIt->setOpen( true );

  DocDigestList::iterator it;
  for ( it = list.begin(); it != list.end(); ++it ) {
    KListViewItem *item = new KListViewItem( chapIt,
                                             (*it).type(), (*it).clientName(),
                                             ( *it).lastModified(), (*it).date(),
                                             ( *it ).whiteboard(), ( *it ).ident() );

    mDocIdDict[item] = (*it).id();

    ArchDocDigestList archDocList = ( *it ).archDocDigestList();
    ArchDocDigestList::iterator archIt;
    for ( archIt = archDocList.begin(); archIt != archDocList.end(); ++archIt ) {
      KListViewItem *archItem = new KListViewItem( item, i18n( "Archived" ), QString(),
                                                   ( *archIt ).printDateString() );
      mArchIdDict[archItem] = (*archIt);
    }
  }
  return chapIt;
}

void DocDigestView::slotRMB( QListViewItem*, const QPoint& point, int )
{
  mContextMenu->popup( point );
}

/* Called after the document was saved, thus the doc is complete.
 * The new entry should set selected.
 */
void DocDigestView::slotNewDoc( DocGuardedPtr doc )
{
  KListViewItem *parent = mLatestDocsParent;

  QListViewItem *currItem = mListView->selectedItem();
  QString itemID; // for docSelected signal
  if ( currItem ) mListView->setSelected( currItem, false );

  if ( !doc ) return;

  // insert item into the "latest docs" list. That makes the latest
  // list one item longer, we're not deleting one entry
  if ( parent ) {
    KListViewItem *item = new KListViewItem( parent );
    item->setPixmap( 0, SmallIcon( "knewstuff" ) );
    setupListViewItemFromDoc( doc, item );
    mListView->setSelected( item, true );
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
    KListViewItem *item = new KListViewItem( parent );
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

  QMap<QListViewItem*, QString>::Iterator it;
  for ( it = mDocIdDict.begin(); it != mDocIdDict.end(); ++it ) {
    QListViewItem* item = it.key();
    QString id = it.data();

    if ( docId == id ) {
      setupListViewItemFromDoc( doc, item );
    }
  }
}

void DocDigestView::setupListViewItemFromDoc( DocGuardedPtr doc, QListViewItem* item )
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
  item->setText( 2, doc->locale()->formatDate( doc->lastModified(), true ) );
  item->setText( 3, doc->locale()->formatDate( doc->date(), true ) );
  item->setText( 4, doc->whiteboard() );
  item->setText( 5, doc->ident() );
}

void DocDigestView::slotDocOpenRequest( QListViewItem *item )
{
  QString id = mDocIdDict[ item ];
  if( ! id.isEmpty() ) {
    kdDebug() << "Opening document " << id << endl;

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
  QListViewItem *current = mListView->currentItem();
  if( current ) {
    return mArchIdDict[current];
  }
  return ArchDocDigest();
}

QString DocDigestView::currentDocumentId()
{
  QString res;

  QListViewItem *current = mListView->currentItem();
  if( current ) {
    res = mDocIdDict[current];
  }
  return res;
}

void DocDigestView::slotCurrentChanged( QListViewItem *item )
{
  dbID id = ( mArchIdDict[item] ).archDocId();
  QString res;
  if ( mDocIdDict[item] ) {
    emit docSelected( mDocIdDict[item] );
  } else if ( id.isOk() ) {
    emit archivedDocSelected( mArchIdDict[item] );
  }
}

#include "docdigestview.moc"
