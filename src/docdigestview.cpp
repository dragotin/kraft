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
#include <ktreeviewsearchline.h>

#include <kcalendarsystem.h>

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
#include "kraftsettings.h"
#include "ktreeviewsearchline.h"

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

  initializeTreeWidgets();
  connect( mToolBox, SIGNAL(currentChanged(int)), this, SLOT(slotCurrentChangedToolbox(int)));

#if 0
  mFilterHeader = new KTreeViewSearchLine( this );

  hbox->addWidget( mFilterHeader );
  hbox->addSpacing( KDialog::marginHint() );

  box->addLayout( hbox );
#endif
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
  const QString state = mLatestView->header()->saveState().toBase64();
  KraftSettings::self()->setDigestListColumns( state );
  KraftSettings::self()->writeConfig();
}

QList<QTreeView *> DocDigestView::initializeTreeWidgets()
{
  //Note: Currently building the views is done in slotBuildView() that is called from the portal
  //      because otherwise we'd access the database before it is initialized
  mAllView =    new QTreeView;
  mLatestView = new QTreeView;
  mTimeView =   new QTreeView;

  mLatestView->setRootIsDecorated( false );
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

  //Add treewidgets to the toolbox: Latest Docs view
  QVBoxLayout *vb1 = new QVBoxLayout;
  vb1->setMargin(0);
  vb1->addWidget( mLatestView );

  mLatestViewDetails = new DocDigestDetailView;
  connect( mLatestViewDetails, SIGNAL( showLastPrint( const dbID& ) ),
           this, SLOT( slotOpenLastPrinted() ) );

  vb1->addWidget( mLatestViewDetails );
  QWidget *w = new QWidget;
  w->setLayout(vb1);
  mLatestViewDetails->setFixedHeight(160);
  //

  int indx = mToolBox->addItem( w, i18n("Latest Documents"));
  mToolBox->setItemIcon( indx, KIcon( "get-hot-new-stuff"));
  mToolBox->setItemToolTip(indx, i18n("Shows the latest ten documents"));
  mTreeViewIndex[indx] = mLatestView;

  //Add treewidgets to the toolbox: All docs view
  vb1 = new QVBoxLayout;
  vb1->setMargin(0);
  vb1->addWidget( mAllView );

  mAllViewDetails = new DocDigestDetailView;
  connect( mAllViewDetails, SIGNAL( showLastPrint( const dbID& ) ),
           this, SLOT( slotOpenLastPrinted() ) );

  vb1->addWidget( mAllViewDetails );
  w = new QWidget;
  w->setLayout(vb1);
  mAllViewDetails->setFixedHeight(160);
  //

  indx = mToolBox->addItem( w, i18n("All Documents"));
  mToolBox->setItemIcon( indx, KIcon( "edit-clear-locationbar-ltr"));
  mToolBox->setItemToolTip(indx, i18n("Shows a complete list of all documents"));
  mTreeViewIndex[indx] = mAllView;

  //Add treewidgets to the toolbox: Timeline view
  vb1 = new QVBoxLayout;
  vb1->setMargin(0);
  vb1->addWidget( mTimeView );

  mTimeLineViewDetails = new DocDigestDetailView;
  connect( mTimeLineViewDetails, SIGNAL( showLastPrint( const dbID& ) ),
           this, SLOT( slotOpenLastPrinted() ) );

  vb1->addWidget( mTimeLineViewDetails );
  w = new QWidget;
  w->setLayout(vb1);
  mTimeLineViewDetails->setFixedHeight(160);
  //

  indx = mToolBox->addItem( w, i18n("Timelined Documents"));
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
  mLatestView->setModel( mLatestDocModel );
  mLatestView->sortByColumn(DocumentModel::Document_CreationDate, Qt::AscendingOrder);
  mLatestView->hideColumn( DocumentModel::Document_ClientId );
  mLatestView->hideColumn( DocumentModel::Document_ClientAddress );
  mLatestView->setSortingEnabled(true);
  mLatestView->header()->restoreState( QByteArray::fromBase64( KraftSettings::self()->digestListColumns().toAscii() ) );

  //Create the all documents view
  mAllDocumentsModel = new DocumentFilterModel(-1, this);
  mAllDocumentsModel->setSourceModel( new DocumentModel ); // ::self());
  mAllView->setModel(mAllDocumentsModel);
  mAllView->sortByColumn(DocumentModel::Document_CreationDate, Qt::DescendingOrder);
  mAllView->hideColumn( DocumentModel::Document_ClientId );
  mAllView->hideColumn( DocumentModel::Document_ClientAddress );
  mAllView->setSortingEnabled(true);

  //Create the timeline view
  mTimelineModel = new TimelineModel(this);
  mTimeView->setModel(mTimelineModel);
  mTimeView->hideColumn( DocumentModel::Document_ClientId );
  mTimeView->hideColumn( DocumentModel::Document_ClientAddress );
  mTimeView->setSortingEnabled(false);

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
    widget->header()->setResizeMode(QHeaderView::Interactive);
   // widget->header()->setResizeMode( DocumentModel::Document_Whiteboard, QHeaderView::Stretch );
    widget->setEditTriggers( QAbstractItemView::NoEditTriggers );
    widget->setExpandsOnDoubleClick( false );
    widget->setUniformRowHeights( true );
  }

#if 0
  mFilterHeader->setTreeView( mAllView );
  mFilterHeader->setTreeView( mLatestView );
  mFilterHeader->setTreeView( mTimeView );
#endif
}

void DocDigestView::slotUpdateView()
{
  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
  static_cast<DocumentModel*>(mLatestDocModel->sourceModel())->setQueryAgain();
  static_cast<DocumentModel*>(mTimelineModel->baseModel())->setQueryAgain();
  static_cast<DocumentModel*>(mAllDocumentsModel->sourceModel())->setQueryAgain();
  QApplication::restoreOverrideCursor();
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

void DocDigestView::slotOpenLastPrinted( )
{
  kDebug() << "slotOpenLastPrinted hit! ";
  emit openArchivedDocument( mLatestArchivedDigest );
}

void DocDigestView::slotDocOpenRequest( QModelIndex index )
{
  QModelIndex idIndx = index.sibling( index.row(), DocumentModel::Document_Id );
  const QString id = idIndx.data( Qt::DisplayRole ).toString();

  kDebug() << "Double click open document ident " << id;
  emit openDocument( id );
}

int DocDigestView::currentDocumentRow() const
{
  return mCurrentlySelected.row();
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
      model = static_cast<DocumentModel*>(mLatestDocModel->sourceModel());
      view = mLatestViewDetails;
    } else if(index.model() == static_cast<QAbstractItemModel*>(mTimelineModel)) {
      mCurrentlySelected = mTimelineModel->mapToSource(index);
      model = static_cast<DocumentModel*>( mTimelineModel->sourceModel() );
      view = mTimeLineViewDetails;
    } else {
      mCurrentlySelected = mAllDocumentsModel->mapToSource(index);
      model = static_cast<DocumentModel*>( mAllDocumentsModel->sourceModel() );
      view = mAllViewDetails;
    }

    QModelIndex idIndx = mCurrentlySelected.sibling( mCurrentlySelected.row(), DocumentModel::Document_Ident );
    QString id = idIndx.data( Qt::DisplayRole ).toString();

    emit docSelected( id );
    DocDigest digest = model->digest( /* index */ mCurrentlySelected );
    view->slotShowDocDetails( digest );
    if( digest.archDocDigestList().size() > 0 ) {
      mLatestArchivedDigest = digest.archDocDigestList()[0];
    } else {
      mLatestArchivedDigest = ArchDocDigest();
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

