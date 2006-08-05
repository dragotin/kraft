/***************************************************************************
                   portal.cpp  - The Kraft portal page
                             -------------------
    begin                : Mar 2006
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

// include files for QT
#include <qdir.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qtimer.h>

// include files for KDE
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstdaction.h>
#include <kdebug.h>
#include <kaction.h>

// application specific includes
#include "kraftview.h"
#include "portal.h"
#include "portalview.h"
#include "kraftdb.h"
#include "katalog.h"
#include "katalogman.h"
#include "kraftdoc.h"
#include "floskeltemplate.h"
#include "templkatalogview.h"
#include "mateditor.h"
#include "brunskatalogview.h"
#include "prefsdialog.h"
#include "documentman.h"
#include "docdigestview.h"
#include "archiveman.h"
#include "reportgenerator.h"
#include "kraftsettings.h"

#define ID_STATUS_MSG 1

Portal::Portal( QWidget* , const char* name)
: KMainWindow(0, name)

{
  config=kapp->config();

  ///////////////////////////////////////////////////////////////////
  // call inits to invoke all other construction parts
  initStatusBar();
  initActions();
  initView();

  readOptions();

  ///////////////////////////////////////////////////////////////////
  // disable actions at startup
  editCut->setEnabled(false);
  editCopy->setEnabled(false);
  editPaste->setEnabled(false);

  // check for database init
  // KraftDB::checkInit();
  QTimer::singleShot( 0, this, SLOT( slotStartupChecks() ) );
}

Portal::~Portal()
{

}

void Portal::initActions()
{
  fileQuit = KStdAction::quit(this, SLOT(slotFileQuit()), actionCollection());
  editCut = KStdAction::cut(this, SLOT(slotEditCut()), actionCollection());
  editCopy = KStdAction::copy(this, SLOT(slotEditCopy()), actionCollection());
  editPaste = KStdAction::paste(this, SLOT(slotEditPaste()), actionCollection());
  viewStatusBar = KStdAction::showStatusbar(this, SLOT(slotViewStatusBar()), actionCollection());
  viewFlosTemplates = new KToggleAction( i18n("Show Floskel Templates"), 0,0,this,SLOT(slotShowTemplates()), actionCollection(), "file_show_templ");
  KStdAction::preferences( this, SLOT( preferences() ), actionCollection() );

  actOpenKatalog = new KAction(i18n("Open Catalog Window"), 0,0,this,
                               SLOT(slotOpenKatalog()), actionCollection(), "open_katalog_window");
  actOpenMatKatalog = new KAction(i18n("Open Material Catalog Window"), 0,0,this,
                                  SLOT(slotOpenMaterialKatalog()), actionCollection(), "open_matkat_window");

  actNewDocument = new KAction(i18n("Create Docume&nt"), "filenew", KStdAccel::shortcut(KStdAccel::New), this,
                               SLOT(slotNewDocument()), actionCollection(), "document_new");

  actPrintDocument = new KAction(i18n("&Print Document"), "printer1", KStdAccel::shortcut(KStdAccel::Print), this,
                                 SLOT(slotPrintDocument()), actionCollection(), "document_print");

  actOpenDocument = new KAction(i18n("&Open Document"),  "fileopen",
  KStdAccel::shortcut(KStdAccel::Open), this,
  SLOT( slotOpenDocument() ), actionCollection(), "document_open" );

  fileQuit->setStatusText(i18n("Quits the application"));
  editCut->setStatusText(i18n("Cuts the selected section and puts it to the clipboard"));
  editCopy->setStatusText(i18n("Copies the selected section to the clipboard"));
  editPaste->setStatusText(i18n("Pastes the clipboard contents to actual position"));
  viewStatusBar->setStatusText(i18n("Enables/disables the statusbar"));

  actOpenKatalog->setStatusText(i18n("Opens a new Catalog window"));
  actNewDocument->setStatusText(i18n("Creates a new Document"));
  actPrintDocument->setStatusText( i18n("Print and archive this Document"));

  setStandardToolBarMenuEnabled( true );
  actOpenDocument->setEnabled( false );
  // use the absolute path to your kraftui.rc file for testing purpose in createGUI();
  char *prjPath = getenv("KRAFT_HOME");
  if( prjPath ) {
      createGUI(QString(prjPath)+"/src/kraftui.rc");
  } else {
      createGUI( "kraftui.rc");
  }

}


void Portal::initStatusBar()
{
  ///////////////////////////////////////////////////////////////////
  // STATUSBAR
  // TODO: add your own items you need for displaying current application status.
  statusBar()->insertItem(i18n("Ready."), ID_STATUS_MSG);
}

void Portal::initView()
{
    ////////////////////////////////////////////////////////////////////
    // create the main widget here that is managed by KTMainWindow's view-region and
    // connect the widget to your document to display document contents.
    m_portalView = new PortalView( this, "mainview", KJanusWidget::IconList );
    connect( m_portalView, SIGNAL(openKatalog( const QString&)),
             this, SLOT(slotOpenKatalog(const QString&)));
    connect( m_portalView, SIGNAL(katalogToXML(const QString& )),
             this, SLOT(slotKatalogToXML(const QString&)));

    // document related connections
    connect( m_portalView, SIGNAL( createDocument() ),
             this, SLOT( slotNewDocument() ) );
    connect( m_portalView, SIGNAL( openDocument( const QString& ) ),
             this, SLOT( slotOpenDocument( const QString& ) ) );
    connect( m_portalView, SIGNAL( printDocument( const QString& ) ),
             this, SLOT( slotPrintDocument() ) );

    setCentralWidget(m_portalView);
}

void Portal::slotStartupChecks()
{
  if( ! KraftDB::getDB() ) {
      KMessageBox::sorry( this, i18n("Can not open the database"),
                          i18n("Database Problem") );
  }
}



void Portal::saveOptions()
{
  config->setGroup("General Options");
  config->writeEntry("Geometry", size());

  config->writeEntry("Show Statusbar",viewStatusBar->isChecked());

}


void Portal::readOptions()
{

  config->setGroup("General Options");

  bool bViewStatusbar = config->readBoolEntry("Show Statusbar", true);
  viewStatusBar->setChecked(bViewStatusbar);
  slotViewStatusBar();

  QSize size=config->readSizeEntry("Geometry");
  if(!size.isEmpty())
  {
    resize(size);
  }
}

void Portal::saveProperties(KConfig *)
{

}

void Portal::readProperties(KConfig*)
{

}

bool Portal::queryClose()
{
  // return doc->saveModified();
  return true;
}

bool Portal::queryExit()
{
  saveOptions();
  return true;
}

/////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATION
/////////////////////////////////////////////////////////////////////

void Portal::slotNewDocument()
{
  slotStatusMsg(i18n("Creating new document..."));

  DocumentMan *docman = DocumentMan::self();
  DocGuardedPtr doc = docman->createDocument();

  slotStatusMsg(i18n("Ready."));
  createView( doc );
}

void Portal::slotOpenDocument()
{
  QString locId = m_portalView->docDigestView()->currentDocumentId();
  slotOpenDocument( locId );
}

void Portal::slotPrintDocument()
{
  QString locId = m_portalView->docDigestView()->currentDocumentId();
  kdDebug() << "printing document " << locId << endl;

  DocumentMan *docman = DocumentMan::self();
  DocGuardedPtr docPtr = docman->openDocument( locId );
  if( docPtr ) {
    ArchiveMan *archman = ArchiveMan::self();
    dbID archID = archman->archiveDocument( docPtr );

    mReportGenerator = ReportGenerator::self();
    mReportGenerator->docPreview( archID );
  }
}

void Portal::slotOpenDocument( const QString& id )
{
  slotStatusMsg(i18n("Opening document..."));

  if( !id.isEmpty() ) {
    DocumentMan *docman = DocumentMan::self();
    DocGuardedPtr doc = docman->openDocument( id );
    createView( doc );
  }

  slotStatusMsg(i18n("Ready."));
}

void Portal::slotDocumentSelected( const QString& doc )
{
  kdDebug() << "a doc was selected: " << doc << endl;
  if( doc.isEmpty() ) {
    actOpenDocument->setEnabled( false );
  } else {
    actOpenDocument->setEnabled( true );
  }
}

void Portal::createView( DocGuardedPtr doc )
{
  // FIXME: We allow only one view for the first time.
  // Later allow one write view and other read onlys.
  KraftView *view = doc->firstView();

  if( ! view ) {
    view = new KraftView( this );
    view->setup( doc );
    view->redrawDocument();
    QSize s = KraftSettings::docViewSize();
    if ( !s.isValid() ) {
      s.setWidth( 640 );
      s.setHeight( 400 );
    }
    view->setInitialSize( s );
    view->show();

    doc->addView( view );
  } else {
    // pop first view to front
    kdDebug() << "There is already a view for this doc!" << endl;
  }
}


void Portal::slotFileQuit()
{
  slotStatusMsg(i18n("Exiting..."));
  saveOptions();
  // close the first window, the list makes the next one the first again.
  // This ensures that queryClose() is called on each window to ask for closing
  KMainWindow* w;
  if(memberList)
  {
    for(w=memberList->first(); w!=0; w=memberList->first())
    {
      // only close the window if the closeEvent is accepted. If the user presses Cancel on the saveModified() dialog,
      // the window and the application stay open.
      if(!w->close())
	break;
    }
  }
}

void Portal::slotEditCut()
{
  slotStatusMsg(i18n("Cutting selection..."));

  slotStatusMsg(i18n("Ready."));
}

void Portal::slotEditCopy()
{
  slotStatusMsg(i18n("Copying selection to clipboard..."));

  slotStatusMsg(i18n("Ready."));
}

void Portal::slotEditPaste()
{
  slotStatusMsg(i18n("Inserting clipboard contents..."));

  slotStatusMsg(i18n("Ready."));
}

void Portal::slotViewStatusBar()
{
  slotStatusMsg(i18n("Toggle the statusbar..."));
  ///////////////////////////////////////////////////////////////////
  //turn Statusbar on or off
  if(!viewStatusBar->isChecked())
  {
    statusBar()->hide();
  }
  else
  {
    statusBar()->show();
  }

  slotStatusMsg(i18n("Ready."));
}



void Portal::slotStatusMsg(const QString &text)
{
  ///////////////////////////////////////////////////////////////////
  // change status message permanently
  statusBar()->clear();
  statusBar()->changeItem(text, ID_STATUS_MSG);
}

/** Show the  window with floskeltemplates */
void Portal::slotShowTemplates(){
}

void Portal::slotOpenKatalog(const QString& kat)
{
    kdDebug() << "opening Katalog " << kat << endl;

    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
    // FIXME: Besser Unterscheidung der Kataloge

    if( kat == QString("Material") ) {
        /* Materialkatalog */
        MatEditor me("Material Allgemein", true, this);

        if ( me.exec() == QDialog::Accepted ) {
            kdDebug() << "fine" << endl;
        }
    } else if( kat.startsWith("Bruns") ) {
        // BrunsKatalog *brunskat = new BrunsKatalog();
        // brunskat->load();
        BrunsKatalogView *katView = new BrunsKatalogView();
        katView->init(kat);
        katView->show();
    } else {
        /* normaler Vorlagenkatalog */
        TemplKatalogView *katView = new TemplKatalogView();
        connect( katView, SIGNAL( newDocPosition( const DocPosition& ) ),
                   this, SLOT( slotOfferNewPosition( const DocPosition& ) ) );
        katView->init(kat);
        katView->show();
    }
    QApplication::restoreOverrideCursor();
}

void Portal::slotOfferNewPosition( const DocPosition& pos )
{
  slotStatusMsg( i18n( "Appending catalog entry to document..." ) );
  DocumentMan *docman = DocumentMan::self();
  docman->offerNewPosition( pos );
  slotStatusMsg( i18n("Ready.") );
}

void Portal::slotOpenKatalog()
{
    kdDebug() << "opening katalog!" << endl;
    KatalogView *katView = new TemplKatalogView(); //this);
    katView->show();

}

void Portal::slotKatalogToXML(const QString& katName)
{
    kdDebug() << "Generating XML for catalog " << katName << endl;

    Katalog *kat = KatalogMan::getKatalog(katName);

    if(kat) {
        kat->writeXMLFile();
    }
}
void Portal::slotOpenMaterialKatalog()
{
    kdDebug() << "opening material katalog!" << endl;
}

QString Portal::textWrap( const QString& t, unsigned int width )
{
    QString re;

    if( t.length() < width )
    {
        re = t;
    }
    else
    {
        unsigned int start = 0;
        int pos = width;
        while( pos < (int) t.length() )
        {
            pos = t.find( ' ', start+width );
            if( pos > -1 ) {
                re += t.mid( start, pos-start)+'\n';
                start = pos;
            } else {
                re += t.mid( start );
                pos = t.length();
            }
        }

    }

    return re;
}

void Portal::preferences()
{
    PrefsDialog dlg( this );
    dlg.exec();
}

QWidget* Portal::mainWidget()
{
  if( m_portalView )
    return m_portalView->pageWidget(0);
  return 0;
}

#include "portal.moc"
