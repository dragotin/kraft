/***************************************************************************
                            alldocsview.cpp 
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
#include <QLocale>
#include <QDebug>
#include <QDialog>
#include <QMenu>
#include <QVBoxLayout>
#include <QToolBox>
#include <QHeaderView>
#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>

#include <KLocalizedString>

#include "models/documentmodel.h"
#include "models/modeltest.h"
#include "models/documentproxymodels.h"
#include "filterheader.h"
#include "alldocsview.h"
#include "documentman.h"
#include "docguardedptr.h"
#include "kraftdoc.h"
#include "defaultprovider.h"
#include "docdigestdetailview.h"
#include "kraftsettings.h"

AllDocsView::AllDocsView( QWidget *parent )
: QWidget( parent )
{
  QVBoxLayout *box = new QVBoxLayout;
  setLayout( box );

  box->setMargin( 0 );
  box->setSpacing( 0 );

  _searchLine = new QLineEdit(this);
  _searchLine->setClearButtonEnabled(true);
  _searchLine->setMinimumWidth(200);
  connect( _searchLine, SIGNAL(textChanged(QString)), this, SLOT(slotSearchTextChanged(QString)) );

  QComboBox *filterCombo = new QComboBox;
  filterCombo->addItem("Recent 10 Documents");
  filterCombo->addItem("All documents");
  filterCombo->addItem("Document Type");

  QHBoxLayout *hbox = new QHBoxLayout;
  QLabel *l1 = new QLabel(i18n("&Show: "));
  l1->setBuddy(filterCombo);

  hbox->addWidget(l1);
  hbox->addWidget(filterCombo);
  hbox->addStretch( 2 );
  QLabel *lab = new QLabel;
  lab->setText( i18n("&Search: ") );
  lab->setBuddy( _searchLine);
  hbox->addWidget( lab );
  hbox->addWidget( _searchLine);
  box->addLayout( hbox );

  QFrame *f = new QFrame;
  f->setLineWidth( 2 );
  f->setMidLineWidth( 3 );
  f->setFrameStyle( QFrame::HLine | QFrame::Raised );
  f->setFixedHeight( 10 );
  box->addWidget( f );

  box->addWidget(initializeTreeWidget());

}

AllDocsView::~AllDocsView()
{
  QString state = mAllView->horizontalHeader()->saveState().toBase64();
  KraftSettings::self()->setDigestListColumnsAll( state );
  KraftSettings::self()->save();
}

void AllDocsView::slotSearchTextChanged(const QString& newStr )
{
    mAllDocumentsModel->setFilterRegExp(newStr);
}

QWidget* AllDocsView::initializeTreeWidget()
{
  //Note: Currently building the views is done in slotBuildView() that is called from the portal
  //      because otherwise we'd access the database before it is initialized
  mAllView =    new QTableView;

  //Initialise
  mAllMenu = new QMenu( mAllView );
  mAllMenu->setTitle( i18n("Document Actions"));

  //Add treewidgets to the toolbox: All docs view
  QVBoxLayout *vb1 = new QVBoxLayout;
  vb1->setMargin(0);
  vb1->addWidget( mAllView );

  mAllViewDetails = new DocDigestDetailView;
  connect( mAllViewDetails, SIGNAL( showLastPrint( const dbID& ) ),
           this, SLOT( slotOpenLastPrinted() ) );

  vb1->addWidget( mAllViewDetails );
  QWidget *w = new QWidget;
  w->setLayout(vb1);
  mAllViewDetails->setFixedHeight(160);
  //
  return w;
}


void AllDocsView::slotBuildView()
{
    QByteArray headerStateAll = QByteArray::fromBase64( KraftSettings::self()->digestListColumnsAll().toAscii() );
    //Create the latest documents view

    //Create the all documents view
    mAllDocumentsModel = new DocumentFilterModel(-1, this);
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

    //Initialize common style options
    QPalette palette;
    palette.setColor( QPalette::AlternateBase, QColor("#e0fdd1") );

    connect( mAllView->selectionModel(), SIGNAL( currentRowChanged(QModelIndex,QModelIndex) ),
             this, SLOT(slotCurrentChanged(QModelIndex,QModelIndex)));
    connect( mAllView, SIGNAL( doubleClicked(QModelIndex) ),
             this, SLOT( slotDocOpenRequest(QModelIndex) ) );

    mAllView->setPalette( palette );
    mAllView->setAlternatingRowColors( true );
    mAllView->setSelectionMode( QAbstractItemView::SingleSelection );
    mAllView->setEditTriggers( QAbstractItemView::NoEditTriggers );
    // mAllView->setExpandsOnDoubleClick( false );
}

void AllDocsView::slotUpdateView()
{
  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
  static_cast<DocumentModel*>(mAllDocumentsModel->sourceModel())->setQueryAgain();
  QApplication::restoreOverrideCursor();
}

void AllDocsView::contextMenuEvent( QContextMenuEvent * event )
{
    mAllMenu->popup( event->globalPos() );
}

void AllDocsView::slotOpenLastPrinted( )
{
  // qDebug () << "slotOpenLastPrinted hit! ";
  emit openArchivedDocument( mLatestArchivedDigest );
}

void AllDocsView::slotDocOpenRequest( QModelIndex index )
{
  QModelIndex idIndx = index.sibling( index.row(), DocumentModel::Document_Id );
  const QString id = idIndx.data( Qt::DisplayRole ).toString();

  // qDebug () << "Double click open document ident " << id;
  emit openDocument( id );
}

int AllDocsView::currentDocumentRow() const
{
  return mCurrentlySelected.row();
}

QString AllDocsView::currentDocumentId( ) const
{
  QModelIndex indx = mCurrentlySelected.sibling( mCurrentlySelected.row(), DocumentModel::Document_Id);

  const QString data = indx.data(Qt::DisplayRole).toString();
  // qDebug () << "This is the current selected docID: " << data;
  return data;
}

void AllDocsView::slotCurrentChanged( QModelIndex index, QModelIndex previous )
{
  Q_UNUSED(previous);

  if(index.isValid()) {
    DocumentModel *model = 0;
    DocDigestDetailView *view = 0;

    // qDebug () << "Picking AllDocumentsView!";
    mCurrentlySelected = mAllDocumentsModel->mapToSource(index);
    model = static_cast<DocumentModel*>( mAllDocumentsModel->sourceModel() );
    view = mAllViewDetails;

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
    // qDebug () << "Got invalid index, clearing digest view.";
    emit docSelected( QString() );
  }
  //// qDebug () << "Supposed row: " << sourceIndex.row() << " Supposed ID: " << DocumentModel::self()->data(sourceIndex, Qt::DisplayRole);
}

QVector<QMenu*> AllDocsView::contextMenus()
{
  QVector<QMenu*> menus;
  menus.append( mAllMenu);

  return menus;
}

