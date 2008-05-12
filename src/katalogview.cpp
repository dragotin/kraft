
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
#include <qlayout.h>

// include files for KDE
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstdaction.h>
#include <kactionclasses.h>
#include <kdebug.h>

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

KatalogView::KatalogView( QWidget* parent, const char* name) :
  KMainWindow(parent, name, 0),
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
      kdDebug() << "ERROR: No listview created !!!" << endl;
  } else {
      m_filterHead = new FilterHeader(listview, w);
      m_filterHead->showCount(false);
      box->insertWidget(0, m_filterHead);

      connect( listview, SIGNAL(selectionChanged(QListViewItem*)),
               this, SLOT(slListviewExecuted(QListViewItem*)));
  }

  setCentralWidget(w);
  m_editListViewItem = 0;
  kdDebug() << "Gettign katalog!" << katName << endl;
  getKatalog( katName );
  listview->addCatalogDisplay( katName );

  kdDebug() << "Listviews context-menu: " << m_acEditChapters << endl;
  KatalogListView *lv = getListView();

  // Populate the context Menu
  m_acEditItem->plug( lv->contextMenu() );
  m_acNewItem->plug( lv->contextMenu() );
  m_acEditChapters->plug( lv->contextMenu() );

  setAutoSaveSettings( QString::fromLatin1( "CatalogWindow" ),  true );
}

void KatalogView::createCentralWidget(QBoxLayout*, QWidget*)
{
    kdDebug() << "I was called!" << endl;
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
  m_acEditChapters = new KAction(i18n("Edit &Catalog Chapters..."),
                                   "contents", 0, this,
                                    SLOT(slEditChapters()),  actionCollection(), "edit_chapters" );

  m_acEditItem = new KAction(i18n("&Edit Item"), "pencil", 0, this,
                             SLOT(slEditVorlage()),  actionCollection(), "edit_vorlage" );

  m_acNewItem  = new KAction( i18n("&New Template"), "filenew", 0, this,
                                SLOT(slNeueVorlage()), actionCollection(), "neue_vorlage");

  m_acNewItem->setStatusText(i18n("Opens the editor window for templates to enter a new template"));
  m_acEditItem->setStatusText(i18n("Opens the editor window for templates to edit the selected one"));
  m_acEditChapters->setStatusText(i18n("Add, remove and edit catalog chapters"));
  m_acNewItem->setEnabled(true);   // can always add new items
  m_acEditItem->setEnabled(false);
  m_acEditChapters->setEnabled(true);

  m_acExport = new KAction( i18n("&Export Catalog..."), "save", 0, this,
                            SLOT(slExport()), actionCollection(), "export_catalog");

  m_acExport->setStatusText(i18n("Export the whole catalog as XML encoded file"));
  m_acExport->setEnabled(true);

  fileClose = KStdAction::close(this, SLOT(slotFileClose()), actionCollection());
  filePrint = KStdAction::print(this, SLOT(slotFilePrint()), actionCollection());
  editCut = KStdAction::cut(    this, SLOT(slotEditCut()),   actionCollection());
  editCopy = KStdAction::copy(  this, SLOT(slotEditCopy()),  actionCollection());
  editPaste = KStdAction::paste(this, SLOT(slotEditPaste()), actionCollection());

  fileClose->setStatusText( i18n("Closes the actual document"));
  filePrint ->setStatusText( i18n("Prints out the actual document"));

  editCut->setStatusText(i18n("Cuts the selected section and puts it to the clipboard"));
  editCopy->setStatusText(i18n("Copies the selected section to the clipboard"));
  editPaste->setStatusText(i18n("Pastes the clipboard contents to actual position"));

  createStandardStatusBarAction();
  setStandardToolBarMenuEnabled( true );

  ///////////////////////////////////////////////////////////////////
  // disable actions at startup
  filePrint->setEnabled(false);
  editCut->setEnabled(false);
  editCopy->setEnabled(false);
  editPaste->setEnabled(false);
  // use the absolute path to your kraftui.rc file for testing purpose in createGUI();
  char *prjPath = getenv( "KRAFT_HOME" );
  if( prjPath ) {
      createGUI(QString(prjPath)+"/src/katalogview.rc");
  } else {
      createGUI("katalogview.rc");
  }

}

void KatalogView::openDocumentFile(const KURL& )
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

  KURL url=KFileDialog::getOpenURL(QString::null,
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

  QPrinter printer;
  if (printer.setup(this))
  {
  }

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
  statusBar()->clear();
  statusBar()->changeItem(text, ID_STATUS_MSG);
}

void KatalogView::slListviewExecuted( QListViewItem *qItem )
{
  KatalogListView *listview = getListView();
  if( !listview ) return;
  if( ! qItem ) return;

  KListViewItem *item = static_cast<KListViewItem*>(qItem);

  bool itemEdit = true;

  if( listview->isRoot(item) ) {
    // we have the root item, not editable
    itemEdit = false;
  } else if( listview->isChapter(item) ) {
    itemEdit = false;
  }
  m_acEditItem->setEnabled(itemEdit);

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
      kdDebug() << "We're not dirty!" << endl;
    }
}

#include "katalogview.moc"
