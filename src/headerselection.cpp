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
#include <kpushbutton.h>
#include <kaction.h>
#include <kaccel.h>
#include <kiconloader.h>

#include <qiconset.h>
#include <qsizepolicy.h>
#include <qlabel.h>
#include <qvbox.h>



HeaderSelection::HeaderSelection( QWidget *parent )
  :QTabWidget( parent )
{
  QVBox *vBox = new QVBox( );
  vBox->setMargin( KDialog::marginHint() );
  vBox->setSpacing( KDialog::spacingHint() );
#if 0
  QHBox *hb = new QHBox( vBox );
  ( new QWidget( hb ) )->setMinimumWidth( 25 );
  mListSearchLine = new FilterHeader( 0, hb ) ;
  mListSearchLine->showCount( false );

  mAddressView = new KListView( vBox );
  mListSearchLine->setListView( mAddressView );
#endif

  addTab( vBox, i18n( "Address Selection" ) );
  mAddressTabId = indexOf( vBox );

  mAddressSelection = new AddressSelection( vBox );
  mAddressSelection->setupAddressList( );

  connect( mAddressSelection, SIGNAL( selectionChanged() ),
           SIGNAL( addressSelectionChanged() ) );

  vBox = new QVBox( );
  vBox->setMargin( KDialog::marginHint() );
  vBox->setSpacing( KDialog::spacingHint() );
  ( void ) new QLabel( i18n( "Entry Text Selection" ), vBox );
  mTextsView = new KListView( vBox );
  mTextsView->addColumn( i18n( "Description" ) );
  mTextsView->addColumn( i18n( "Text" ) );
  connect( mTextsView, SIGNAL( selectionChanged() ),
           SIGNAL( textSelectionChanged() ) );
  getHeaderTextList();

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


void HeaderSelection::getHeaderTextList()
{
  QStringList docTypes = DefaultProvider::self()->docTypes();
  for ( QStringList::Iterator dtIt = docTypes.begin(); dtIt != docTypes.end(); ++dtIt ) {
    KListViewItem *docTypeItem = new KListViewItem( mTextsView, *dtIt );
    docTypeItem->setOpen( true );

    DocTextList dtList = DefaultProvider::self()->documentTexts( *dtIt, DocText::Header );
    DocTextList::iterator textIt;
    for ( textIt = dtList.begin(); textIt != dtList.end(); ++textIt ) {

      KListViewItem *item1 = new KListViewItem( docTypeItem, ( *textIt ).description(),
                                                ( *textIt ).text() );
      mTextMap[item1] = *textIt;
    }

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

QString HeaderSelection::currentText() const
{
  QString re;

  QListViewItem *curr = mTextsView->currentItem();
  if ( curr ) {
    DocText dt = mTextMap[curr];
    re = dt.text();
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
