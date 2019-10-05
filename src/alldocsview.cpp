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
  filterCombo->addItem(i18n("All documents"));
  filterCombo->addItem(i18n("Documents of last week"));
  filterCombo->addItem(i18n("Documents of last month"));
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
  //
  return w;
}

void AllDocsView::setView( ViewType type )
{
    // change the document listing widget
    QModelIndex current;
    if( type == FlatList) {
        _stack->setCurrentIndex(0);
        current = _tableView->currentIndex();
    } else {
        _stack->setCurrentIndex(1);
        current = _dateView->currentIndex();
    }
    // clear the details view
    mAllViewDetails->slotClearView();

    if( current.isValid() > 0 ) {
        slotCurrentChanged(current, QModelIndex());
    } else {
        // workaround, not cool.
        mCurrentlySelected = QModelIndex();
    }
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

    _tableView->sortByColumn(DocumentModel::Document_CreationDateRaw, Qt::DescendingOrder);
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
    _tableView->showColumn( DocumentModel::Document_CreationDateRaw);
    _tableView->hideColumn( DocumentModel::Document_CreationDate);
    _tableView->hideColumn( DocumentModel::Document_Id_Raw);
    _tableView->hideColumn( DocumentModel::Treestruct_Type);
    _tableView->hideColumn( DocumentModel::Treestruct_Month);
    _tableView->hideColumn( DocumentModel::Treestruct_Year);

    _dateView->header()->restoreState( headerStateDate );

    _dateView->hideColumn( DocumentModel::Document_ClientId );
    _dateView->hideColumn( DocumentModel::Document_ClientAddress );
    _dateView->showColumn( DocumentModel::Document_ClientName );
    _dateView->showColumn( DocumentModel::Document_CreationDateRaw);
    _dateView->hideColumn( DocumentModel::Document_CreationDate);

    _dateView->hideColumn( DocumentModel::Document_Id_Raw);
    _dateView->hideColumn( DocumentModel::Treestruct_Type);
    _dateView->hideColumn( DocumentModel::Treestruct_Month);
    _dateView->hideColumn( DocumentModel::Treestruct_Year);

    _dateView->setSelectionBehavior( QAbstractItemView::SelectRows );

    //Initialize common style options
    connect( _tableView->selectionModel(), SIGNAL( currentRowChanged(QModelIndex,QModelIndex) ),
             this, SLOT(slotCurrentChanged(QModelIndex,QModelIndex)));
    connect( _dateView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
             this, SLOT(slotCurrentChanged(QModelIndex,QModelIndex)));

    connect( _tableView, SIGNAL( doubleClicked(QModelIndex) ),
             this, SLOT( slotDocOpenRequest(QModelIndex) ) );
    connect( _dateView, SIGNAL( doubleClicked(QModelIndex) ),
             this, SLOT( slotDocOpenRequest(QModelIndex) ) );

    _tableView->setAlternatingRowColors( true );
    _dateView->setAlternatingRowColors(false);
    _tableView->setSelectionMode( QAbstractItemView::SingleSelection );
    _tableView->setEditTriggers( QAbstractItemView::NoEditTriggers );
    _dateView->setEditTriggers( QAbstractItemView::NoEditTriggers );
    _dateView->setExpandsOnDoubleClick( false );

    // expand the current year and month
    QModelIndex startIdx = mDateModel->index(0,DocBaseModel::Treestruct_Year, QModelIndex());
    QModelIndexList yearIndexes = mDateModel->match(startIdx, Qt::DisplayRole,
                                          QVariant(QDate::currentDate().year()) );

    if( yearIndexes.size() > 0 ) {
        QModelIndex yearIndx = mDateModel->index(yearIndexes.first().row(), 0, yearIndexes.first().parent());
        _dateView->setExpanded(yearIndx,true);

        QModelIndex startIdxM = mDateModel->index(0, DocBaseModel::Treestruct_Month, yearIndx);
        QModelIndexList monthIndexes = mDateModel->match(startIdxM, Qt::DisplayRole,
                                                         QVariant(QDate::currentDate().month()) );
        if( monthIndexes.size() > 0 ) {
            QModelIndex mIdx = monthIndexes.first();
            QModelIndex rIdx = mDateModel->index(mIdx.row(), 0, mIdx.parent());
            _dateView->setExpanded(rIdx, true);
        }
    }
}

void AllDocsView::slotUpdateView()
{
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
    static_cast<DocBaseModel*>(mDateModel->sourceModel())->resetData();
    static_cast<DocBaseModel*>(mTableModel->sourceModel())->resetData();
    mCurrentlySelected = QModelIndex();
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

ArchDocDigest AllDocsView::currentLatestArchivedDoc() const
{
    return mLatestArchivedDigest;
}

void AllDocsView::slotDocOpenRequest( QModelIndex index )
{
    Q_UNUSED(index)
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
        bool isDateModel= false;

        if( _stack->currentIndex() == 0 ) {
            mCurrentlySelected = mTableModel->mapToSource(index);
            model = static_cast<DocumentModel*>( mTableModel->sourceModel() );
        } else {
            isDateModel = true;
            mCurrentlySelected = mDateModel->mapToSource(index);
            model = static_cast<DateModel*>( mDateModel->sourceModel() );
            isDoc = model->isDocument(mCurrentlySelected);
        }

        /* get the corresponding document id */
        if( isDoc ) {
            DocDigest digest;
            QModelIndex idIndx = model->index(mCurrentlySelected.row(),
                                              DocumentModel::Document_Ident,
                                              mCurrentlySelected.parent());

            const QString id = idIndx.data( Qt::DisplayRole ).toString();

            emit docSelected( id );
            digest = model->digest( mCurrentlySelected );
            mAllViewDetails->slotShowDocDetails( digest );
            if( digest.archDocDigestList().size() > 0 ) {
                mLatestArchivedDigest = digest.archDocDigestList()[0];
            } else {
                mLatestArchivedDigest = ArchDocDigest();
            }
        } else {
            const QModelIndex idIndx = model->index(mCurrentlySelected.row(),
                                                    DocumentModel::Treestruct_Type,
                                                    mCurrentlySelected.parent());

            int type = idIndx.data( Qt::DisplayRole ).toInt();
            if( isDateModel) {
                const QModelIndex yIndx = model->index(mCurrentlySelected.row(),
                                                       DocumentModel::Treestruct_Year,
                                                       mCurrentlySelected.parent());
                int year = yIndx.data( Qt::DisplayRole ).toInt();

                if( type == AbstractIndx::MonthType ) {
                    const QModelIndex mIndx = model->index(mCurrentlySelected.row(),
                                                     DocumentModel::Treestruct_Month,
                                                     mCurrentlySelected.parent());
                    int month = mIndx.data( Qt::DisplayRole ).toInt();

                    mAllViewDetails->slotShowMonthDetails(year, month);
                } else if( type == AbstractIndx::YearType ) {
                    mAllViewDetails->slotShowYearDetails(year);
                }
            }
        }
    } else {
        // qDebug () << "Got invalid index, clearing digest view.";
        emit docSelected( QString() );
        mAllViewDetails->slotClearView();
    }
    //// qDebug () << "Supposed row: " << sourceIndex.row() << " Supposed ID: " << DocumentModel::self()->data(sourceIndex, Qt::DisplayRole);
}

QVector<QMenu*> AllDocsView::contextMenus()
{
  QVector<QMenu*> menus;
  menus.append( mAllMenu);

  return menus;
}

