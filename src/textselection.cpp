/***************************************************************************
  textselection  - widget to select header- and footer text data for the doc
                             -------------------
    begin                : 2007-06-01
    copyright            : (C) 2007 by Klaas Freitag
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
#include "textselection.h"
#include "filterheader.h"
#include "defaultprovider.h"
#include "kraftdoc.h"
#include "doctype.h"

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
#include <qpopupmenu.h>

TextSelection::TextSelection( QWidget *parent, KraftDoc::Part part )
  :QVBox( parent )
{
  setMargin( KDialog::marginHint() );
  setSpacing( KDialog::spacingHint() );

  /* a view for the entry text repository */
  ( void ) new QLabel( i18n( "%1 Text Selection" ).arg( KraftDoc::partToString( part ) ), this );

  mTextsView = new KListView( this );
  mTextsView->setItemMargin( 4 );
  mTextsView->setRootIsDecorated( false );
  mTextsView->header()->setHidden( true );
  mTextsView->setResizeMode( QListView::LastColumn );
  mTextsView->setSelectionMode( QListView::Single );

  mTextsView->addColumn( i18n( "Text" ) );

  connect( mTextsView, SIGNAL( selectionChanged( QListViewItem* ) ),
           SIGNAL( textSelectionChanged( QListViewItem* ) ) );
  buildTextList( part );

  // Context Menu
  mMenu = new QPopupMenu( mTextsView );
  // mMenu->insertTitle( i18n("Template Actions") );
  // connect( this, SIGNAL( contextMenuRequested( QListViewItem *, const QPoint& , int ) ),
  //           this, SLOT( slotRMB( QListViewItem *, const QPoint &, int ) ) );
  connect( mTextsView, SIGNAL( contextMenu( KListView*, QListViewItem *, const QPoint& ) ),
           this, SLOT( slotRMB( KListView*, QListViewItem *, const QPoint & ) ) );

  initActions();
}

void TextSelection::buildTextList( KraftDoc::Part part )
{
  QStringList docTypes = DocType::allLocalised();
  mDocTypeItemMap.clear();
  for ( QStringList::Iterator dtIt = docTypes.begin(); dtIt != docTypes.end(); ++dtIt ) {
    KListViewItem *docTypeItem = new KListViewItem( mTextsView, *dtIt );
    docTypeItem->setOpen( true );
    mDocTypeItemMap[*dtIt] = docTypeItem;

    DocTextList dtList = DefaultProvider::self()->documentTexts( *dtIt, part );
    DocTextList::iterator textIt;
    for ( textIt = dtList.begin(); textIt != dtList.end(); ++textIt ) {
      ( *textIt ).setListViewItem( addOneDocText( docTypeItem, *textIt ) );
    }
  }
}

void TextSelection::slotSelectDocType( const QString& doctype )
{
  QStringList docTypes = DocType::allLocalised();
  for ( QStringList::Iterator dtIt = docTypes.begin(); dtIt != docTypes.end(); ++dtIt ) {
    QListViewItem *item = mDocTypeItemMap[ ( *dtIt ) ];

    if ( doctype != *dtIt ) {
      item->setVisible( false );
    } else {
      item->setVisible( true );
    }
  }
}

KListViewItem *TextSelection::addOneDocText( QListViewItem* parent, const DocText& dt )
{
  QString name = dt.name();

  KListViewItem *item1 = new KListViewItem( parent, name );
  if ( name == i18n( "Standard" ) ) {
    item1->setPixmap( 0, SmallIcon( "knewstuff" ) );
  }

  KListViewItem *item2 = new KListViewItem( item1, dt.text() );
  item2->setMultiLinesEnabled( true );

  kdDebug() << "Document database id is "<< dt.dbId().toString() << endl;
  mTextMap[item1] = dt;
  mTextMap[item2] = dt;
  // kdDebug() << "Document database id2 is "<< ( mTextMap[item2] ).dbId().toString() << endl;
  // item1->setOpen( true );
  return item1;
}

QListViewItem* TextSelection::addNewDocText( const DocText& dt )
{
  QListViewItem *item = mDocTypeItemMap[dt.docType()];

  if ( item ) {
    mTextsView->clearSelection();
    QListViewItem *newItem = addOneDocText( item, dt );

    // newItem->setSelected( true );
    return newItem;
  }
  return 0;
}

/* requires the QListViewItem set as a member in the doctext */
void TextSelection::updateDocText( const DocText& dt )
{
  QListViewItem *item = 0;
  // search for the listviewitem that is showing the doctext
  QMap<QListViewItem*, DocText>::iterator it;
  for ( it = mTextMap.begin(); !item && it != mTextMap.end(); ++it ) {
    if ( it.data() == dt && ( it.key() )->firstChild() ) {
      item = it.key();
    }
  }

  if ( item ) {
    kdDebug() << "Update Doc Text Item" << endl;

    mTextMap[item] = dt;

    item->setText( 0, dt.name() );
    QListViewItem *itChild = item->firstChild();
    if ( itChild ) {
      itChild->setText( 0, dt.text() );
    }
  }
}

void TextSelection::deleteCurrentText()
{
  QListViewItem *curr = mTextsView->currentItem();
  if ( mDocTypeItemMap.values().find( curr ) == mDocTypeItemMap.values().end() ) {
    kdDebug() << "Can not delete the doc type item" << endl;
    return;
  }

  if ( ! curr ) return;

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


TextSelection::~TextSelection()
{
}

void TextSelection::initActions()
{
  mActions     = new KActionCollection( this );
  mAcMoveToDoc = new KAction( i18n("&Use in Document"), "back", 0, this,
                              SIGNAL( actionCurrentTextToDoc() ), mActions, "moveToDoc");
  mAcMoveToDoc->plug( mMenu );
}

DocText TextSelection::currentDocText() const
{
  DocText dt;

  QListViewItem *curr = mTextsView->currentItem();
  if ( curr ) {
    dt = mTextMap[curr];
  }

  return dt;
}

QString TextSelection::currentText() const
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


void TextSelection::slotRMB( KListView*, QListViewItem* item, const QPoint& point )
{
  if( ! item ) return;

  // fill the document list with a list of the open docs
  mMenu->popup( point );
}

#include "textselection.moc"
