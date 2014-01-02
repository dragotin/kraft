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
#include "kraftsettings.h"
#include "quicksearchwidget.h"

#include <QtGui>
#include <kabc/addressee.h>
#include <kabc/contactgroup.h>
#include <kselectionproxymodel.h>
#include <klocale.h>
#include <kcheckableproxymodel.h>

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


namespace {
static bool isStructuralCollection( const Akonadi::Collection &collection )
{
  QStringList mimeTypes;
  mimeTypes << KABC::Addressee::mimeType() << KABC::ContactGroup::mimeType();
  const QStringList collectionMimeTypes = collection.contentMimeTypes();
  foreach ( const QString &mimeType, mimeTypes ) {
    if ( collectionMimeTypes.contains( mimeType ) ) {
      return false;
    }
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
      // kDebug() << "Collection is Valid: " << collection.isValid();
      // kDebug() << "Collection is Structural: " << isStructuralCollection( collection );
      if ( collection.isValid() && isStructuralCollection( collection ) ) {
        return QVariant();
      }
    }
    return KCheckableProxyModel::data( index, role );
  }
};

}


AkonadiAddressSelector::AkonadiAddressSelector(QWidget *parent, bool /* showText */) :
  QWidget(parent),
  mContactsEditor(0)
{
  /* This code was borrowed from the KAddressbook Code from Tobias König.
   * It makes good use of models and proxymodels to get the lists done.
   * Complex stuff - be prepared ;-)
   */
  setupGui();

  // The collection tree lists the various address books, they're collections
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

  connect( mQuickSearchWidget, SIGNAL( filterStringChanged( const QString& ) ),
           mContactsFilterModel, SLOT( setFilterString( const QString& ) ) );
  connect( mQuickSearchWidget, SIGNAL( arrowDownKeyPressed() ),
           mItemView, SLOT( setFocus() ) );

  mItemView->setModel( mContactsFilterModel );
  mItemView->setSelectionMode( QAbstractItemView::SingleSelection );
  mItemView->setRootIsDecorated( false );
  mItemView->header()->setDefaultAlignment( Qt::AlignCenter );

  connect( mItemView, SIGNAL( currentChanged( const Akonadi::Item& ) ),
           this, SLOT( slotItemSelected( const Akonadi::Item& ) ) );

  QMetaObject::invokeMethod( this, "delayedInit", Qt::QueuedConnection );
}

void AkonadiAddressSelector::delayedInit()
{
  // restore previous state
  connect( GlobalContactModel::instance()->model(), SIGNAL(modelAboutToBeReset()), SLOT(saveState()) );
  connect( GlobalContactModel::instance()->model(), SIGNAL(modelReset()), SLOT(restoreState()) );

  restoreState();
}

AkonadiAddressSelector::~AkonadiAddressSelector()
{
  delete mContactsEditor;
}

void AkonadiAddressSelector::setupGui()
{
  QVBoxLayout *vbox = new QVBoxLayout;
  setLayout( vbox );

  mQuickSearchWidget = new QuickSearchWidget;
  mQuickSearchWidget->setSizePolicy( QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed ));
  QLabel *lab = new QLabel;
  lab->setSizePolicy( QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed ));
  lab->setText( i18n("&Search: ") );
  lab->setBuddy( mQuickSearchWidget );
  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->addWidget( lab );
  hbox->addWidget( mQuickSearchWidget );
  hbox->addStretch(3);
  vbox->addLayout( hbox );

  mSplitter = new QSplitter;
  vbox->addWidget( mSplitter );
  mCollectionView = new Akonadi::EntityTreeView();
  mCollectionView->setObjectName( "CollectionView" );
  mSplitter->addWidget( mCollectionView );

  // the items view, items are contacts in this case
  mItemView = new Akonadi::EntityTreeView();
  mItemView->setObjectName( "ContactView" );
  mSplitter->addWidget( mItemView );

  // the viewer for the contact.
  Akonadi::ContactViewer *viewer = new Akonadi::ContactViewer( this );
  mSplitter->addWidget( viewer );
  connect( mItemView, SIGNAL( currentChanged( const Akonadi::Item& ) ),
           viewer, SLOT( setContact( const Akonadi::Item& ) ) );

  // the open address book button is below.
  QHBoxLayout *hboxBot = new QHBoxLayout;
  mBookButton = new QPushButton;
  mBookButton->setCheckable( true );

  connect(mBookButton,SIGNAL(clicked()),this,SLOT(slotToggleBookSelection()));

  mBookButton->setIcon( KIcon( "address-book-new" ));
  mBookButton->setSizePolicy( QSizePolicy( QSizePolicy::Maximum, QSizePolicy::Maximum ));
  mBookButton->setToolTip( i18n("Display address book selection list and hide address details.") );

  hboxBot->addWidget( mBookButton );
  hboxBot->addStretch(4);
  vbox->addLayout( hboxBot );
  mButEditContact = new QPushButton(i18n("Edit Contact..."));
  mButEditContact->setToolTip( i18n("Edit the currently selected contact" ));
  mButEditContact->setEnabled( false );
  hboxBot->addWidget( mButEditContact );
  QPushButton *butCreateContact = new QPushButton(i18n("New Contact..."));
  butCreateContact->setToolTip( i18n("Create a new Contact" ) );
  hboxBot->addWidget( butCreateContact );

  connect(butCreateContact,SIGNAL(clicked()),SLOT( slotCreateNewContact()));
  connect(mButEditContact,SIGNAL(clicked()),SLOT(slotEditContact()));
}

void AkonadiAddressSelector::restoreState()
{
  // collection view
  {
    Akonadi::ETMViewStateSaver *saver = new Akonadi::ETMViewStateSaver;
    saver->setView( mCollectionView );

    const KConfigGroup group( KraftSettings::self()->config(), "CollectionViewState" );
    saver->restoreState( group );
  }

  // collection view
  {
    Akonadi::ETMViewStateSaver *saver = new Akonadi::ETMViewStateSaver;
    saver->setSelectionModel( mCollectionSelectionModel );

    const KConfigGroup group( KraftSettings::self()->config(), "CollectionViewCheckState" );
    saver->restoreState( group );
  }

  // restore the central slider
  QList<int> sizes = KraftSettings::self()->addressPickerSplitterSize();
  if( sizes.isEmpty() ) {
    sizes << 200 << 200 << 0;
  } else {
    mBookButton->setDown( (sizes[0] > 0) );
  }
  mSplitter->setSizes( sizes );

  // item view
}

void AkonadiAddressSelector::saveState()
{
  // collection view
  {
    Akonadi::ETMViewStateSaver saver;
    saver.setView( mCollectionView );

    KConfigGroup group( KraftSettings::self()->config(), "CollectionViewState" );
    saver.saveState( group );
    group.sync();
  }

  // collection view
  {
    Akonadi::ETMViewStateSaver saver;
    saver.setSelectionModel( mCollectionSelectionModel );

    KConfigGroup group( KraftSettings::self()->config(), "CollectionViewCheckState" );
    saver.saveState( group );
    group.sync();
  }

  KraftSettings::self()->setAddressPickerSplitterSize( mSplitter->sizes() );


  // item view
  {
    Akonadi::ETMViewStateSaver saver;
    saver.setView( mItemView );
    saver.setSelectionModel( mItemView->selectionModel() );

    KConfigGroup group( KraftSettings::self()->config(), "ItemViewState" );
    saver.saveState( group );
    group.sync();
  }
}


void AkonadiAddressSelector::slotCreateNewContact()
{
  if( mContactsEditor ) delete( mContactsEditor );

  mContactsEditor = new Akonadi::ContactEditorDialog( Akonadi::ContactEditorDialog::CreateMode, this );
  mContactsEditor->show();
}

void AkonadiAddressSelector::slotEditContact()
{
  if( mItemView->selectionModel()->hasSelection() ) {
    QModelIndex index = mItemView->selectionModel()->currentIndex();
    if ( index.isValid() ) {
      const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
      if ( item.isValid() && item.hasPayload<KABC::Addressee>() ) {
        if( mContactsEditor ) delete( mContactsEditor );
        mContactsEditor = new Akonadi::ContactEditorDialog( Akonadi::ContactEditorDialog::EditMode, this );
        mContactsEditor->setContact( item );
        mContactsEditor->show();
      }
    }
  }
}

void AkonadiAddressSelector::slotItemSelected( const Akonadi::Item& item )
{
  if ( item.hasPayload<KABC::Addressee>() ) {
    const KABC::Addressee contact = item.payload<KABC::Addressee>();
    emit addressSelected( contact );
    mButEditContact->setEnabled( true );
  } else {
    kDebug() << "No address was selected!";
    mButEditContact->setEnabled( false );
  }
}

void AkonadiAddressSelector::slotToggleBookSelection()
{
  QList<int> sizes = mSplitter->sizes();
  QList<int> newSizes;
  if( sizes[0] == 0 ) {
    // The address selection is currently not visible. We show it.
    newSizes << 200 << 200 << 0;
  } else {
    // The address selection is currently visible
    newSizes << 0 << 200 << 200;
  }

  mSplitter->setSizes( newSizes );
}
