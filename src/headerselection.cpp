/***************************************************************************
        headerselection  - widget to select header data for the doc
                             -------------------
    begin                : 2007-03-24
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
#include "headerselection.h"
#include "addressselection.h"

#include "filterheader.h"
#include "defaultprovider.h"

#include <klocale.h>
#include <kdebug.h>
#include <klistview.h>
#include <kdialog.h>
#include <kaction.h>
#include <kaccel.h>
#include <kiconloader.h>

#include <qiconset.h>
#include <qsizepolicy.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qheader.h>


HeaderSelection::HeaderSelection( QWidget *parent )
  :QTabWidget( parent )
{
  QVBox *vBox = new QVBox( );
  vBox->setMargin( KDialog::marginHint() );
  vBox->setSpacing( KDialog::spacingHint() );

  addTab( vBox, i18n( "Address Selection" ) );
  mAddressTabId = indexOf( vBox );

  mAddressSelection = new AddressSelection( vBox );
  mAddressSelection->setupAddressList( );

  connect( mAddressSelection, SIGNAL( selectionChanged() ),
           SIGNAL( addressSelectionChanged() ) );

  /* a view for the entry text repository */
  vBox = new QVBox( );
  vBox->setMargin( KDialog::marginHint() );
  vBox->setSpacing( KDialog::spacingHint() );
  ( void ) new QLabel( i18n( "Entry Text Selection" ), vBox );
  mTextsView = new KListView( vBox );
  mTextsView->header()->setHidden( true );
  mTextsView->setResizeMode( QListView::LastColumn );
  mTextsView->setSelectionMode( QListView::Single );

  mTextsView->addColumn( i18n( "Text" ) );


  // mTextsView->addColumn( i18n( "Text" ) );
  connect( mTextsView, SIGNAL( selectionChanged() ),
           SIGNAL( textSelectionChanged() ) );
  buildHeaderTextList();

  addTab( vBox, i18n( "Text Templates" ) );
  mTextsTabId = indexOf( vBox );

  connect( this, SIGNAL( currentChanged( QWidget* ) ),
           this, SLOT( slotCurrentTabChanged( QWidget ) ) );

  initActions();

}

void HeaderSelection::slotAddressNew()
{
  kdDebug() << "New address requested!" << endl;
}


void HeaderSelection::buildHeaderTextList()
{
  QStringList docTypes = DefaultProvider::self()->docTypes();
  for ( QStringList::Iterator dtIt = docTypes.begin(); dtIt != docTypes.end(); ++dtIt ) {
    KListViewItem *docTypeItem = new KListViewItem( mTextsView, *dtIt );
    docTypeItem->setOpen( true );
    mDocTypeItemMap[*dtIt] = docTypeItem;

    DocTextList dtList = DefaultProvider::self()->documentTexts( *dtIt, KraftDoc::Header );
    DocTextList::iterator textIt;
    for ( textIt = dtList.begin(); textIt != dtList.end(); ++textIt ) {
      ( *textIt ).setListViewItem( addOneDocText( docTypeItem, *textIt ) );
    }
  }
}

void HeaderSelection::slotSelectDocType( const QString& doctype )
{
  QStringList docTypes = DefaultProvider::self()->docTypes();
  for ( QStringList::Iterator dtIt = docTypes.begin(); dtIt != docTypes.end(); ++dtIt ) {
    QListViewItem *item = mDocTypeItemMap[ ( *dtIt ) ];

    if ( doctype != *dtIt ) {
      item->setVisible( false );
    } else {
      item->setVisible( true );
    }
  }
}

KListViewItem *HeaderSelection::addOneDocText( QListViewItem* parent, const DocText& dt )
{
  QString name = dt.name();

  KListViewItem *item1 = new KListViewItem( parent, name );
  if ( name == i18n( "Standard" ) ) {
    item1->setPixmap( 0, SmallIcon( "knewstuff" ) );
  }

  KListViewItem *item2 = new KListViewItem( item1, dt.text() );

  kdDebug() << "Document database id is "<< dt.dbId().toString() << endl;
  mTextMap[item1] = dt;
  mTextMap[item2] = dt;
  // kdDebug() << "Document database id2 is "<< ( mTextMap[item2] ).dbId().toString() << endl;
  // item1->setOpen( true );
  return item1;
}

QListViewItem* HeaderSelection::addNewDocText( const DocText& dt )
{
  QListViewItem *item = mDocTypeItemMap[dt.docType()];
  if ( item ) {
    mTextsView->clearSelection();
    return addOneDocText( item, dt );
  }
  return 0;
}

/* requires the QListViewItem set in the doctext */
void HeaderSelection::updateDocText( const DocText& dt )
{
  kdDebug() << "Update Doc Text" << endl;
  QListViewItem *it = dt.listViewItem();
  if ( it ) {
    kdDebug() << "Update Doc Text Item" << endl;

    mTextMap[it] = dt;

    it->setText( 0, dt.name() );
    QListViewItem *itChild = it->firstChild();
    if ( itChild ) {
      itChild->setText( 0, dt.text() );
    }
  }
}

void HeaderSelection::deleteCurrentText()
{
  QListViewItem *curr = mTextsView->currentItem();

  if ( curr->firstChild() ) {
    // If the parent item is in the docType map the child must be deleted.
    mTextMap.remove( curr->firstChild() );
    delete curr->firstChild();
    mTextMap.remove( curr );
    delete curr;
  } else {
    // If the parent is in not in a docType Item, it must be deleted.
    mTextMap.remove( curr->parent() );
    delete curr->parent();
    mTextMap.remove( curr );
    // the current item gets already deleted from its parent.
    // delete curr;
  }
}

bool HeaderSelection::textPageActive()
{
  return ( currentPageIndex() == mTextsTabId );
}

bool HeaderSelection::addressPageActive()
{
  return ( currentPageIndex() == mAddressTabId );
}

HeaderSelection::~HeaderSelection()
{
  delete mAddressSelection;
}

void HeaderSelection::initActions()
{

}

KABC::Addressee HeaderSelection::currentAddressee()
{
  KABC::Addressee adr;
  adr = mAddressSelection->currentAddressee();
  return adr;
}

DocText HeaderSelection::currentDocText() const
{
  DocText dt;

  QListViewItem *curr = mTextsView->currentItem();
  if ( curr ) {
    dt = mTextMap[curr];
  }
  dt.setListViewItem( curr );
  return dt;
}

QString HeaderSelection::currentText() const
{
  QString re;

  QListViewItem *curr = mTextsView->currentItem();
  if ( curr ) {
    DocText dt = mTextMap[curr];
    re = dt.text();
  } else {
    kdDebug() << "No current Item!" << endl;
  }

  return re;
}

void HeaderSelection::slotCurrentTabChanged( QWidget *w )
{
  // mPbAdd->setEnabled( false );
  // Hier gehts weiter.

  if ( indexOf( w ) == mTextsTabId ) {

  } else if ( indexOf( w ) == mAddressTabId ) {

  } else {
    kdError() << "Unknown Widget!" << endl;
  }
}

#include "headerselection.moc"
