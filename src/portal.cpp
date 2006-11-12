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
#include <qsqldatabase.h>

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
#include <kcmdlineargs.h>

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
#include "prefsdialog.h"

#define ID_STATUS_MSG 1

Portal::Portal( QWidget*, KCmdLineArgs *args, const char* name)
: KMainWindow(0, name),
  mCmdLineArgs( args )

{
  ///////////////////////////////////////////////////////////////////
  // call inits to invoke all other construction parts
  initStatusBar();
  initActions();
  initView();

  ///////////////////////////////////////////////////////////////////
  // disable actions at startup
  editCut->setEnabled(false);
  editCopy->setEnabled(false);
  editPaste->setEnabled(false);

  // check for database init
  // KraftDB::checkInit();
  setAutoSaveSettings();
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
  viewFlosTemplates = new KToggleAction( i18n("Show Floskel Templates"), 0,0, this,
                                         SLOT(slotShowTemplates()),
                                         actionCollection(), "file_show_templ");
  KStdAction::preferences( this, SLOT( preferences() ), actionCollection() );

  actNewDocument = new KAction(i18n("Create Docume&nt"), "filenew",
                               KStdAccel::shortcut(KStdAccel::New), this,
                               SLOT(slotNewDocument()),
                               actionCollection(), "document_new");

  actPrintDocument = new KAction(i18n("&Print Document"), "printer1",
                                 KStdAccel::shortcut(KStdAccel::Print), this,
                                 SLOT(slotPrintDocument()),
                                 actionCollection(), "document_print");

  actOpenDocument = new KAction(i18n("&Open Document"),  "fileopen",
  KStdAccel::shortcut(KStdAccel::Open), this,
  SLOT( slotOpenDocument() ), actionCollection(), "document_open" );

  fileQuit->setStatusText(i18n("Quits the application"));
  editCut->setStatusText(i18n("Cuts the selected section and puts it to the clipboard"));
  editCopy->setStatusText(i18n("Copies the selected section to the clipboard"));
  editPaste->setStatusText(i18n("Pastes the clipboard contents to actual position"));
  viewStatusBar->setStatusText(i18n("Enables/disables the statusbar"));

  actNewDocument->setStatusText( i18n( "Creates a new Document" ) );
  actPrintDocument->setStatusText( i18n( "Print and archive this Document" ) );
  actOpenDocument->setStatusText( i18n( "Opens the document for editing" ) );

  setStandardToolBarMenuEnabled( true );
  actOpenDocument->setEnabled( false );
  actPrintDocument->setEnabled( false );
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
  /*
    Since we do the database version check in the slotStartupChecks, we can not
    do database interaction here in initView.
  */
    ////////////////////////////////////////////////////////////////////
    // create the main widget here that is managed by KTMainWindow's view-region and
    // connect the widget to your document to display document contents.
    m_portalView = new PortalView( this, "mainview", KJanusWidget::IconList );

    actNewDocument->plug( m_portalView->docDigestView()->contextMenu() );
    actPrintDocument->plug( m_portalView->docDigestView()->contextMenu() );
    actOpenDocument->plug( m_portalView->docDigestView()->contextMenu() );

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
    connect( m_portalView,  SIGNAL( documentSelected( const QString& ) ),
             this,  SLOT( slotDocumentSelected( const QString& ) ) );
    setCentralWidget(m_portalView);
}

void Portal::slotStartupChecks()
{
  const QString dbName = KraftDB::self()->databaseName();
  if ( dbName.isEmpty() ) {
    // Problem: Database name is not set in the config.
    PrefsDialog dia( this );
    if ( ! dia.exec() ) {
      return;
    }
  }

  connect( KraftDB::self(),  SIGNAL( statusMessage( const QString& ) ),
           SLOT( slotStatusMsg( const QString& ) ) );

  if( ! KraftDB::self()->isOk() ) {
    QSqlError err = KraftDB::self()->lastError();
    kdDebug() << "The last sql error id: " << err.type() << endl;

    QString text;

    if ( err.text().contains( "Can't connect to local MySQL server through socket" ) ) {
      text = i18n( "Kraft can not connect to the specified MySQL server. "
                   "Please check the Kraft database settings, check if the server is "
                   "running and verify if a database with the name %1 exits!" ).arg( dbName );
    } else if ( err.text().contains( "Unknown database '" + dbName + "' QMYSQL3: Unable to connect" ) ) {
      text = i18n( "The database with the name %1 does not exist on the database server. "
                   "Please make sure the database exists and is accessible by the user "
                   "running Kraft." ).arg( dbName );
    } else {
      text = i18n( "There is a database problem: %1" ).arg( err.text() );
    }


    KMessageBox::sorry( this, text, i18n("Serious Database Problem") );
    slotStatusMsg( i18n( "Database Problem." ) );
  } else {
    KraftDB::self()->checkSchemaVersion( this );

    // Database interaction after this point.
    m_portalView->slotBuildView();
    m_portalView->fillCatalogDetails();
    m_portalView->fillSystemDetails();

    slotStatusMsg( i18n( "Commandline actions" ) );
    QCString docId = mCmdLineArgs->getOption( "d" ); //  <documentId>" );
    if ( ! docId.isEmpty() ) {
      kdDebug() << "open a document: " << docId << endl;
      slotPrintDocument( dbID( docId.toInt() ) );
    }

    mCmdLineArgs->clear();

    slotStatusMsg( i18n( "Ready." ) );
  }
}

bool Portal::queryClose()
{
  return true;
}

bool Portal::queryExit()
{
  return true;
}

/////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATION
/////////////////////////////////////////////////////////////////////
void Portal::busyCursor( bool on )
{
  if ( on ) {
    QApplication::setOverrideCursor( QCursor( BusyCursor ) );
  } else {
    QApplication::restoreOverrideCursor();
  }
}

void Portal::slotNewDocument()
{
  slotStatusMsg(i18n("Creating new document..."));
  busyCursor( true );
  DocumentMan *docman = DocumentMan::self();
  DocGuardedPtr doc = docman->createDocument();
  busyCursor( false );

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

  busyCursor( true );
  slotStatusMsg( i18n( "Generating PDF..." ) );
  DocumentMan *docman = DocumentMan::self();
  DocGuardedPtr docPtr = docman->openDocument( locId );
  if( docPtr ) {
    ArchiveMan *archman = ArchiveMan::self();
    dbID archID = archman->archiveDocument( docPtr );
    slotPrintDocument( archID );
  }
  busyCursor( false );
  slotStatusMsg( i18n( "Ready." ) );

}

void Portal::slotPrintDocument( const dbID& archID )
{
  if ( archID.isOk() ) {
    mReportGenerator = ReportGenerator::self();
    mReportGenerator->createRmlFromArchive( archID ); // work on document identifier.
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
    actPrintDocument->setEnabled( false );
  } else {
    actOpenDocument->setEnabled( true );
    actPrintDocument->setEnabled( true );
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
    connect( view, SIGNAL( viewClosed( bool ) ),
             this, SLOT( slotViewClosed( bool ) ) );
  } else {
    // pop first view to front
    kdDebug() << "There is already a view for this doc!" << endl;
  }
}

void Portal::slotViewClosed( bool )
{
  kdDebug() << "A view was closed" << endl;
  m_portalView->slotBuildView();
}

void Portal::slotFileQuit()
{
  slotStatusMsg(i18n("Exiting..."));
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
        MatEditor me("Material Allgemein", false, this);
        QApplication::restoreOverrideCursor();

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

    Katalog *kat = KatalogMan::self()->getKatalog(katName);

    if(kat) {
        kat->writeXMLFile();
    }
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
    if ( dlg.exec() ) {
    }
}

QWidget* Portal::mainWidget()
{
  if( m_portalView )
    return m_portalView->pageWidget(0);
  return 0;
}

#include "portal.moc"
