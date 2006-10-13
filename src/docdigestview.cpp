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

#include <klistview.h>
#include <klocale.h>
#include <kdebug.h>
#include <kstdaction.h>
#include <kaction.h>
#include <ktoolbar.h>

#include "filterheader.h"
#include "docdigestview.h"

DocDigestView::DocDigestView( QWidget *parent )
: QWidget( parent )
{
  QWidget *w = new QWidget(parent);

  QBoxLayout *box = new QVBoxLayout( w );
  QBoxLayout *hbox = new QHBoxLayout( w );
  hbox->addStretch(1);
  box->addLayout( hbox );
  mListView = new KListView( w );
  // mListView->header()->hide();
  mListView->setRootIsDecorated(  true );

  mFilterHeader = new FilterHeader( mListView, w );
  mFilterHeader->showCount( false );

  connect( mListView, SIGNAL( executed( QListViewItem* ) ),
           this, SLOT( slotDocOpenRequest( QListViewItem* ) ) );

  connect( mListView, SIGNAL( currentChanged( QListViewItem* ) ),
           this, SLOT( slotCurrentChanged( QListViewItem* ) ) );

  hbox->addWidget( mFilterHeader );
  box->addWidget( mListView );

  mListView->addColumn( i18n( "Type" ) );
  mListView->addColumn( i18n( "Client Name" ) );
  mListView->addColumn( i18n( "Last Modified" ) );
  mListView->addColumn( i18n( "Date" ) );
  mListView->setSorting( 155 ); // sort only manually.
}

DocDigestView::~DocDigestView()
{

}

KListViewItem* DocDigestView::addChapter( const QString& chapter, DocDigestList list, KListViewItem *chapParent )
{
  kdDebug() << "Adding docview chapter " << chapter << " with " << list.size() << " elems" << endl;

  KListViewItem *chapIt;
  if ( chapParent ) {
    chapIt = new KListViewItem( chapParent,  chapter );
  } else {
    chapIt = new KListViewItem( mListView, chapter );
  }
  chapIt->setOpen( true );

  DocDigestList::iterator it;
  for ( it = list.begin(); it != list.end(); ++it ) {
    KListViewItem *item = new KListViewItem( chapIt,
                             (*it).type(), (*it).clientName(),
                                             ( *it).lastModified(), (*it).date()  );
    mDocIdDict[item] = (*it).id();

    ArchDocDigestList archDocList = ( *it ).archDocDigestList();
    ArchDocDigestList::iterator archIt;
    for ( archIt = archDocList.begin(); archIt != archDocList.end(); ++archIt ) {
      ( void ) new KListViewItem( item, i18n( "Archived" ), QString(),
                                  ( *archIt ).printDateString() );

    }
  }
  return chapIt;
}

void DocDigestView::slotNewDoc()
{

}

void DocDigestView::slotDocOpenRequest( QListViewItem *item )
{
  QString id = mDocIdDict[ item ];
  if( ! id.isEmpty() ) {
    kdDebug() << "Opening document " << id << endl;

    emit openDocument( id );
  }
}

void DocDigestView::slotOpenCurrentDoc()
{
  slotDocOpenRequest( mListView->currentItem() );
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
  QString res;
  if( item ) {
    res = mDocIdDict[item];
  }
  kdDebug() << "Current doc selection changed to " << res << endl;
  emit selectionChanged( res );
}

#include "docdigestview.moc"
