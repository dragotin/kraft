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
#include <krun.h>
#include <kapplication.h>
#include <kabc/addressbook.h>
#include <kabc/stdaddressbook.h>
#include <kabc/addressee.h>

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
#include "materialkatalogview.h"
#include "prefsdialog.h"
#include "documentman.h"
#include "docdigestview.h"
#include "archiveman.h"
#include "reportgenerator.h"
#include "kraftsettings.h"
#include "prefsdialog.h"
#include "defaultprovider.h"
#include "archdoc.h"
#include "newdocassistant.h"

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

  actCopyDocument = new KAction( i18n( "Copy Document" ), "editcopy",
                                 KStdAccel::shortcut( KStdAccel::Copy ), this,
                                 SLOT( slotCopyDocument() ),
                                 actionCollection(), "document_copy" );

  actPrintDocument = new KAction(i18n("&Print Document"), "printer1",
                                 KStdAccel::shortcut(KStdAccel::Print), this,
                                 SLOT(slotPrintDocument()),
                                 actionCollection(), "document_print");

  actOpenArchivedDocument = new KAction( i18n( "Open &Archived Document" ), "attach",
                                         KShortcut( Qt::CTRL + Qt::Key_A ), this,
                                         SLOT( slotArchivedDocExecuted() ), actionCollection(),
                                         "archived_open" );

  actOpenDocument = new KAction(i18n("&Open Document"),  "fileopen",
                                KStdAccel::shortcut(KStdAccel::Open), this,
                                SLOT( slotOpenDocument() ), actionCollection(), "document_open" );

  actMailDocument = new KAction( i18n( "&Mail Document" ), "mail_generic",
                                 KShortcut( Qt::CTRL + Qt::Key_M ), this,
                                 SLOT( slotMailDocument() ), actionCollection(), "document_mail" );
#if 0
  KActionMenu *actMenuTexts = new KActionMenu( i18n( "&Header Text" ), actionCollection(), "menu_texts" );
  QStringList docTypes = DefaultProvider::self()->docTypes();

  for ( QStringList::Iterator it = docTypes.begin(); it != docTypes.end(); ++it ) {
    actMenuTexts->insert( new KAction( *it, KShortcut(), actMenuTexts, SLOT( slotEditText() ),
                            actionCollection() ) );
  }
#endif

  fileQuit->setStatusText(i18n("Quits the application"));
  editCut->setStatusText(i18n("Cuts the selected section and puts it to the clipboard"));
  editCopy->setStatusText(i18n("Copies the selected section to the clipboard"));
  editPaste->setStatusText(i18n("Pastes the clipboard contents to actual position"));
  viewStatusBar->setStatusText(i18n("Enables/disables the statusbar"));

  actNewDocument->setStatusText( i18n( "Creates a new Document" ) );
  actPrintDocument->setStatusText( i18n( "Print and archive this Document" ) );
  actOpenDocument->setStatusText( i18n( "Opens the document for editing" ) );
  actMailDocument->setStatusText( i18n( "Send document per mail" ) );

  actOpenArchivedDocument->setStatusText( i18n( "Open a viewer on an archived document" ) );
  setStandardToolBarMenuEnabled( true );
  actOpenDocument->setEnabled( false );
  actPrintDocument->setEnabled( false );
  actMailDocument->setEnabled( false );
  actOpenArchivedDocument->setEnabled( false );
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
    actOpenDocument->plug( m_portalView->docDigestView()->contextMenu() );
    actCopyDocument->plug( m_portalView->docDigestView()->contextMenu() );
    actPrintDocument->plug( m_portalView->docDigestView()->contextMenu() );
    actMailDocument->plug( m_portalView->docDigestView()->contextMenu() );
    actOpenArchivedDocument->plug( m_portalView->docDigestView()->contextMenu() );

    connect( m_portalView, SIGNAL(openKatalog( const QString&)),
             this, SLOT(slotOpenKatalog(const QString&)));
    connect( m_portalView, SIGNAL(katalogToXML(const QString& )),
             this, SLOT(slotKatalogToXML(const QString&)));

    // document related connections
    connect( m_portalView, SIGNAL( createDocument() ),
             this, SLOT( slotNewDocument() ) );
    connect( m_portalView, SIGNAL( copyDocument( const QString& ) ),
             this, SLOT( slotCopyDocument( const QString& ) ) );
    connect( m_portalView, SIGNAL( openDocument( const QString& ) ),
             this, SLOT( slotOpenDocument( const QString& ) ) );
    connect( m_portalView, SIGNAL( openArchivedDocument( const ArchDocDigest& ) ),
             this, SLOT( slotOpenArchivedDoc( const ArchDocDigest& ) ) );
    connect( m_portalView, SIGNAL( printDocument( const QString& ) ),
             this, SLOT( slotPrintDocument() ) );
    connect( m_portalView,  SIGNAL( documentSelected( const QString& ) ),
             this,  SLOT( slotDocumentSelected( const QString& ) ) );
    connect( m_portalView,  SIGNAL( archivedDocSelected( const ArchDocDigest& ) ),
             this,  SLOT( slotArchivedDocSelected( const ArchDocDigest& ) ) );
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
    } else if ( err.text().contains( "Driver not loaded" ) ) {
      text = i18n( "The Qt database driver could not be loaded. That probably means, that "
                   "they are not installed. Please make sure the Qt database packages are "
                   "installed and try again." );
    } else {
      text = i18n( "There is a database problem: %1" ).arg( err.text() );
    }


    // KMessageBox::sorry( this, text, i18n("Serious Database Problem") );
    m_portalView->systemInitError( m_portalView->ptag( text, "problem" ) );

    // disable harmfull actions
    actNewDocument->setEnabled( false );
    actPrintDocument->setEnabled( false );
    actOpenDocument->setEnabled( false );
    actOpenArchivedDocument->setEnabled( false );
    actMailDocument->setEnabled( false );

    slotStatusMsg( i18n( "Database Problem." ) );
  } else {
    KraftDB::self()->checkSchemaVersion( this );

    // Database interaction after this point.
    m_portalView->slotBuildView();
    m_portalView->fillCatalogDetails();
    m_portalView->fillSystemDetails();

    slotStatusMsg( i18n( "Commandline actions" ) );

    if ( mCmdLineArgs ) {
      QCString docId = mCmdLineArgs->getOption( "d" ); //  <documentId>" );
      if ( ! docId.isEmpty() ) {
        kdDebug() << "open a archived document: " << docId << endl;
        slotPrintDocument( QString(), dbID( docId.toInt() ) );
      }

      mCmdLineArgs->clear();
    }

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

  busyCursor( false );

  KraftWizard wiz;
  wiz.init();
  if ( wiz.exec() ) {
    DocumentMan *docman = DocumentMan::self();
    DocGuardedPtr doc = docman->createDocument();

    doc->setDate( wiz.date() );
    doc->setAddressUid( wiz.addressUid() );
    doc->setDocType( wiz.docType() );
    doc->setWhiteboard( wiz.whiteboard() );
    createView( doc );
  }
  slotStatusMsg(i18n("Ready."));
}

void Portal::slotCopyDocument()
{
  const QString locId = m_portalView->docDigestView()->currentDocumentId();
  slotCopyDocument( locId );
}

void Portal::slotCopyDocument( const QString& id )
{
  if ( id.isEmpty() ) {
    return;
  }

  DocGuardedPtr doc = DocumentMan::self()->createDocument( id );
  kdDebug() << "Document created from id " << id << endl;
  // set the current date
  doc->setDate( QDate::currentDate() );
  doc->saveDocument();
  m_portalView->slotDocumentCreated( doc );
  kdDebug() << "Document saved with dbID " << doc->docID().toString() << endl;
}

void Portal::slotOpenDocument()
{
  QString locId = m_portalView->docDigestView()->currentDocumentId();
  slotOpenDocument( locId );
}

void Portal::slotOpenArchivedDoc( const ArchDocDigest& d )
{
  busyCursor( true );
  ArchDocDigest digest( d );

  QString outputDir = ArchiveMan::self()->pdfBaseDir();
  QString filename = ArchiveMan::self()->archiveFileName( digest.archDocIdent(),
                                                          digest.archDocId().toString(), "pdf" );
  QString file = QString( "%1/%2" ).arg( outputDir ).arg( filename );

  kdDebug() << "archived doc selected: " << file << endl;
  slotOpenPdf( file );


  busyCursor( false );
}

void Portal::slotPrintDocument()
{
  QString locId = m_portalView->docDigestView()->currentDocumentId();
  kdDebug() << "printing document " << locId << endl;

  busyCursor( true );
  slotStatusMsg( i18n( "Generating PDF..." ) );
  DocumentMan *docman = DocumentMan::self();
  DocGuardedPtr docPtr = docman->openDocument( locId );
  QString ident;
  if ( docPtr ) ident = docPtr->ident();

  if( docPtr ) {
    ArchiveMan *archman = ArchiveMan::self();
    dbID archID = archman->archiveDocument( docPtr );
    slotPrintDocument( ident, archID );
  }
  busyCursor( false );
  slotStatusMsg( i18n( "Ready." ) );

}

void Portal::slotMailDocument()
{
  QString locId = m_portalView->docDigestView()->currentDocumentId();
  kdDebug() << "Mailing document " << locId << endl;

  busyCursor( true );
  slotStatusMsg( i18n( "Generating PDF..." ) );
  DocumentMan *docman = DocumentMan::self();
  DocGuardedPtr docPtr = docman->openDocument( locId );
  QString ident;
  if ( docPtr ) {
    ident = docPtr->ident();

    KABC::AddressBook *ab = KABC::StdAddressBook::self();
    KABC::Addressee addressee = ab->findByUid( docPtr->addressUid() );

    mMailReceiver = addressee.fullEmail();

    ArchiveMan *archman = ArchiveMan::self();
    dbID archID = archman->archiveDocument( docPtr );

    connect( ReportGenerator::self(), SIGNAL( pdfAvailable( const QString& ) ),
             this, SLOT( slotMailDocument( const QString& ) ) );
    ReportGenerator::self()->createPdfFromArchive( ident, archID );
  }
  busyCursor( false );
  slotStatusMsg( i18n( "Ready." ) );
}

void Portal::slotMailDocument( const QString& fileName )
{
  kdDebug() << "Mailing away " << fileName << endl;

  disconnect( ReportGenerator::self(), SIGNAL( pdfAvailable( const QString& ) ),0,0 );

  // FIXME: the mailed document should go to another directory to be traceable
  // for the time being we rely on the mailer sent mail folder ;-)

  KURL mailTo;
  mailTo.setProtocol( "mailto" );
  if ( ! mMailReceiver.isEmpty() ) {
    mailTo.addQueryItem( "to", mMailReceiver );
  }
  mailTo.addQueryItem( "attach", fileName );

  kdDebug() << "Use this mailto: " << mailTo << endl;

  KApplication::kApplication()->invokeMailer( mailTo, "Kraft", true );
}

/*
 * id    : document ID
 * archID: database ID of archived document
 */
void Portal::slotPrintDocument( const QString& id,  const dbID& archID )
{
  if ( archID.isOk() ) {
    slotStatusMsg(i18n("Printing archived document...") );
    mReportGenerator = ReportGenerator::self();
    connect( mReportGenerator, SIGNAL( pdfAvailable( const QString& ) ),
             this,  SLOT( slotOpenPdf( const QString& ) ) );

    mReportGenerator->createPdfFromArchive( id, archID ); // work on document identifier.
  }
}

void Portal::slotOpenPdf( const QString& fileName )
{
    disconnect( ReportGenerator::self(), SIGNAL( pdfAvailable( const QString& ) ),0,0 );
    KURL url( fileName );
    KRun::runURL( url, "application/pdf" );
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
    actMailDocument->setEnabled( false );
  } else {
    actOpenDocument->setEnabled( true );
    actPrintDocument->setEnabled( true );
    actMailDocument->setEnabled( true );
    actOpenArchivedDocument->setEnabled( false );
  }
}

void Portal::slotArchivedDocExecuted()
{

  ArchDocDigest dig = m_portalView->docDigestView()->currentArchiveDoc();

  slotOpenArchivedDoc( dig );
}

void Portal::slotArchivedDocSelected( const ArchDocDigest& )
{
  // slotDocumentSelected( QString() );
  actOpenArchivedDocument->setEnabled( true );
  actOpenDocument->setEnabled( false );
  actPrintDocument->setEnabled( false );
  actMailDocument->setEnabled( false );
}

void Portal::createView( DocGuardedPtr doc )
{
  // FIXME: We allow only one view for the first time.
  // Later allow one write view and other read onlys.
  if ( !doc ) return;
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
    connect( view, SIGNAL( viewClosed( bool, DocGuardedPtr ) ),
             this, SLOT( slotViewClosed( bool, DocGuardedPtr ) ) );
  } else {
    // pop first view to front
    kdDebug() << "There is already a view for this doc!" << endl;
  }
}

void Portal::slotViewClosed( bool success, DocGuardedPtr doc )
{
  // doc is only valid on success!
  if ( doc && success )  {
    kdDebug() << "A view was closed saving and doc is new: " << doc->isNew() << endl;
    if ( doc->isNew() ) {
      m_portalView->slotDocumentCreated( doc );
    } else {
      m_portalView->slotDocumentUpdate( doc );
    }
  } else {
    kdDebug() << "A view was closed canceled" << endl;
  }
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
      // only close the window if the closeEvent is accepted.
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

    if ( mKatalogViews.contains( kat ) ) {
      // bring up the katalog view window.
      kdDebug() << "Katalog " << kat << " already open in a view" << endl;
      mKatalogViews[kat]->show();
      mKatalogViews[kat]->raise();
    } else {
      QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

      KatalogView *katView = 0;
      if( kat == MaterialKatalogView::MaterialCatalogName ) {
        /* Materialkatalog */
        katView = new MaterialKatalogView();
      } else if( kat.startsWith("Bruns") ) {
        // BrunsKatalog *brunskat = new BrunsKatalog();
        // brunskat->load();
        katView = new BrunsKatalogView();
      } else {
        /* normaler Vorlagenkatalog */
        katView = new TemplKatalogView();
      }

      if ( katView ) {
        katView->init(kat);
        katView->show();
        mKatalogViews[kat] = katView;
        KatalogMan::self()->registerKatalogListView( kat, katView->getListView() );
      }
      QApplication::restoreOverrideCursor();
    }
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

    if( t.length() <= width )
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
