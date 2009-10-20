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
#include "textselection.h"

#include <klocale.h>
#include <kdebug.h>

#include <kdialog.h>
#include <kaction.h>
#include <kiconloader.h>

#include <QTreeWidget>
#include <QIcon>
#include <qsizepolicy.h>
#include <qlabel.h>
#include <q3vbox.h>
#include <q3header.h>


HeaderSelection::HeaderSelection( QWidget *parent )
  :QTabWidget( parent )
{
  Q3VBox *vBox = new Q3VBox( );
  vBox->setMargin( KDialog::marginHint() );
  vBox->setSpacing( KDialog::spacingHint() );

  addTab( vBox, i18n( "Address Selection" ) );
  mAddressTabId = indexOf( vBox );

  FilterHeader *fh = new FilterHeader( 0, vBox );
  mAddressSelection = new AddressSelection( vBox );
  fh->setListView( mAddressSelection );
  fh->showCount( false );
  mAddressSelection->setupAddressList( );

  connect( mAddressSelection, SIGNAL( selectionChanged( Q3ListViewItem* ) ),
           SIGNAL( addressSelectionChanged() ) );

  connect( mAddressSelection, SIGNAL( doubleClicked( Q3ListViewItem* ) ),
           SIGNAL( doubleClickedOnItem() ) );

  /* a view for the entry text repository */
  vBox = new Q3VBox( );
  vBox->setMargin( KDialog::marginHint() );
  vBox->setSpacing( KDialog::spacingHint() );

  mTextsView = new TextSelection( vBox, KraftDoc::Header );

  // mTextsView->addColumn( i18n( "Text" ) );
  connect( mTextsView,
           SIGNAL( textSelectionChanged( Q3ListViewItem* ) ),
           SIGNAL( textSelectionChanged( Q3ListViewItem* ) ) );

  addTab( vBox, i18n( "Text Templates" ) );
  mTextsTabId = indexOf( vBox );

  connect( this, SIGNAL( currentChanged( QWidget* ) ),
           this, SLOT( slotCurrentTabChanged( QWidget* ) ) );

  setCurrentIndex( mTextsTabId );

}

void HeaderSelection::slotSelectDocType( const QString& doctype )
{
  mTextsView->slotSelectDocType( doctype );
}

bool HeaderSelection::textPageActive()
{
  return ( currentIndex() == mTextsTabId );
}

QTreeWidgetItem *HeaderSelection::itemSelected()
{
  if ( textPageActive() ) {
    return mTextsView->textsListView()->currentItem();
  } else {
    return mAddressSelection->currentItem();
  }
  return 0;
}

bool HeaderSelection::addressPageActive()
{
  return ( currentIndex() == mAddressTabId );
}

HeaderSelection::~HeaderSelection()
{
  delete mAddressSelection;
}

KABC::Addressee HeaderSelection::currentAddressee()
{
  KABC::Addressee adr;
  adr = mAddressSelection->currentAddressee();
  return adr;
}

DocText HeaderSelection::currentDocText() const
{
  return mTextsView->currentDocText();
}

QString HeaderSelection::currentText() const
{
  return currentDocText().text();
}

void HeaderSelection::slotCurrentTabChanged( QWidget *w )
{
  // mPbAdd->setEnabled( false );
  // Hier gehts weiter.

  if ( indexOf( w ) == mTextsTabId ) {
    emit switchedToHeaderTab( TextTab );
  } else if ( indexOf( w ) == mAddressTabId ) {
    emit switchedToHeaderTab( AddressTab );
  } else {
    kError() << "Unknown Widget!" << endl;
  }
}

// #include "headerselection.moc"
