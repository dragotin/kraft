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
#include <QVBoxLayout>
#include <q3header.h>


HeaderSelection::HeaderSelection( QWidget *parent )
  : QTabWidget( parent )
{
  QWidget *w = new QWidget( );
  QVBoxLayout *l = new QVBoxLayout();
  w->setLayout(l);
  l->setMargin( KDialog::marginHint() );
  l->setSpacing( KDialog::spacingHint() );

  addTab( w, i18n( "Address Selection" ) );
  mAddressTabId = indexOf( w );

  FilterHeader *fh = new FilterHeader( 0 );
  l->addWidget(fh);
  mAddressSelection = new AddressSelection();
  l->addWidget(mAddressSelection);
  fh->setListView( mAddressSelection );
  fh->showCount( false );
  mAddressSelection->setupAddressList( );

  connect( mAddressSelection, SIGNAL( currentItemChanged( QTreeWidgetItem*,QTreeWidgetItem* ) ),
           SIGNAL( addressSelectionChanged() ) );

  connect( mAddressSelection, SIGNAL( doubleClicked( QModelIndex ) ),
           SIGNAL( doubleClickedOnItem() ) );

  /* a view for the entry text repository */
  w = new QWidget( );
  l = new QVBoxLayout();
  w->setLayout(l);
  l->setMargin( KDialog::marginHint() );
  l->setSpacing( KDialog::spacingHint() );

  mTextsView = new TextSelection( 0, KraftDoc::Header );
  l->addWidget(mTextsView);

  // mTextsView->addColumn( i18n( "Text" ) );
  connect( mTextsView, SIGNAL( textSelectionChanged( QTreeWidgetItem* ) ),
           this, SIGNAL( textSelectionChanged( QTreeWidgetItem* ) ) );

  addTab( w, i18n( "Text Templates" ) );
  mTextsTabId = indexOf( w );

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
