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
#include <qdir.h>
#include <qprinter.h>
#include <qpainter.h>

// include files for KDE
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kmenubar.h>
#include <kmenu.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <kdebug.h>

#include <kapplication.h>
#include <kshortcut.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kmenubar.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstandardaction.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>
#include <kactionmenu.h>
#include <kglobal.h>
#include <kxmlguifactory.h>
#include <KXmlGuiWindow>

// application specific includes
#include "katalogview.h"
#include "katalog.h"
#include "floskeltemplate.h"
#include "kataloglistview.h"
#include "flostempldialog.h"
#include "templkatalog.h"
#include "filterheader.h"
#include "catalogchapteredit.h"
#include "docposition.h"
#include "katalogman.h"

#define ID_STATUS_MSG 1

KatalogView::KatalogView( QWidget* parent, const char* ) :
  KXmlGuiWindow(parent, 0),
    m_acEditChapters(0),
    m_acEditItem(0),
    m_acNewItem(0),
    m_acExport(0),
    m_filterHead(0)
{
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
      kDebug() << "ERROR: No listview created !!!" << endl;
  } else {
      m_filterHead = new FilterHeader(listview, w);
      m_filterHead->showCount(false);
      box->insertWidget(0, m_filterHead);

      connect( listview, SIGNAL(currentItemChanged ( QTreeWidgetItem*, QTreeWidgetItem*)),
               this, SLOT(slTreeviewItemChanged( QTreeWidgetItem*, QTreeWidgetItem*)) );
               // this, SLOT(slListviewExecuted(Q3ListViewItem*)));
  }

  setCentralWidget(w);
  m_editListViewItem = 0;
  kDebug() << "Gettign katalog!" << katName << endl;
  getKatalog( katName );
  listview->addCatalogDisplay( katName );

  kDebug() << "Listviews context-menu: " << m_acEditChapters << endl;
  KatalogListView *lv = getListView();

  // Populate the context Menu
  (lv->contextMenu())->addAction( m_acEditItem );
  (lv->contextMenu())->addAction( m_acNewItem );
  (lv->contextMenu())->addAction( m_acEditChapters );

  // m_acEditItem->plug( lv->contextMenu() );
  // m_acNewItem->plug( lv->contextMenu() );
  // m_acEditChapters->plug( lv->contextMenu() );

  setAutoSaveSettings( QString::fromLatin1( "CatalogWindow" ),  true );
}

void KatalogView::createCentralWidget(QBoxLayout*, QWidget*)
{
    kDebug() << "I was called!" << endl;
}

KatalogView::~KatalogView()
{

}

Katalog* KatalogView::getKatalog( const QString& name )
{

  KatalogMan::self()->registerKatalogListView( name, getListView() );

  return 0;
}

void KatalogView::initActions()
{
#if 0
  m_acEditChapters = new KAction( i18n("Edit &Catalog Chapters..."), this );
  connect( m_acEditChapters, SIGNAL(triggered()), this, SLOT(slEditChapters()));

  m_acNewItem = new KAction( i18n("&New Template"), this );
  connect( m_acNewItem, SIGNAL(triggered()), this, SLOT(slNeueVorlage()));

  m_acEditItem = new KAction( i18n("&Edit Item"), this );
  connect( m_acEditItem, SIGNAL(triggered()), this, SLOT(slEditVorlage()));

  m_acExport = new KAction( i18n("&Export Catalog"), this );
  connect( m_acEditItem, SIGNAL(triggered()), this, SLOT(slExport()));
#endif

  m_acEditChapters = actionCollection()->addAction( "edit_chapters", this, SLOT( slEditChapters() ) );
  //
  m_acEditItem = actionCollection()->addAction( "edit_vorlage", this, SLOT( slEditVorlage() ) );
  m_acNewItem = actionCollection()->addAction( "neue_vorlage", this, SLOT( slNeueVorlage() ) );
  m_acExport = actionCollection()->addAction( "export_catalog", this, SLOT( slExport() ) );

  m_acFileClose = actionCollection()->addAction( KStandardAction::Close, this, SLOT( slotFileClose() ) );
  m_acFilePrint = actionCollection()->addAction( KStandardAction::Print, this, SLOT( slotFilePrint() ) );
  m_acEditCut = actionCollection()->addAction( KStandardAction::Cut, this, SLOT( slotEditCut() ) );
  m_acEditCopy = actionCollection()->addAction( KStandardAction::Copy, this, SLOT( slotEditCopy() ) );
  m_acEditPaste = actionCollection()->addAction( KStandardAction::Paste, this, SLOT( slotEditPaste() ) );

#if 0
  m_acEditChapters = new KAction(i18n("Edit &Catalog Chapters..."),
                                   "contents", 0, this,
                                    SLOT(slEditChapters()),  actionCollection(), "edit_chapters" );

  m_acEditItem = new KAction(i18n("&Edit Item"), "pencil", 0, this,
                             SLOT(slEditVorlage()),  actionCollection(), "edit_vorlage" );

  m_acNewItem  = new KAction( i18n("&New Template"), "filenew", 0, this,
                                SLOT(slNeueVorlage()), actionCollection(), "neue_vorlage");

  m_acExport = new KAction( i18n("&Export Catalog..."), "save", 0, this,
                            SLOT(slExport()), actionCollection(), "export_catalog");

#endif

  m_acNewItem->setStatusTip(i18n("Opens the editor window for templates to enter a new template"));
  m_acEditItem->setStatusTip(i18n("Opens the editor window for templates to edit the selected one"));
  m_acEditChapters->setStatusTip(i18n("Add, remove and edit catalog chapters"));
  m_acNewItem->setEnabled(true);   // can always add new items
  m_acEditItem->setEnabled(false);
  m_acEditChapters->setEnabled(true);


  m_acExport->setStatusTip(i18n("Export the whole catalog as XML encoded file"));
  m_acExport->setEnabled(false); // FIXME: Repair XML Export

#if 0
  m_acFileClose = KStandardAction::close(this, SLOT(slotFileClose()), actionCollection());
  filePrint = KStandardAction::print(this, SLOT(slotFilePrint()), actionCollection());
  editCut = KStandardAction::cut(    this, SLOT(slotEditCut()),   actionCollection());
  editCopy = KStandardAction::copy(  this, SLOT(slotEditCopy()),  actionCollection());
  editPaste = KStandardAction::paste(this, SLOT(slotEditPaste()), actionCollection());
#endif

  m_acFileClose->setStatusTip( i18n("Closes the actual document"));
  m_acFilePrint ->setStatusTip( i18n("Prints out the actual document"));

  m_acEditCut->setStatusTip(i18n("Cuts the selected section and puts it to the clipboard"));
  m_acEditCopy->setStatusTip(i18n("Copies the selected section to the clipboard"));
  m_acEditPaste->setStatusTip(i18n("Pastes the clipboard contents to actual position"));

  // createStandardStatusBarAction();
  // setStandardToolBarMenuEnabled( true );

  ///////////////////////////////////////////////////////////////////
  // disable actions at startup
  m_acFilePrint->setEnabled(false);
  m_acEditCut->setEnabled(false);
  m_acEditCopy->setEnabled(false);
  m_acEditPaste->setEnabled(false);
  // use the absolute path to your kraftui.rc file for testing purpose in createGUI();
  char *prjPath = getenv( "KRAFT_HOME" );
  if( prjPath ) {
      createGUI(QString(prjPath)+"/src/katalogview.rc");
  } else {
      createGUI("katalogview.rc");
  }

}

void KatalogView::openDocumentFile(const KUrl& )
{
  slotStatusMsg(i18n("Opening file..."));

  slotStatusMsg(i18n("Ready."));
}

bool KatalogView::queryClose()
{
    return true;
}

bool KatalogView::queryExit()
{
  return true;
}

/////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATION
/////////////////////////////////////////////////////////////////////

void KatalogView::slotFileNewWindow()
{
  slotStatusMsg(i18n("Opening a new katalog window..."));

  KatalogView *new_window= new KatalogView();
  new_window->show();

  slotStatusMsg(i18n("Ready."));
}


void KatalogView::slotFileOpen()
{
  slotStatusMsg(i18n("Opening file..."));

  KUrl url=KFileDialog::getOpenUrl( KUrl(),
                                   i18n("*|All files"), this, i18n("Open File..."));
  if(!url.isEmpty())
  {
      // doc->openDocument(url);
      setCaption(url.fileName(), false);
    }
  slotStatusMsg(i18n("Ready."));
}


void KatalogView::slotFileSave()
{
  slotStatusMsg(i18n("Saving file..."));

  // doc->saveDocument(doc->URL());

  slotStatusMsg(i18n("Ready."));
}


void KatalogView::slotFileClose()
{
  slotStatusMsg(i18n("Closing file..."));

  close();

  slotStatusMsg(i18n("Ready."));
}

void KatalogView::slotFilePrint()
{
  slotStatusMsg(i18n("Printing..."));

#if 0
  QPrinter printer;
  if (printer.setup(this))
  {
  }
#endif
  slotStatusMsg(i18n("Ready."));
}


void KatalogView::slotEditCut()
{
  slotStatusMsg(i18n("Cutting selection..."));

  slotStatusMsg(i18n("Ready."));
}

void KatalogView::slotEditCopy()
{
  slotStatusMsg(i18n("Copying selection to clipboard..."));

  slotStatusMsg(i18n("Ready."));
}

void KatalogView::slotEditPaste()
{
  slotStatusMsg(i18n("Inserting clipboard contents..."));

  slotStatusMsg(i18n("Ready."));
}

void KatalogView::slotStatusMsg(const QString &text)
{
  ///////////////////////////////////////////////////////////////////
  // change status message permanently
  statusBar()->clearMessage();
  statusBar()->changeItem(text, ID_STATUS_MSG);
}

void KatalogView::slTreeviewItemChanged( QTreeWidgetItem *newItem, QTreeWidgetItem * /* prevItem */ )
{
  KatalogListView *listview = getListView();
  if( !listview ) return;
  if( ! newItem ) return;

  bool itemEdit = true;
  bool itemNew = true;

  if( listview->isRoot(newItem) ) {
    // we have the root item, not editable
    itemEdit = false;
    itemNew = false;
  } else if( listview->isChapter(newItem) ) {
    itemEdit = false;
  }
  m_acEditItem->setEnabled(itemEdit);
  m_acNewItem->setEnabled( itemNew );
}

void KatalogView::slExport()
{
    slotStatusMsg(i18n("Exporting file..."));
    Katalog *k = getKatalog(m_katalogName);
    if(k)
        k->writeXMLFile();
    slotStatusMsg(i18n("Ready."));
}

void KatalogView::slEditChapters()
{
    CatalogChapterEditDialog d( this, m_katalogName );

    d.exec();
    if( d.dirty() ) {
      // have to update the catalog view.
      KatalogListView *listview = getListView();
      // listview->setupChapters();
      listview->addCatalogDisplay( m_katalogName );
    } else {
      kDebug() << "We're not dirty!" << endl;
    }
}

#include "katalogview.moc"
