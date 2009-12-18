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

#include <kdialog.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kiconloader.h>

#include <QIcon>
#include <QSizePolicy>
#include <QLabel>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QMenu>

TextSelection::TextSelection( QWidget *parent, KraftDoc::Part part )
  :QWidget( parent )
{
  QVBoxLayout *layout = new QVBoxLayout;
  setLayout(layout);

  layout->setMargin( KDialog::marginHint() );
  layout->setSpacing( KDialog::spacingHint() );

  /* a view for the entry text repository */
  QLabel *label = new QLabel( i18n( "%1 Text Selection" ).arg( KraftDoc::partToString( part ) ));
  layout->addWidget(label);

  mTextsView = new QTreeWidget;
  layout->addWidget(mTextsView);
  mTextsView->setRootIsDecorated( false );
  mTextsView->headerItem()->setHidden( true );
  mTextsView->setSelectionMode( QAbstractItemView::SingleSelection );
  mTextsView->setColumnCount( 1 );
  mTextsView->setHeaderLabel( i18n("Text"));

  connect( mTextsView, SIGNAL( currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*) ),
           this, SLOT( slotSelectionChanged( QTreeWidgetItem* ) ) );
  connect( mTextsView, SIGNAL(doubleClicked(QModelIndex) ),
           this, SLOT( slotSelectionChanged( QTreeWidgetItem* ) ) );

  buildTextList( part );

  // Context Menu
  mMenu = new QMenu( mTextsView );
  mMenu->setTitle( i18n("Template Actions") );
  mTextsView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect( mTextsView, SIGNAL(customContextMenuRequested(QPoint) ),
            this, SLOT( slotRMB( QPoint ) ) );

  initActions();
}

void TextSelection::buildTextList( KraftDoc::Part part )
{
  QStringList docTypes = DocType::allLocalised();
  mDocTypeItemMap.clear();
  for ( QStringList::Iterator dtIt = docTypes.begin(); dtIt != docTypes.end(); ++dtIt ) {
    QTreeWidgetItem *docTypeItem = new QTreeWidgetItem( mTextsView );
    docTypeItem->setText(0 , *dtIt );
    docTypeItem->setExpanded( true );
    mDocTypeItemMap[*dtIt] = docTypeItem;

    DocTextList dtList = DefaultProvider::self()->documentTexts( *dtIt, part );
    DocTextList::iterator textIt;
    for ( textIt = dtList.begin(); textIt != dtList.end(); ++textIt ) {
      QTreeWidgetItem *item = addOneDocText( docTypeItem, *textIt );
      QString textname = ( *textIt ).name();
      if ( ( *textIt ).isStandardText() ) {
        mStandardItemMap[*dtIt] = item;
      }
      ( *textIt ).setListViewItem( item );
    }
  }
}

void TextSelection::slotSelectionChanged( QTreeWidgetItem* item )
{
  // do not fire the signal for the root element which is the doc type
  QTreeWidgetItem *it = 0;
  QList<QTreeWidgetItem*> itemsList = mDocTypeItemMap.values();
  if ( itemsList.indexOf( item ) == -1 ) {
    it = item; // was not found in the doctype item list
  }
  emit textSelectionChanged( it );
}

void TextSelection::slotSelectDocType( const QString& doctype )
{
  QStringList docTypes = DocType::allLocalised();
  for ( QStringList::Iterator dtIt = docTypes.begin(); dtIt != docTypes.end(); ++dtIt ) {
    QTreeWidgetItem *item = mDocTypeItemMap[ ( *dtIt ) ];

    if ( doctype != *dtIt ) {
      item->setHidden( true );
    } else {
      item->setHidden( false );
    }
  }
  if ( mStandardItemMap.contains( doctype )  ) {
    mStandardItemMap[doctype]->setSelected( true );
  } else {
    kDebug() << "no standard text found for "<< doctype << endl;
  }
}

QTreeWidgetItem *TextSelection::addOneDocText( QTreeWidgetItem* parent, const DocText& dt )
{
  QString name = dt.name();
  DocText newDt = dt;

  QTreeWidgetItem *item1 = new QTreeWidgetItem( parent );
  item1->setText( 0, name );
  item1->setIcon( 0, dt.pixmap() );
  if ( dt.isStandardText() ) {
    mTextsView->blockSignals( true );
    item1->setSelected( true );
    mTextsView->blockSignals( false );
  }
  newDt.setListViewItem( item1 );

  QTreeWidgetItem *item2 = new QTreeWidgetItem( item1 );
  item2->setText(0, dt.text() );
  // item2->setMultiLinesEnabled( true ); FIXME

  kDebug() << "Document database id is "<< dt.dbId().toString() << endl;
  mTextMap[item1] = newDt;
  mTextMap[item2] = newDt;
  // kDebug() << "Document database id2 is "<< ( mTextMap[item2] ).dbId().toString() << endl;
  // item1->setOpen( true );
  return item1;
}

QTreeWidgetItem* TextSelection::addNewDocText( const DocText& dt )
{
  QTreeWidgetItem *item = mDocTypeItemMap[dt.docType()];

  if ( item ) {
    mTextsView->clearSelection();
    QTreeWidgetItem *newItem = addOneDocText( item, dt );
    return newItem;
  }
  return 0;
}

/* requires the QListViewItem set as a member in the doctext */
void TextSelection::updateDocText( const DocText& dt )
{
  QTreeWidgetItem *item = dt.listViewItem();

  if ( item ) {
    kDebug() << "Update Doc Text Item" << item << endl;

    mTextMap[item] = dt;

    item->setText( 0, dt.name() );
    item->setIcon( 0, dt.pixmap() );

    QTreeWidgetItem *itChild = item->child( 0 );
    if ( itChild ) {
      itChild->setText( 0, dt.text() );
      mTextMap[itChild] = dt;
    }
  }
}

void TextSelection::deleteCurrentText()
{
  QTreeWidgetItem *curr = mTextsView->selectedItems()[0];
  if ( mDocTypeItemMap.values().indexOf( curr ) == mDocTypeItemMap.values().count() ) {
    kDebug() << "Can not delete the doc type item" << endl;
    return;
  }

  if ( ! curr ) return;

  if ( curr->child(0) ) {
    // If the parent item is in the docType map the child must be deleted.
    mTextMap.remove( curr->child(0) );
    delete curr->child(0);
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
  mAcMoveToDoc = mActions->addAction( "moveToDoc", this, SIGNAL(actionCurrentTextToDoc()));
  mAcMoveToDoc->setIcon( KIcon( "go-previous" ));
  mAcMoveToDoc->setText( i18n("&Use in Document") );

  mMenu->addAction( mAcMoveToDoc );

}

DocText TextSelection::currentDocText() const
{
  DocText dt;

  QTreeWidgetItem *curr = mTextsView->selectedItems()[0];
  if ( curr ) {
    dt = mTextMap[curr];
  }

  return dt;
}

QString TextSelection::currentText() const
{
  QString re;

  QTreeWidgetItem *curr = mTextsView->selectedItems()[0];
  if ( curr ) {
    DocText dt = mTextMap[curr];
    re = dt.text();
  } else {
    kDebug() << "No current Item!" << endl;
  }

  return re;
}


void TextSelection::slotRMB(QPoint point )
{
  mMenu->popup( mTextsView->mapToGlobal(point) );
}

#include "textselection.moc"
