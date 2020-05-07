/***************************************************************************
                          katalogview.cpp
                             -------------------
    begin                : 2005
    copyright            : (C) 2005 by Klaas Freitag
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
#include <stdlib.h>
// include files for QT
#include <QtGui>
#include <QMenu>
#include <QProgressBar>
#include <QDebug>
#include <QStatusBar>
#include <QMenuBar>

// application specific includes
#include "katalogview.h"
#include "katalog.h"
#include "floskeltemplate.h"
#include "kataloglistview.h"
#include "flostempldialog.h"
#include "templkatalog.h"
#include "filterheader.h"
#include "docposition.h"
#include "katalogman.h"
#include "defaultprovider.h"
#include "format.h"
#include "kraftsettings.h"

#define ID_STATUS_MSG 1

KatalogView::KatalogView( QWidget* parent, const char* ) :
    QMainWindow(parent, nullptr),
    m_acEditChapter(nullptr),
    m_acEditItem(nullptr),
    m_acNewItem(nullptr),
    m_acDeleteItem(nullptr),
    m_acExport(nullptr),
    m_filterHead(nullptr),
    m_editListViewItem(nullptr),
    mTemplateText(nullptr),
    mTemplateStats(nullptr)
{
  setObjectName( "catalogeview" );
  //We don't want to delete this view when we close it!
  setAttribute(Qt::WA_DeleteOnClose, false);
}

void KatalogView::init(const QString& katName )
{
  m_katalogName = katName;
  initActions();

  ///////////////////////////////////////////////////////////////////
  // set up a vertical layout box
  QWidget *w = new QWidget(this);
  QBoxLayout *box = new QVBoxLayout(w);

  // start to set up the listview
  createCentralWidget(box, w);
  KatalogListView *listview = getListView();

  if( ! listview ) {
      // qDebug () << "ERROR: No listview created !!!" << endl;
  } else {
      QHBoxLayout *horizLay = new QHBoxLayout;
      m_filterHead = new FilterHeader(w, listview);
      horizLay->insertWidget(0, m_filterHead, 2);
      horizLay->addStretch(1);
      box->insertLayout(0, horizLay);

      connect( listview, SIGNAL(currentItemChanged ( QTreeWidgetItem*, QTreeWidgetItem*)),
               this, SLOT(slTreeviewItemChanged( QTreeWidgetItem*, QTreeWidgetItem*)) );
      connect( listview, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
               this, SLOT(slEditTemplate()));
      connect( listview, SIGNAL(templateHoovered(CatalogTemplate*)),
               this, SLOT(slotShowTemplateDetails( CatalogTemplate*)));

      // Populate the context Menu
      (listview->contextMenu())->addAction( m_acEditItem );
      (listview->contextMenu())->addAction( m_acNewItem );
      (listview->contextMenu())->addAction( m_acDeleteItem );
      (listview->contextMenu())->addSeparator();
      (listview->contextMenu())->addAction( m_acAddChapter );
      (listview->contextMenu())->addAction( m_acEditChapter );
      (listview->contextMenu())->addAction( m_acRemChapter );
      getKatalog( katName );
      listview->addCatalogDisplay( katName );
  }

  setCentralWidget(w);
  m_editListViewItem = nullptr;
  // qDebug () << "Getting katalog!" << katName << endl;

  // setAutoSaveSettings( QString::fromLatin1( "CatalogWindow" ),  true );
}

void KatalogView::createCentralWidget(QBoxLayout *box, QWidget* )
{

  mTemplateText = new QLabel( "Nothing selected.");
  box->addWidget( mTemplateText );
  QHBoxLayout *hb = new QHBoxLayout;
  box->addLayout( hb );
  mTemplateStats = new QLabel( );
  mProgress = new QProgressBar;
  hb->addWidget( mTemplateStats );
  hb->addStretch();
  hb->addWidget( mProgress );

  connect( getListView(), SIGNAL( sequenceUpdateMaximum( int )),
           mProgress, SLOT( setMaximum(int) ) );
  connect( getListView(), SIGNAL( sequenceUpdateProgress( int ) ),
           this, SLOT( setProgressValue(int) ) );

  const QByteArray state = windowState();
  restoreState(state);
  const QByteArray geo = windowGeo();
  restoreGeometry(geo);
}

void KatalogView::setProgressValue( int val )
{
  if( ! mProgress ) return;
  mProgress->setValue( val );
  if( val == mProgress->maximum() ) {
    QTimer::singleShot( 3000, mProgress, SLOT(reset()));
  }
}

KatalogView::~KatalogView()
{

}

Katalog* KatalogView::getKatalog( const QString& name )
{

  KatalogMan::self()->registerKatalogListView( name, getListView() );

  return nullptr;
}

void KatalogView::initActions()
{  
    QIcon newIcon = QIcon::fromTheme( "folder-documents");
     m_acEditChapter = new QAction(newIcon, i18n("Edit Sub chapter"), this);
     m_acEditChapter->setShortcut( Qt::CTRL + Qt::Key_S);
     m_acEditChapter->setStatusTip(i18n("Edit a catalog sub chapter"));
     connect(m_acEditChapter, &QAction::triggered, this, &KatalogView::slEditSubChapter);

     newIcon = QIcon::fromTheme( "document-edit");
      m_acAddChapter = new QAction(newIcon, i18n("Add a sub chapter"), this);
      m_acAddChapter->setShortcut( Qt::CTRL + Qt::Key_A);
      m_acAddChapter->setStatusTip(i18n("Add a sub chapter below the selected one"));
      connect(m_acAddChapter, &QAction::triggered, this, &KatalogView::slAddSubChapter);

      newIcon = QIcon::fromTheme( "document-edit");
       m_acRemChapter = new QAction(newIcon, i18n("Remove a sub chapter"), this);
       m_acRemChapter->setShortcut( Qt::CTRL + Qt::Key_R);
       m_acRemChapter->setStatusTip(i18n("Remove a sub chapter"));
       connect(m_acRemChapter, &QAction::triggered, this, &KatalogView::slRemoveSubChapter);

       newIcon = QIcon::fromTheme( "document-edit");
        m_acEditItem = new QAction(newIcon, i18n("Edit Template"), this);
        m_acEditItem->setShortcut( Qt::CTRL + Qt::Key_T);
        m_acEditItem->setStatusTip(i18n("Opens the editor window for templates to edit the selected one"));
        m_acEditItem->setEnabled(false);
        connect(m_acEditItem, &QAction::triggered, this, &KatalogView::slEditTemplate);

        newIcon = QIcon::fromTheme( "document-new");
        m_acNewItem = new QAction(newIcon, i18n("New template"), this);
        m_acNewItem->setShortcut( Qt::CTRL + Qt::Key_N);
        m_acNewItem->setStatusTip(i18n("Opens the editor window for templates to enter a new template"));
        connect(m_acNewItem, &QAction::triggered, this, &KatalogView::slNewTemplate);
        m_acNewItem->setEnabled(true);

        newIcon = QIcon::fromTheme( "document-delete");
        m_acDeleteItem = new QAction(newIcon, i18n("Delete template"), this);
        m_acDeleteItem->setShortcut( QKeySequence::Delete);
        m_acDeleteItem->setStatusTip(i18n("Deletes the template"));
        connect(m_acDeleteItem, &QAction::triggered, this, &KatalogView::slDeleteTemplate);
        m_acDeleteItem->setEnabled(true);

        newIcon = QIcon(); // QIcon::fromTheme( "document-delete");
        m_acExport = new QAction(newIcon, i18n("Export catalog"), this);
        m_acExport->setShortcut( Qt::Key_E);
        m_acExport->setStatusTip(i18n("Export the whole catalog as XML encoded file"));
        connect(m_acExport, &QAction::triggered, this, &KatalogView::slExport);
        m_acExport->setEnabled(false);

        newIcon = QIcon(); // QIcon::fromTheme( "document-delete");
        m_acImport = new QAction(newIcon, i18n("Import catalog"), this);
        m_acExport->setShortcut( Qt::Key_I);
        m_acExport->setStatusTip(i18n("Import a catalog from a XML file"));
        connect(m_acExport, &QAction::triggered, this, &KatalogView::slImport);
        m_acExport->setEnabled(false);

        QMenu *catalogMenu = menuBar()->addMenu(i18n("&Catalog"));
        catalogMenu->addAction(m_acAddChapter);
        catalogMenu->addAction(m_acEditChapter);
        catalogMenu->addAction(m_acRemChapter);
        catalogMenu->addSeparator();
        catalogMenu->addAction(m_acNewItem);
        catalogMenu->addAction(m_acEditItem);
        catalogMenu->addAction(m_acDeleteItem);
#ifdef HAVE_EXPORT
        catalogMenu->addSeparator();
        catalogMenu->addAction(m_acExport);
        catalogMenu->addAction(m_acImport);
#endif

}

void KatalogView::openDocumentFile(const QUrl& )
{
  slotStatusMsg(i18n("Opening file..."));

  slotStatusMsg(i18n("Ready."));
}

void KatalogView::closeEvent( QCloseEvent *event )
{
    slotStatusMsg(i18n("Exiting..."));

    if( event )
        event->accept();
}

void KatalogView::slotSaveState()
{
    getListView()->saveState(); // saves the header state

    const QByteArray state = saveState().toBase64();
    saveWindowState(state);
    const QByteArray geo = saveGeometry().toBase64();
    saveWindowGeo(geo);
}

void KatalogView::slotStatusMsg(const QString &text)
{
    if( text.isEmpty() ) {
        statusBar()->clearMessage();
    } else {
        statusBar()->showMessage(text, 30*1000 /* milliseconds timeout */ );
    }
}

void KatalogView::slTreeviewItemChanged( QTreeWidgetItem *newItem, QTreeWidgetItem * /* prevItem */ )
{
  KatalogListView *listview = getListView();
  if( !listview ) return;
  if( ! newItem ) return;

  bool itemEdit = true;
  bool itemNew = true;
  bool chapterNew = false;
  bool chapterEdit = false;

  if( listview->isRoot(newItem) ) {
    // we have the root item, not editable
    itemEdit = false;
    itemNew  = false;
    chapterNew = true;
  } else if( listview->isChapter(newItem) ) {
    itemEdit = false;
    chapterNew = true;
    chapterEdit = true;
  }
  m_acEditItem->setEnabled(itemEdit);
  m_acDeleteItem->setEnabled(itemEdit);
  m_acNewItem->setEnabled( itemNew );
  m_acAddChapter->setEnabled( chapterNew );
  m_acEditChapter->setEnabled( chapterEdit );
  m_acRemChapter->setEnabled( chapterEdit );

}

void KatalogView::slExport()
{
    slotStatusMsg(i18n("Exporting file..."));
    Katalog *k = getKatalog(m_katalogName);
    if(k)
        k->writeXMLFile();
    slotStatusMsg(i18n("Ready."));
}

void KatalogView::slImport()
{
    slotStatusMsg(i18n("Importfile... (not yet implemented)"));
}

void KatalogView::slAddSubChapter()
{
  slotStatusMsg( i18n("Creating a new sub chapter..."));
  KatalogListView *listview = getListView();
  if( listview )
    listview->slotCreateNewChapter();
  slotStatusMsg( i18n("Ready."));
}

void KatalogView::slEditSubChapter()
{
  slotStatusMsg( i18n("Editing a sub chapter..."));
  KatalogListView *listview = getListView();
  if( listview )
    listview->slotEditCurrentChapter();
  slotStatusMsg( i18n("Ready."));
}

void KatalogView::slRemoveSubChapter()
{
  slotStatusMsg( i18n("Removing a sub chapter..."));
  KatalogListView *listview = getListView();
  if( listview )
    listview->slotRemoveCurrentChapter();
  slotStatusMsg( i18n("Ready."));

}

void KatalogView::slotShowTemplateDetails( CatalogTemplate *tmpl )
{
  if( ! (mTemplateText && mTemplateStats) ) {
    // qDebug () << "Hoover-Text: No label ready.";
    return;
  }

  if( ! tmpl ) {
    mTemplateText->setText( QString() );
    mTemplateStats->setText( QString() );
    return;
  }

  QString t;
  QString flos = tmpl->getText();
  QFontMetrics fm( mTemplateText->font() );
  int w = mTemplateText->width() - 30;

  t = QString( "<em>%1</em>").arg( fm.elidedText(flos, Qt::ElideMiddle, w ) );
  mTemplateText->setText( t );

  int useCount = tmpl->useCounter();

  t = "<table border=\"0\">";
  t += i18n("<tr><td>Created at:</td><td>%1</td>")
          .arg( Format::toDateTimeString(tmpl->enterDate(), KraftSettings::self()-> dateFormat()) );
  if (useCount > 0) {
      t += i18n("<td>&nbsp;&nbsp;Last used:</td><td>%1</td>" )
          .arg( Format::toDateTimeString(tmpl->lastUsedDate(), KraftSettings::self()-> dateFormat()));
  }
  t += QStringLiteral("</tr>");

  const QDateTime dt = tmpl->modifyDate();
  if (dt.isValid()) {
      t += i18n("<tr><td>Modified at:</td><td>%1</td>")
              .arg( Format::toDateTimeString( dt, KraftSettings::self()-> dateFormat()) );
      if (useCount > 0) {
          t += i18n("<td>&nbsp;&nbsp;Use Count:</td><td>%1</td>" )
              .arg(useCount);
      }
      t += QStringLiteral("</tr>");
  }
  t += "</table>";
  // qDebug() << "Hoover-String: " << t;
  mTemplateStats->setText( t );
}
