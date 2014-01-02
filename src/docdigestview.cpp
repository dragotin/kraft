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

DocDigestView::DocDigestView( QWidget *parent )
: QWidget( parent ),
  mOldToolboxIndex( -1 )
{
  QVBoxLayout *box = new QVBoxLayout;
  setLayout( box );

  box->setMargin( 0 );
  box->setSpacing( 0 );

  mToolBox = new QToolBox;
  initializeTreeWidgets();
  connect( mToolBox, SIGNAL(currentChanged(int)), this, SLOT(slotCurrentChangedToolbox(int)));

  mFilterHeader = new KTreeViewSearchLine( this );
  mFilterHeader->setMinimumWidth( 200 );
  mFilterHeader->setKeepParentsVisible( true );

  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->insertStretch( 0, 2 );
  QLabel *lab = new QLabel;
  lab->setText( i18n("&Search: ") );
  lab->setBuddy( mFilterHeader );
  hbox->addWidget( lab );
  hbox->addWidget( mFilterHeader );
  box->addLayout( hbox );

  box->addWidget( mToolBox );


  QFrame *f = new QFrame;
  f->setLineWidth( 2 );
  f->setMidLineWidth( 3 );
  f->setFrameStyle( QFrame::HLine | QFrame::Raised );
  f->setFixedHeight( 10 );
  box->addWidget( f );
}

DocDigestView::~DocDigestView()
{
  QString state = mLatestView->horizontalHeader()->saveState().toBase64();
  KraftSettings::self()->setDigestListColumnsLatest( state );
  state = mAllView->horizontalHeader()->saveState().toBase64();
  KraftSettings::self()->setDigestListColumnsAll( state );
  state = mTimeView->header()->saveState().toBase64();
  KraftSettings::self()->setDigestListColumnsTime( state );
  KraftSettings::self()->writeConfig();
}

void DocDigestView::initializeTreeWidgets()
{
  //Note: Currently building the views is done in slotBuildView() that is called from the portal
  //      because otherwise we'd access the database before it is initialized
  mAllView =    new QTableView;
  mLatestView = new QTableView;
  mTimeView =   new QTreeView;

  mTreeViewIndex.resize(3);

  //Add the widgets to a temporary list so we can iterate over them and centralize the common initialization
  mTreeViewList.clear();
  mTreeViewList.append(mAllView);
  mTreeViewList.append(mLatestView);
  mTreeViewList.append(mTimeView);

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
}


void DocDigestView::slotCurrentChangedToolbox(int index)
{
  kDebug() << "INDEX: " << index;
  if( index < 0 || index > mTreeViewIndex.size() ) return;

  // move the state of the columns from one view to the other
//  if( mOldToolboxIndex > -1 && mOldToolboxIndex < mTreeViewIndex.size() ) {
//    mTreeViewIndex[index]->header()->restoreState( mTreeViewIndex[mOldToolboxIndex]->header()->saveState() );
//    if( mTreeViewIndex[index] == mTimeView )
//      mTimeView->showColumn( DocumentModel::Document_Id );
//    else
//      mTreeViewIndex[index]->hideColumn( DocumentModel::Document_Id );
//  }
  mOldToolboxIndex = index;

  if( index == 0 ) { // latest
    mLatestDocModel->setMaxRows(12);
   } else if( index == 1 ) { // all

    kDebug()<< "SHOWING all rows!";
    mLatestDocModel->setMaxRows( -1 );
  }
  // QTreeView *treeview = mTreeViewIndex[index];
  QAbstractItemView *treeview = mTreeViewIndex[index];
  if(treeview->selectionModel()->hasSelection())
    slotCurrentChanged(treeview->selectionModel()->selectedRows().at(0), QModelIndex());
  else
    slotCurrentChanged(QModelIndex(), QModelIndex());
}

void DocDigestView::slotBuildView()
{
  QByteArray headerStateLatest = QByteArray::fromBase64( KraftSettings::self()->digestListColumnsLatest().toAscii() );
  QByteArray headerStateAll = QByteArray::fromBase64( KraftSettings::self()->digestListColumnsAll().toAscii() );
  QByteArray headerStateTime = QByteArray::fromBase64( KraftSettings::self()->digestListColumnsTime().toAscii() );
  //Create the latest documents view
  mLatestDocModel = new DocumentFilterModel(10, this);
  mLatestView->setModel( mLatestDocModel );
  mLatestView->sortByColumn(DocumentModel::Document_CreationDate, Qt::AscendingOrder);
  mLatestView->horizontalHeader()->setMovable( true );
  mLatestView->verticalHeader()->hide();
  mLatestView->setSortingEnabled(true);
  mLatestView->horizontalHeader()->restoreState( headerStateLatest );
  mLatestView->horizontalHeader()->setSortIndicatorShown( true );
  mLatestView->setSelectionBehavior( QAbstractItemView::SelectRows );
  mLatestView->setShowGrid( false );
  mLatestView->hideColumn( DocumentModel::Document_Id );
  mLatestView->hideColumn( DocumentModel::Document_ClientId );
  mLatestView->hideColumn( DocumentModel::Document_ClientAddress );
  mLatestView->showColumn( DocumentModel::Document_ClientName );

  //Create the all documents view
  mAllDocumentsModel = mLatestDocModel;
  mAllView->setModel(mAllDocumentsModel);
  mAllView->sortByColumn(DocumentModel::Document_CreationDate, Qt::DescendingOrder);
  mAllView->verticalHeader()->hide();
  mAllView->setSortingEnabled(true);
  mAllView->horizontalHeader()->setMovable( true );
  mAllView->horizontalHeader()->setSortIndicatorShown( true );
  mAllView->horizontalHeader()->restoreState( headerStateAll );
  mAllView->setSelectionBehavior( QAbstractItemView::SelectRows );
  mAllView->setShowGrid( false );
  mAllView->hideColumn( DocumentModel::Document_Id );
  mAllView->hideColumn( DocumentModel::Document_ClientId );
  mAllView->hideColumn( DocumentModel::Document_ClientAddress );
  mAllView->showColumn( DocumentModel::Document_ClientName );

  //Create the timeline view
  mTimelineModel = new TimelineModel(this);
  mTimeView->setModel(mTimelineModel);
  mTimeView->setSortingEnabled(false);
  mTimeView->header()->setMovable( false );
  mTimeView->showColumn( DocumentModel::Document_Id );
  mTimeView->header()->restoreState( headerStateTime);
  mTimeView->setRootIsDecorated( true );
  mTimeView->setUniformRowHeights( true );
  mTimeView->hideColumn( DocumentModel::Document_ClientId );
  mTimeView->hideColumn( DocumentModel::Document_ClientAddress );
  mTimeView->showColumn( DocumentModel::Document_ClientName );

  //Initialize common style options
  QPalette palette;
  palette.setColor( QPalette::AlternateBase, QColor("#e0fdd1") );

  QList<QAbstractItemView*> treeviewlist;
  treeviewlist.append( mLatestView );
  treeviewlist.append( mAllView );
  treeviewlist.append( mTimeView );

  foreach( QAbstractItemView *widget, treeviewlist ) {
    connect( widget->selectionModel(), SIGNAL( currentRowChanged(QModelIndex,QModelIndex) ),
             this, SLOT(slotCurrentChanged(QModelIndex,QModelIndex)));
    connect( widget, SIGNAL( doubleClicked(QModelIndex) ),
             this, SLOT( slotDocOpenRequest(QModelIndex) ) );

    widget->setPalette( palette );
    widget->setAlternatingRowColors( true );
    widget->setSelectionMode( QAbstractItemView::SingleSelection );
    widget->setEditTriggers( QAbstractItemView::NoEditTriggers );
    // widget->setExpandsOnDoubleClick( false );
  }

  mFilterHeader->addTableView( mLatestView );
  mFilterHeader->addTableView( mAllView );
  mFilterHeader->setTreeView( mTimeView );
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
  QAbstractItemView *currView = mTreeViewIndex[ mToolBox->currentIndex() ];

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

  if(index.isValid()) {
    DocumentModel *model = 0;
    DocDigestDetailView *view = 0;

    int toolboxIndx = mToolBox->currentIndex();
    if( toolboxIndx == 0 ) {
      mCurrentlySelected = mLatestDocModel->mapToSource(index);
      model = static_cast<DocumentModel*>(mLatestDocModel->sourceModel());
      view = mLatestViewDetails;
    } else if( toolboxIndx == 1 ) {
      kDebug() << "Picking AllDocumentsView!";
      mCurrentlySelected = mAllDocumentsModel->mapToSource(index);
      model = static_cast<DocumentModel*>( mAllDocumentsModel->sourceModel() );
      view = mAllViewDetails;
    } else if( toolboxIndx == 2 ) {
      mCurrentlySelected = mTimelineModel->mapToSource(index);
      model = static_cast<DocumentModel*>( mTimelineModel->sourceModel() );
      view = mTimeLineViewDetails;
    }

    /* get the corresponding document id */
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
    kDebug() << "Got invalid index, clearing digest view.";
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

