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
#include <QStackedWidget>

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
: QWidget( parent ),
  mTableModel(0),
  mDateModel(0)
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
  filterCombo->addItem("All documents");
  filterCombo->addItem("Documents of last week");
  filterCombo->addItem("Documents of last month");
  // filterCombo->addItem("Document Type");
  connect( filterCombo, SIGNAL(activated(int)),
           this, SLOT(slotAmountFilterChanged(int)));

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
    const QByteArray state = _tableView->horizontalHeader()->saveState().toBase64();
    KraftSettings::self()->setDigestListColumnsAll( state );

    const QByteArray state1 = _dateView->header()->saveState().toBase64();
    KraftSettings::self()->setDigestListColumnsTime(state1);

    KraftSettings::self()->save();
}

void AllDocsView::slotAmountFilterChanged(int entryNo)
{
    int num = -1;
    if( entryNo == 1 ) {
        num = 7;
    } else if( entryNo == 2 ) {
        num = 31;
    }
    mTableModel->setMaxRows(num);
    mDateModel->setMaxRows(num);
}

void AllDocsView::slotSearchTextChanged(const QString& newStr )
{
    mTableModel->setFilterRegExp(newStr);
    mDateModel->setFilterRegExp(newStr);
}

QWidget* AllDocsView::initializeTreeWidget()
{
  //Note: Currently building the views is done in slotBuildView() that is called from the portal
  //      because otherwise we'd access the database before it is initialized
  _tableView = new QTableView;
  _dateView = new QTreeView;

  //Initialise
  mAllMenu = new QMenu( _tableView );
  mAllMenu->setTitle( i18n("Document Actions"));

  //Add treewidgets to the toolbox: All docs view
  QVBoxLayout *vb1 = new QVBoxLayout;
  vb1->setMargin(0);
  _stack = new QStackedWidget(this);
  _stack->addWidget(_tableView);
  _stack->addWidget(_dateView);

  vb1->addWidget( _stack );

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

void AllDocsView::setView( ViewType type )
{
    // change the document listing widget
    if( type == FlatList) {
        _stack->setCurrentIndex(0);
    } else {
        _stack->setCurrentIndex(1);
    }
    // clear the details view
    mAllViewDetails->slotClearView();

    mCurrentlySelected = QModelIndex();
}

void AllDocsView::slotBuildView()
{
    const QByteArray headerStateTable = QByteArray::fromBase64( KraftSettings::self()->digestListColumnsAll().toAscii() );
    const QByteArray headerStateDate =  QByteArray::fromBase64( KraftSettings::self()->digestListColumnsTime().toAscii() );

    mDateModel = new DocumentFilterModel(-1, this);
    mDateModel->setEnableTreeview(true);
    mTableModel = new DocumentFilterModel(-1, this);
    mTableModel->setEnableTreeview(false);

    _tableView->setModel(mTableModel);
    _dateView->setModel(mDateModel);

    _tableView->sortByColumn(DocumentModel::Document_CreationDate, Qt::DescendingOrder);
    _tableView->verticalHeader()->hide();
    _tableView->setSortingEnabled(true);
    _tableView->horizontalHeader()->setMovable( true );
    _tableView->horizontalHeader()->setSortIndicatorShown( true );
    _tableView->horizontalHeader()->restoreState( headerStateTable );
    _tableView->setSelectionBehavior( QAbstractItemView::SelectRows );
    _tableView->setShowGrid( false );
    _tableView->hideColumn( DocumentModel::Document_Id );
    _tableView->hideColumn( DocumentModel::Document_ClientId );
    _tableView->hideColumn( DocumentModel::Document_ClientAddress );
    _tableView->showColumn( DocumentModel::Document_ClientName );
    _tableView->hideColumn( DocumentModel::Document_CreationDateRaw);
    _tableView->hideColumn( DocumentModel::Document_Id_Raw);

    _dateView->hideColumn( DocumentModel::Document_ClientId );
    _dateView->hideColumn( DocumentModel::Document_ClientAddress );
    _dateView->showColumn( DocumentModel::Document_ClientName );
    _dateView->hideColumn( DocumentModel::Document_CreationDateRaw);
    _dateView->hideColumn( DocumentModel::Document_Id_Raw);
    _dateView->header()->restoreState( headerStateDate );

    //Initialize common style options

    connect( _tableView->selectionModel(), SIGNAL( currentRowChanged(QModelIndex,QModelIndex) ),
             this, SLOT(slotCurrentChanged(QModelIndex,QModelIndex)));
    connect( _dateView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
             this, SLOT(slotCurrentChanged(QModelIndex,QModelIndex)));

    connect( _tableView, SIGNAL( doubleClicked(QModelIndex) ),
             this, SLOT( slotDocOpenRequest(QModelIndex) ) );
    connect( _dateView, SIGNAL( doubleClicked(QModelIndex) ),
             this, SLOT( slotDocOpenRequest(QModelIndex) ) );

 //   _tableView->setPalette( palette );
 //   _dateView->setPalette( palette );
    _tableView->setAlternatingRowColors( true );
    _dateView->setAlternatingRowColors(false);
    _tableView->setSelectionMode( QAbstractItemView::SingleSelection );
    _tableView->setEditTriggers( QAbstractItemView::NoEditTriggers );
    _dateView->setEditTriggers( QAbstractItemView::NoEditTriggers );
    _dateView->setExpandsOnDoubleClick( false );
    slotUpdateView();

}

void AllDocsView::slotUpdateView()
{
  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

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
    Q_UNUSED(index);
    const QString id = currentDocumentId();

    emit openDocument( id );
}

int AllDocsView::currentDocumentRow() const
{
  return mCurrentlySelected.row();
}

QString AllDocsView::currentDocumentId( ) const
{
    bool isDoc = true;
    DocBaseModel *model;

    if( _stack->currentIndex() == 0 ) {
        model = static_cast<DocumentModel*>( mTableModel->sourceModel() );
    } else {
        model = static_cast<DateModel*>( mDateModel->sourceModel() );
        isDoc = model->isDocument(mCurrentlySelected);
    }

    QString id;
    if( isDoc ) {
        QModelIndex idIndx = model->index(mCurrentlySelected.row(),
                                          DocumentModel::Document_Id_Raw, mCurrentlySelected.parent());

        id = idIndx.data( Qt::DisplayRole ).toString();
    }
    return id;
}

void AllDocsView::slotCurrentChanged( QModelIndex index, QModelIndex previous )
{
    Q_UNUSED(previous);

    if(index.isValid()) {

        DocBaseModel *model;
        bool isDoc = true;

        if( _stack->currentIndex() == 0 ) {
            mCurrentlySelected = mTableModel->mapToSource(index);
            model = static_cast<DocumentModel*>( mTableModel->sourceModel() );
        } else {
            mCurrentlySelected = mDateModel->mapToSource(index);
            model = static_cast<DateModel*>( mDateModel->sourceModel() );
            isDoc = model->isDocument(mCurrentlySelected);
        }

        /* get the corresponding document id */
        DocDigest digest;
        if( isDoc ) {
            QModelIndex idIndx = model->index(mCurrentlySelected.row(),
                                              DocumentModel::Document_Ident,
                                              mCurrentlySelected.parent());

            const QString id = idIndx.data( Qt::DisplayRole ).toString();

            emit docSelected( id );
            digest = model->digest( /* index */ mCurrentlySelected );
            mAllViewDetails->slotShowDocDetails( digest );
            if( digest.archDocDigestList().size() > 0 ) {
                mLatestArchivedDigest = digest.archDocDigestList()[0];
            } else {
                mLatestArchivedDigest = ArchDocDigest();
            }
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

