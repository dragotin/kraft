/***************************************************************************
    akonadiaddressselector - Address Selection Widget based on Akonadi
                             -------------------
    begin                : Jul 2011
    copyright            : (C) 2011 by Klaas Freitag
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

#include "akonadiaddressselector.h"

#include <QtGui>
#include <kabc/addressee.h>
#include <kabc/contactgroup.h>
#include <kcheckableproxymodel.h>
#include <kselectionproxymodel.h>
#include <klocale.h>


#include <models/globalcontactmodel.h>

#include <akonadi/etmviewstatesaver.h>
#include <akonadi/collectionfilterproxymodel.h>
#include <akonadi/collectionmodel.h>
#include <akonadi/contact/contactdefaultactions.h>
#include <akonadi/contact/contacteditordialog.h>
#include <akonadi/contact/contactgroupeditordialog.h>
#include <akonadi/contact/contactgroupviewer.h>
#include <akonadi/contact/contactsfilterproxymodel.h>
#include <akonadi/contact/contactstreemodel.h>
#include <akonadi/contact/contactviewer.h>
#include <akonadi/contact/standardcontactactionmanager.h>
#include <akonadi/control.h>
#include <akonadi/entitymimetypefiltermodel.h>
#include <akonadi/entitytreeview.h>
#include <akonadi/entitytreeviewstatesaver.h>
#include <akonadi/itemview.h>
#include <akonadi/mimetypechecker.h>
#include <akonadi/contact/contacteditor.h>
#include <akonadi/contact/contacteditordialog.h>

namespace {
static bool isStructuralCollection( const Akonadi::Collection &collection )
{
  QStringList mimeTypes;
  mimeTypes << KABC::Addressee::mimeType() << KABC::ContactGroup::mimeType();
  const QStringList collectionMimeTypes = collection.contentMimeTypes();
  foreach ( const QString &mimeType, mimeTypes ) {
    if ( collectionMimeTypes.contains( mimeType ) )
      return false;
  }
  return true;
}

class StructuralCollectionsNotCheckableProxy : public KCheckableProxyModel {
public:
  StructuralCollectionsNotCheckableProxy(QObject* parent)
      : KCheckableProxyModel(parent)
  { }

  /* reimp */ QVariant data( const QModelIndex &index, int role ) const
  {
    if ( !index.isValid() )
      return QVariant();

    if ( role == Qt::CheckStateRole ) {
      // Don't show the checkbox if the collection can't contain incidences
      const Akonadi::Collection collection = index.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
      if ( collection.isValid() && isStructuralCollection( collection ) ) {
        return QVariant();
      }
    }
    return KCheckableProxyModel::data( index, role );
  }
};

 }


AkonadiAddressSelector::AkonadiAddressSelector(QWidget *parent, bool showText) :
    QWidget(parent)
{
  setupGui();

  mCollectionTree = new Akonadi::EntityMimeTypeFilterModel( this );
  mCollectionTree->setDynamicSortFilter( true );
  mCollectionTree->setSortCaseSensitivity( Qt::CaseInsensitive );
  mCollectionTree->setSourceModel( GlobalContactModel::instance()->model() );
  mCollectionTree->addMimeTypeInclusionFilter( Akonadi::Collection::mimeType() );
  mCollectionTree->setHeaderGroup( Akonadi::EntityTreeModel::CollectionTreeHeaders );

  mCollectionSelectionModel = new QItemSelectionModel( mCollectionTree );
  StructuralCollectionsNotCheckableProxy *checkableProxyModel = new StructuralCollectionsNotCheckableProxy( this );
  checkableProxyModel->setSelectionModel( mCollectionSelectionModel );
  checkableProxyModel->setSourceModel( mCollectionTree );

  mCollectionView->setModel( checkableProxyModel );
  mCollectionView->header()->setDefaultAlignment( Qt::AlignCenter );
  mCollectionView->header()->setSortIndicatorShown( false );

  KSelectionProxyModel *selectionProxyModel = new KSelectionProxyModel( mCollectionSelectionModel,
                                                                        this );
  selectionProxyModel->setSourceModel( GlobalContactModel::instance()->model() );
  selectionProxyModel->setFilterBehavior( KSelectionProxyModel::ChildrenOfExactSelection );

  mItemTree = new Akonadi::EntityMimeTypeFilterModel( this );
  mItemTree->setSourceModel( selectionProxyModel );
  mItemTree->addMimeTypeExclusionFilter( Akonadi::Collection::mimeType() );
  mItemTree->setHeaderGroup( Akonadi::EntityTreeModel::ItemListHeaders );

  mContactsFilterModel = new Akonadi::ContactsFilterProxyModel( this );
  mContactsFilterModel->setSourceModel( mItemTree );
  // connect( mQuickSearchWidget, SIGNAL( filterStringChanged( const QString& ) ),
  //          mContactsFilterModel, SLOT( setFilterString( const QString& ) ) );
  // connect( mQuickSearchWidget, SIGNAL( filterStringChanged( const QString& ) ),
  //          this, SLOT( selectFirstItem() ) );
  // connect( mQuickSearchWidget, SIGNAL( arrowDownKeyPressed() ),
  //         mItemView, SLOT( setFocus() ) );

  mItemView->setModel( mContactsFilterModel );
  mItemView->setSelectionMode( QAbstractItemView::ExtendedSelection );
  mItemView->setRootIsDecorated( false );
  mItemView->header()->setDefaultAlignment( Qt::AlignCenter );

  connect( mItemView, SIGNAL( currentChanged( const Akonadi::Item& ) ),
           this, SLOT( itemSelected( const Akonadi::Item& ) ) );
 // connect( mItemView, SIGNAL( doubleClicked( const Akonadi::Item& ) ),
 //          mActionManager->action( Akonadi::StandardContactActionManager::EditItem ), SLOT( trigger() ) );
  connect( mItemView->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ),
           this, SLOT( itemSelectionChanged( const QModelIndex&, const QModelIndex& ) ) );

}

AkonadiAddressSelector::~AkonadiAddressSelector()
{

}

void AkonadiAddressSelector::setupGui()
{
  QVBoxLayout *vbox = new QVBoxLayout;
  setLayout( vbox );

  // The splitter that contains the three main parts of the gui
  //   - collection view on the left
  //   - item view in the middle
  //   - details pane on the right, that contains
  //       - details view stack on the top
  //       - contact switcher at the bottom


  // the collection view
  QHBoxLayout *hbox = new QHBoxLayout;
  vbox->addLayout( hbox );
  mCollectionView = new Akonadi::EntityTreeView();
  mCollectionView->setObjectName( "CollectionView" );
  hbox->addWidget( mCollectionView );

  // the items view
  mItemView = new Akonadi::EntityTreeView();
  mItemView->setObjectName( "ContactView" );
  vbox->addWidget( mItemView );

  // the open address book button
  QHBoxLayout *hbox1 = new QHBoxLayout;
  vbox->addLayout( hbox1 );

  QPushButton *openAdrBook = new QPushButton(i18n("New Contact..."));
  hbox1->addWidget( openAdrBook );
  connect( openAdrBook, SIGNAL(clicked() ), SLOT( slotOpenAddressBook() ) );
  hbox1->addStretch(4);

}


void AkonadiAddressSelector::slotOpenAddressBook()
{
  // ContactEditorDialog *dlg = new ContactEditorDialog( Akonadi::ContactEditorDialog::CreateMode, this );
  // connect( dlg, SIGNAL( contactStored( const Akonadi::Item& ) ),
  //          this, SLOT( slotUpdateAddressList( const Akonadi::Item& ) ) );
  // dlg->show();
}

