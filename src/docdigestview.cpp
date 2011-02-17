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
#include <QtGui>
#include <QtCore>
#include <QItemSelectionModel>

#include <klocale.h>
#include <kdebug.h>
#include <kstandardaction.h>
#include <kstandarddirs.h>
#include <kaction.h>
#include <ktoolbar.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <kmenu.h>
#include <KHTMLView>

#include <kcalendarsystem.h>
#include <kabc/addressbook.h>
#include <kabc/stdaddressbook.h>
#include <kabc/addresseedialog.h>
#include <kabc/addressee.h>

#include "models/documentmodel.h"
#include "models/modeltest.h"
#include "models/documentproxymodels.h"
#include "filterheader.h"
#include "docdigestview.h"
#include "documentman.h"
#include "docguardedptr.h"
#include "kraftdoc.h"
#include "defaultprovider.h"
#include "docdigestdetailview.h"

DocDigestView::DocDigestView( QWidget *parent )
: QWidget( parent )
{
  QVBoxLayout *box = new QVBoxLayout;
  setLayout( box );

  box->setMargin( 0 );
  box->setSpacing( 0 );

  QHBoxLayout *hbox = new QHBoxLayout;

  mNewDocButton = new QPushButton( i18n( "Create Document" ) );
  connect( mNewDocButton, SIGNAL( clicked() ), this, SIGNAL( createDocument() ) );
  hbox->addWidget( mNewDocButton );
  hbox->addStretch(1);
  mToolBox = new QToolBox;

  QList<QTreeView *> treelist = initializeTreeWidgets();
  connect( mToolBox, SIGNAL(currentChanged(int)), this, SLOT(slotCurrentChangedToolbox(int)));

  mFilterHeader = new FilterHeader( new QTreeWidget );
  mFilterHeader->showCount( false );

  hbox->addWidget( mFilterHeader );
  hbox->addSpacing( KDialog::marginHint() );

  box->addLayout( hbox );

  QHBoxLayout *hbox2 = new QHBoxLayout;
  hbox2->addWidget( mToolBox );
  hbox2->addSpacing( KDialog::marginHint() );
  box->addLayout( hbox2 );

  QFrame *f = new QFrame;
  f->setLineWidth( 2 );
  f->setMidLineWidth( 3 );
  f->setFrameStyle( QFrame::HLine | QFrame::Raised );
  f->setFixedHeight( 10 );
  box->addWidget( f );

  QHBoxLayout *hbox3 = new QHBoxLayout;

  hbox3->addSpacing( KDialog::marginHint() );

  // hbox3->addWidget( mShowDocDetailsView );
  box->addLayout( hbox3 );
}

DocDigestView::~DocDigestView()
{

}

QList<QTreeView *> DocDigestView::initializeTreeWidgets()
{
  //Note: Currently building the views is done in slotBuildView() that is called from the portal
  //      because otherwise we'd access the database before it is initialized
  mAllView = new QTreeView;
  mLatestView = new QTreeView;
  mTimeView = new QTreeView;

  mTreeViewIndex.resize(3);

  //Add the widgets to a temporary list so we can iterate over them and centralise the common initialization
  treeviewlist.clear();
  treeviewlist.append(mAllView);
  treeviewlist.append(mLatestView);
  treeviewlist.append(mTimeView);

  //Initialise
  mAllMenu = new KMenu( mAllView );
  mAllMenu->setTitle( i18n("Document Actions"));
  mTimelineMenu = new KMenu( mTimeView );
  mTimelineMenu->setTitle( i18n("Document Actions"));
  mLatestMenu = new KMenu( mLatestView );
  mLatestMenu->setTitle( i18n("Document Actions"));

  //Add treewidgets to the toolbox
  QVBoxLayout *vb1 = new QVBoxLayout;
  vb1->setMargin(0);
  vb1->addWidget( mLatestView );
  mLatestViewDetails = new DocDigestDetailView;
  vb1->addWidget( mLatestViewDetails );
  QWidget *w = new QWidget;
  w->setLayout(vb1);
  mLatestViewDetails->setFixedHeight(160);

  int indx = mToolBox->addItem( w, i18n("Latest Documents"));
  mToolBox->setItemIcon( indx, KIcon( "get-hot-new-stuff"));
  mToolBox->setItemToolTip(indx, i18n("Shows the latest ten documents"));
  mTreeViewIndex[indx] = mLatestView;

  indx = mToolBox->addItem( mAllView, i18n("All Documents"));
  mToolBox->setItemIcon( indx, KIcon( "edit-clear-locationbar-ltr"));
  mToolBox->setItemToolTip(indx, i18n("Shows a complete list of all documents"));
  mTreeViewIndex[indx] = mAllView;

  indx = mToolBox->addItem( mTimeView, i18n("Timelined Documents"));
  mToolBox->setItemIcon( indx, KIcon( "chronometer"));
  mToolBox->setItemToolTip(indx, i18n("Shows all documents along a timeline"));
  mTreeViewIndex[indx] = mTimeView;

  return treeviewlist;
}


void DocDigestView::slotCurrentChangedToolbox(int index)
{
  if( index < 0 || index > mTreeViewIndex.size() ) return;

  QTreeView *treeview = mTreeViewIndex[index];

  if(treeview->selectionModel()->hasSelection())
    slotCurrentChanged(treeview->selectionModel()->selectedRows().at(0), QModelIndex());
  else
    slotCurrentChanged(QModelIndex(), QModelIndex());
}

void DocDigestView::slotBuildView()
{
  //Create the latest documents view
  mLatestDocModel = new DocumentFilterModel(10, this);
  mLatestView->setModel(mLatestDocModel);
  mLatestView->sortByColumn(DocumentModel::Document_CreationDate, Qt::DescendingOrder);

  //Create the all documents view
  mAllDocumentsModel = new DocumentFilterModel(-1, this);
  mAllDocumentsModel->setSourceModel(DocumentModel::self());
  mAllView->setModel(mAllDocumentsModel);
  mAllView->sortByColumn(DocumentModel::Document_CreationDate, Qt::DescendingOrder);
  mAllView->setSortingEnabled(true);

  //Create the timeline view
  mTimelineModel = new TimelineModel(this);
  mTimeView->setModel(mTimelineModel);

  //Initialize common style options
  QPalette palette;
  palette.setColor( QPalette::AlternateBase, QColor("#e0fdd1") );

  for(int i=0; i < treeviewlist.count(); ++i) {
    QTreeView *widget = treeviewlist.at(i);
    connect( widget->selectionModel(), SIGNAL( currentRowChanged(QModelIndex,QModelIndex) ),
             this, SLOT(slotCurrentChanged(QModelIndex,QModelIndex)));
    connect( widget, SIGNAL( doubleClicked(QModelIndex) ),
             this, SLOT( slotDocOpenRequest(QModelIndex) ) );

    widget->setAnimated( true );
    widget->setPalette( palette );
    widget->setAlternatingRowColors( true );
    widget->setRootIsDecorated( true );
    widget->setSelectionMode( QAbstractItemView::SingleSelection );
    widget->header()->setResizeMode(QHeaderView::ResizeToContents);
   // widget->header()->setResizeMode( DocumentModel::Document_Whiteboard, QHeaderView::Stretch );
    widget->setEditTriggers( QAbstractItemView::NoEditTriggers );
    widget->setExpandsOnDoubleClick( false );
  }
}

void DocDigestView::contextMenuEvent( QContextMenuEvent * event )
{
  QTreeView *currView = mTreeViewIndex[ mToolBox->currentIndex() ];

  if( currView == mLatestView ) {
    mLatestMenu->popup( event->globalPos() );
  } else if( currView == mTimeView ) {
    mTimelineMenu->popup( event->globalPos() );
  } else if( currView == mAllView ) {
    mAllMenu->popup( event->globalPos() );
  }
}

#if 0
void DocDigestView::slotDocViewRequest( QTreeWidgetItem *item )
{
  QString id = mDocIdDict[ item ];
  if( ! id.isEmpty() ) {
    kDebug() << "Opening document " << id;

    emit viewDocument( id );
  }
}
#endif

void DocDigestView::slotDocOpenRequest( QModelIndex index )
{
  Q_UNUSED(index);
  QModelIndex idIndx = index.sibling( index.row(), DocumentModel::Document_Ident );
  QString id = idIndx.data( Qt::DisplayRole ).toString();

  kDebug() << "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO "<< "about to open: " << id;
  if( index.data(DocumentModel::DataType) == DocumentModel::DocumentType)
    emit openDocument( id );
  else if( index.data(DocumentModel::DataType) == DocumentModel::ArchivedType)
    emit openArchivedDocument( idIndx.parent().data( Qt::DisplayRole).toString(),
                              idIndx.data( Qt::DisplayRole).toString() );
}

int DocDigestView::currentDocumentRow() const
{
    if(mCurrentlySelected.data(DocumentModel::DataType) == DocumentModel::DocumentType)
        return mCurrentlySelected.row();
    else if(mCurrentlySelected.data(DocumentModel::DataType) == DocumentModel::ArchivedType)
        return mCurrentlySelected.parent().row();
    else
        return -1;
}

int DocDigestView::currentArchivedRow() const
{
    if(mCurrentlySelected.data(DocumentModel::DataType) == DocumentModel::ArchivedType)
        return mCurrentlySelected.row();
    else
        return -1;
}

QString DocDigestView::currentDocumentId( ) const
{
  QModelIndex indx = mCurrentlySelected.sibling( mCurrentlySelected.row(), DocumentModel::Document_Id);

  const QString data = indx.data(Qt::DisplayRole).toString();
  kDebug() << "This is the current selected docID: " << data;
  return data;
}

void DocDigestView::slotCurrentChanged( QModelIndex index, QModelIndex previous )
{
  Q_UNUSED(previous);

  if(index.isValid())
  {
    DocumentModel *model = 0;
    DocDigestDetailView *view = 0;

    if(index.model() == static_cast<QAbstractItemModel*>(mLatestDocModel)) {
      mCurrentlySelected = mLatestDocModel->mapToSource(index);
      view = mLatestViewDetails;
      model = static_cast<DocumentModel*>(mLatestDocModel->sourceModel());
    } else if(index.model() == static_cast<QAbstractItemModel*>(mTimelineModel)) {
      mCurrentlySelected = mTimelineModel->mapToSource(index);
      model = static_cast<DocumentModel*>( mTimelineModel->sourceModel() );
      view = mAllViewDetails;
    } else {
      mCurrentlySelected = mAllDocumentsModel->mapToSource(index);
      model = static_cast<DocumentModel*>( mAllDocumentsModel->sourceModel() );
      view = mTimeLineViewDetails;
    }

    if(mCurrentlySelected.data(DocumentModel::DataType) == DocumentModel::DocumentType) {
      QModelIndex idIndx = mCurrentlySelected.sibling( mCurrentlySelected.row(), DocumentModel::Document_Ident );
      QString id = idIndx.data( Qt::DisplayRole ).toString();

      emit docSelected( id );
      view->slotShowDocDetails( model->digest( index ));

    } else if(mCurrentlySelected.data(DocumentModel::DataType) == DocumentModel::ArchivedType) {
      emit archivedDocSelected( mCurrentlySelected.parent().data( Qt::DisplayRole).toString(),
                               mCurrentlySelected.data( Qt::DisplayRole).toString() );
    } else {
      emit docSelected( QString() );

    }
  } else {
    emit docSelected( QString() );
  }
  //kDebug() << "Supposed row: " << sourceIndex.row() << " Supposed ID: " << DocumentModel::self()->data(sourceIndex, Qt::DisplayRole);
}

QList<KMenu*> DocDigestView::contextMenus()
{
  QList<KMenu*> menus;
  menus.append( mAllMenu);
  menus.append( mTimelineMenu );
  menus.append( mLatestMenu);

  return menus;
}

