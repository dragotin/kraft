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

  QVBox *vBox = new QVBox(  );
  vBox->setMargin( KDialog::marginHint() );
  vBox->setSpacing( KDialog::spacingHint() );
  QHBox *hb = new QHBox( vBox );
  ( new QWidget( hb ) )->setMinimumWidth( 25 );
  mListSearchLine = new FilterHeader( 0, hb ) ;
  mListSearchLine->showCount( false );


  mAddressView = new KListView( vBox );
  mListSearchLine->setListView( mAddressView );
  mAddressView->setRootIsDecorated( true );
  mAddressView->addColumn( i18n( "Real Name" ) );
  mAddressView->addColumn( i18n( "Locality" ) );

  addTab( vBox, i18n( "Address Selection" ) );

  mAddressSelection = new AddressSelection();
  mAddressSelection->setupAddressList( mAddressView );

  vBox = new QVBox( );
  vBox->setMargin( KDialog::marginHint() );
  vBox->setSpacing( KDialog::spacingHint() );
  ( void ) new QLabel( i18n( "Entry Text Selection" ), vBox );
  mTextsView = new KListView( vBox );
  mTextsView->addColumn( i18n( "Description" ) );
  mTextsView->addColumn( i18n( "Text" ) );

  getHeaderTextList();

  addTab( vBox, i18n( "Text Templates" ) );

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
      /* KListViewItem *item1 = */ ( void ) new KListViewItem( docTypeItem, ( *textIt ).description(),
                                                ( *textIt ).text() );
    }

  }
}

HeaderSelection::~HeaderSelection()
{
  delete mAddressSelection;
}

void HeaderSelection::initActions()
{

}

#include "headerselection.moc"
