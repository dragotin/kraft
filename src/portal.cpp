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
#include <QDir>
#include <QPrinter>
#include <QPainter>
#include <QApplication>
#include <QCursor>
#include <QTimer>
#include <QSqlDatabase>
#include <QSqlError>

// include files for KDE
#include <kaction.h>
#include <kactioncollection.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kmenu.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kstandardaction.h>
#include <kstandardshortcut.h>
#include <kcmdlineargs.h>
#include <krun.h>
#include <kapplication.h>
#include <akonadi/control.h>
#include <kabc/addressee.h>
#include <ktoolinvocation.h>
#include <kstandarddirs.h>
#include <kemailsettings.h>

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
#include "brunskatalogview.h"
#include "materialkatalogview.h"
#include "prefsdialog.h"
#include "documentman.h"
#include "docdigestview.h"
#include "archiveman.h"
#include "reportgenerator.h"
#include "kraftsettings.h"
#include "defaultprovider.h"
#include "archdoc.h"
#include "newdocassistant.h"
#include "doctype.h"
#include "tagtemplatesdialog.h"
#include "kraftview_ro.h"
#include "databasesettings.h"
#include "setupassistant.h"
#include "addressprovider.h"

#define ID_STATUS_MSG 1

Portal::Portal( QWidget *parent, KCmdLineArgs *args, const char* name)
: KXmlGuiWindow( parent, 0 ),
  mCmdLineArgs( args )
{
  setObjectName( name );
  ///////////////////////////////////////////////////////////////////
  // call inits to invoke all other construction parts
  initStatusBar();
  initActions();
  initView();

  setAttribute( Qt::WA_QuitOnClose );
  ///////////////////////////////////////////////////////////////////
  // disable actions at startup
  editCut->setEnabled(false);
  editCopy->setEnabled(false);
  editPaste->setEnabled(false);

  mAddressProvider = new AddressProvider( this );
  connect( mAddressProvider, SIGNAL( addresseeFound( const QString&, const KABC::Addressee&)),
          this, SLOT( slotReceivedMyAddress( const QString&, const KABC::Addressee& ) ) );

  setAutoSaveSettings();
  QTimer::singleShot( 0, this, SLOT( slotStartupChecks() ) );
}

void Portal::initActions()
{
  fileQuit = actionCollection()->addAction( KStandardAction::Quit, this, SLOT(slotFileQuit() ) );
  editCut = actionCollection()->addAction( KStandardAction::Cut, this, SLOT(slotEditCut() ) );
  editCopy = actionCollection()->addAction( KStandardAction::Copy, this, SLOT(slotEditCopy() ) );
  editPaste = actionCollection()->addAction( KStandardAction::Paste, this, SLOT(slotEditPaste() ) );
  viewStatusBar = KStandardAction::showStatusbar(this, SLOT(slotViewStatusBar()), actionCollection());

  actionCollection()->addAction( KStandardAction::Preferences, this, SLOT( preferences() ) );

  actNewDocument = actionCollection()->addAction( "document_new", this, SLOT( slotNewDocument()) );
  actNewDocument->setText( i18n("Create Document") );
  actNewDocument->setShortcut( KStandardShortcut::shortcut(KStandardShortcut::New) );
  actNewDocument->setIcon( KIcon("document-new"));

  actCopyDocument = actionCollection()->addAction( "document_copy", this, SLOT( slotCopyDocument()) );
  actCopyDocument->setText( i18n("Copy Document"));
  actCopyDocument->setShortcut( KStandardShortcut::shortcut(KStandardShortcut::Copy) );
  actCopyDocument->setIcon( KIcon( "document-edit"));

  actFollowDocument = actionCollection()->addAction( "document_follow", this, SLOT( slotFollowUpDocument() ) );
  actFollowDocument->setText( i18n("Follow Document" ));
  actFollowDocument->setShortcut( KShortcut( Qt::CTRL + Qt::Key_F ));
  actFollowDocument->setIcon( KIcon( "document-edit"));

  actPrintDocument = actionCollection()->addAction( "document_print", this, SLOT( slotPrintDocument()) );
  actPrintDocument->setText( i18n("Print Document"));
  actPrintDocument->setShortcut( KStandardShortcut::shortcut(KStandardShortcut::Print) );
  actPrintDocument->setIcon( KIcon("document-print"));

  actOpenArchivedDocument = actionCollection()->addAction( "archived_open", this, SLOT( slotArchivedDocExecuted()) );
  actOpenArchivedDocument->setText( i18n("Open Archived Document"));
  actOpenArchivedDocument->setShortcut( KShortcut(Qt::CTRL + Qt::Key_A) );

  actViewDocument  = actionCollection()->addAction( "document_view", this, SLOT( slotViewDocument()));
  actViewDocument->setText(i18n("Show Document"));
  actViewDocument->setShortcut( KStandardShortcut::shortcut(KStandardShortcut::Reload) );
  actViewDocument->setIcon( KIcon("document-preview" ));

  actOpenDocument = actionCollection()->addAction( "document_open", this, SLOT( slotOpenDocument()) );
  actOpenDocument->setText( i18n("Edit Document"));
  actOpenDocument->setShortcut( KStandardShortcut::shortcut(KStandardShortcut::Open) );
  actOpenDocument->setIcon( KIcon("document-open" ));

  actMailDocument = actionCollection()->addAction( "document_mail", this, SLOT( slotMailDocument()) );
  actMailDocument->setText(i18n("&Mail Document"));
  actMailDocument->setShortcut( KShortcut(Qt::CTRL + Qt::Key_M ));
  actMailDocument->setIcon( KIcon("mail-forward"));

  actEditTemplates = actionCollection()->addAction( "edit_tag_templates", this, SLOT( slotEditTagTemplates() ) );
  actEditTemplates->setText("Edit Tag Templates");
  actEditTemplates->setShortcut( KShortcut( Qt::CTRL + Qt::Key_E ));

  KAction *reconfDb = actionCollection()->addAction( "reconfigure_db", this, SLOT( slotReconfigureDatabase() ) );
  reconfDb->setText("Redo Initial Setup...");
  reconfDb->setShortcut( KShortcut( Qt::CTRL + Qt::Key_R ));

  fileQuit->setStatusTip(i18n("Quits the application"));
  editCut->setStatusTip(i18n("Cuts the selected section and puts it to the clipboard"));
  editCopy->setStatusTip(i18n("Copies the selected section to the clipboard"));
  editPaste->setStatusTip(i18n("Pastes the clipboard contents to current position"));
  viewStatusBar->setStatusTip(i18n("Enables/disables the statusbar"));

  actNewDocument->setStatusTip( i18n( "Creates a new Document" ) );
  actPrintDocument->setStatusTip( i18n( "Print and archive this Document" ) );
  actCopyDocument->setStatusTip( i18n( "Creates a new document which is a copy of the selected document" ) );
  actFollowDocument->setStatusTip( i18n( "Create a followup document for the current document" ) );
  actOpenDocument->setStatusTip( i18n( "Opens the document for editing" ) );
  actViewDocument->setStatusTip( i18n( "Opens a read only view on the document." ) );
  actMailDocument->setStatusTip( i18n( "Send document per mail" ) );
  actEditTemplates->setStatusTip( i18n("Edit the available tag templates which can be assigned to document items.") );
  reconfDb->setStatusTip( i18n( "Configure the Database Kraft is working on." ) );

  actOpenArchivedDocument->setStatusTip( i18n( "Open a viewer on an archived document" ) );
  setStandardToolBarMenuEnabled( true );
  actOpenDocument->setEnabled( false );
  actViewDocument->setEnabled( false );
  actPrintDocument->setEnabled( false );
  actCopyDocument->setEnabled( false );
  actFollowDocument->setEnabled( false );
  actMailDocument->setEnabled( false );

  actOpenArchivedDocument->setEnabled( false );
  // use the absolute path to your kraftui.rc file for testing purpose in createGUI();
  QString prjPath = QString::fromUtf8(qgetenv("KRAFT_HOME"));
  if( !prjPath.isEmpty() ) {
      createGUI(QString("%1/src/kraftui.rc").arg(prjPath));
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
    m_portalView = new PortalView( this, "PortalMainView" );
    QList<KMenu*> menus = m_portalView->docDigestView()->contextMenus();
    foreach( KMenu *menu, menus ) {
      menu->setTitle( i18n("Document Actions"));
      menu->addAction( actViewDocument );
      menu->addAction( actOpenDocument );
      menu->addAction( actNewDocument );
      menu->addAction( actCopyDocument );
      menu->addAction( actFollowDocument );
      menu->addSeparator();
      menu->addAction( actPrintDocument );
      menu->addAction( actMailDocument );
      menu->addAction( actOpenArchivedDocument );
    }

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
    connect( m_portalView, SIGNAL( viewDocument( const QString& ) ),
             this, SLOT( slotViewDocument( const QString& ) ) );
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
  QString dbName = DatabaseSettings::self()->dbDatabaseName();

  if ( !Akonadi::Control::start( this ) ) {
    kError() << "Failed to start Akonadi!";
  }
  Akonadi::Control::widgetNeedsAkonadi( this );
  // FIXME: This starts Akonadi and such, check if the decision if the setup
  // assistant is really needed can't be pulled here.
  SetupAssistant assi(this);
  if( assi.init( SetupAssistant::Update) ) {
    assi.exec();
  }

  if( ! KraftDB::self()->isOk() ) {
    QSqlError err = KraftDB::self()->lastError();
    kDebug() << "The last sql error id: " << err.type() << endl;

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
    actCopyDocument->setEnabled( false );
    actFollowDocument->setEnabled(false);
    actOpenDocument->setEnabled( false );
    actViewDocument->setEnabled( false );
    actOpenArchivedDocument->setEnabled( false );
    actMailDocument->setEnabled( false );

    slotStatusMsg( i18n( "Database Problem." ) );
  } else {
    // Database interaction is ok after this point.
    m_portalView->slotBuildView();
    m_portalView->fillCatalogDetails();
    m_portalView->fillSystemDetails();

    slotStatusMsg( i18n( "Check commandline actions" ) );

    if ( mCmdLineArgs ) {
      QString docId = mCmdLineArgs->getOption( "d" ); //  <documentId>" );
      if ( ! docId.isEmpty() ) {
        kDebug() << "open a archived document: " << docId << endl;
        slotPrintDocument( QString(), dbID( docId.toInt() ) );
      }

      mCmdLineArgs->clear();
    }

    // Fetch my address
    QString myUid = KraftSettings::self()->userUid();
    if( ! myUid.isEmpty() ) {
      kDebug() << "Got My UID: " << myUid;
      mAddressProvider->getAddressee( myUid );
    }

    slotStatusMsg( i18n( "Ready." ) );
  }
}

void Portal::slotReceivedMyAddress( const QString& uid, const KABC::Addressee& contact )
{
  if( contact.isEmpty() ) {
    kDebug() << "My-Contact is empty!";
    return;
  }

  myContact = contact;

  KraftSettings::self()->setUserUid( contact.uid() );
  KraftSettings::self()->writeConfig();

  kDebug() << "Received my address: " << contact.realName() << "(" << uid << ")";
  ReportGenerator::self()->setMyContact( contact );

  disconnect( mAddressProvider, SIGNAL( addresseeFound(const QString&, const KABC::Addressee&)),
              this, SLOT(slotReceivedMyAddress( const QString&, const KABC::Addressee&)));
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
    QApplication::setOverrideCursor( QCursor( Qt::BusyCursor ) );
  } else {
    QApplication::restoreOverrideCursor();
  }
}

void Portal::slotNewDocument()
{
  slotStatusMsg(i18n("Creating new document..."));

  KraftWizard wiz;
  wiz.init();
  if ( wiz.exec() ) {
    DocumentMan *docman = DocumentMan::self();
    DocGuardedPtr doc = docman->createDocument( wiz.docType() );

    doc->setDate( wiz.date() );
    doc->setAddressUid( wiz.addressUid() );
    // doc->setDocType( wiz.docType() );
    doc->setWhiteboard( wiz.whiteboard() );
    createView( doc );
  }
  slotStatusMsg(i18n("Ready."));
}

void Portal::slotFollowUpDocument()
{
  const QString locId = m_portalView->docDigestView()->currentDocumentId();

  DocGuardedPtr doc = DocumentMan::self()->openDocument( locId );

  DocType dt( doc->docType() );

  KraftWizard wiz;
  wiz.init();

  QStringList followers = dt.follower();
  if ( followers.count() > 0 ) {
    // only if there are currently followers defined, if not the default wiht
    // all doc types works.
    wiz.setAvailDocTypes( dt.follower() );
  }

  kDebug() << "doc identifier: "<< doc->docIdentifier() << endl;
  wiz.setDocIdentifier( doc->docIdentifier() );
  delete doc;
  if ( wiz.exec() ) {
    DocGuardedPtr doc = DocumentMan::self()->createDocument( dt.name(), locId );
    doc->setDate( wiz.date() );
    doc->setDocType( wiz.docType() );
    doc->setWhiteboard( wiz.whiteboard() );
    createView( doc );
  }
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
  DocGuardedPtr oldDoc = DocumentMan::self()->openDocument( id );

  KraftWizard wiz;
  wiz.init();
  if ( wiz.exec() ) {
    DocGuardedPtr doc = DocumentMan::self()->createDocument( oldDoc->docType(), id );
    doc->setDate( wiz.date() );
    doc->setDocType( wiz.docType() );
    doc->setWhiteboard( wiz.whiteboard() );
    doc->setAddressUid( wiz.addressUid() );
    doc->saveDocument();
    m_portalView->docDigestView()->slotUpdateView();
    kDebug() << "Document created from id " << id << ", saved with id " << doc->docID().toString() << endl;
  }
}

void Portal::slotOpenDocument()
{
  QString locId = m_portalView->docDigestView()->currentDocumentId();
  slotOpenDocument( locId );
}

void Portal::slotViewDocument()
{
  QString locId = m_portalView->docDigestView()->currentDocumentId();
  slotViewDocument( locId );
}

void Portal::slotViewDocument( const QString& id )
{
  QString locId = m_portalView->docDigestView()->currentDocumentId();

  slotStatusMsg(i18n("Opening document to view..."));

  if( !id.isEmpty() ) {
    DocumentMan *docman = DocumentMan::self();
    DocGuardedPtr doc = docman->openDocument( id );
    createROView( doc );
  }

  slotStatusMsg(i18n("Ready."));

}

void Portal::slotOpenArchivedDoc( const ArchDocDigest& d )
{
  busyCursor( true );
  ArchDocDigest digest( d );

  QString outputDir = ArchiveMan::self()->pdfBaseDir();
  QString filename = ArchiveMan::self()->archiveFileName( digest.archDocIdent(),
                                                          digest.archDocId().toString(), "pdf" );
  QString file = QString( "%1/%2" ).arg( outputDir ).arg( filename );

  kDebug() << "archived doc selected: " << file << endl;
  slotOpenPdf( file );


  busyCursor( false );
}

void Portal::slotPrintDocument()
{
  QString locId = m_portalView->docDigestView()->currentDocumentId();
  kDebug() << "printing document " << locId << endl;

  busyCursor( true );
  slotStatusMsg( i18n( "Generating PDF..." ) );
  DocumentMan *docman = DocumentMan::self();
  _currentDoc = docman->openDocument( locId );
  QString ident;

  if ( _currentDoc ) {
      ident = _currentDoc->ident();

      ArchiveMan *archman = ArchiveMan::self();
      dbID archID = archman->archiveDocument( _currentDoc );
      slotPrintDocument( ident, archID );
      // m_portalView->docDigestView()->addArchivedItem(docPtr->docID(), archID);
  }
  busyCursor( false );
  slotStatusMsg( i18n( "Ready." ) );

}

void Portal::slotMailDocument()
{
  QString locId = m_portalView->docDigestView()->currentDocumentId();
  kDebug() << "Mailing document " << locId << endl;

  slotStatusMsg( i18n( "Generating PDF..." ) );
  DocumentMan *docman = DocumentMan::self();
  DocGuardedPtr docPtr = docman->openDocument( locId );
  QString ident;
  if ( docPtr ) {
    ident = docPtr->ident();

    ArchiveMan *archman = ArchiveMan::self();
    dbID archID = archman->archiveDocument( docPtr );

    _clientId = docPtr->addressUid();
    busyCursor( true );

    connect( ReportGenerator::self(), SIGNAL( pdfAvailable( const QString& ) ),
             this, SLOT( slotMailPdfAvailable( const QString& ) ) );
    ReportGenerator::self()->createPdfFromArchive( ident, archID );
    busyCursor( false );
  }
  slotStatusMsg( i18n( "Ready." ) );
}

void Portal::slotMailPdfAvailable( const QString& fileName )
{
    if( fileName.isEmpty() ) {
        kDebug() << "Filename to mail is empty!";
        return;
    }

    QFileInfo fi(fileName);

    _pdfFileName = fi.canonicalFilePath();

    // get the email.
    if( !_clientId.isEmpty() && mAddressProvider ) {
        connect( mAddressProvider, SIGNAL(addresseeFound(QString,KABC::Addressee)),
                 this, SLOT(slotMailAddresseeFound(QString, KABC::Addressee)));
        mAddressProvider->getAddressee(_clientId);
        _clientId.clear();
    } else {
        slotMailAddresseeFound( QString::null, KABC::Addressee() );
    }

}

void Portal::slotMailAddresseeFound( const QString& uid, const KABC::Addressee& contact )
{
    QString mailReceiver;
    if( !contact.isEmpty() ) {
        mailReceiver = contact.fullEmail(); // the prefered email
    }

    kDebug() << "Found mail address " << mailReceiver << " for " << uid;

    kDebug() << "Mailing away " << _pdfFileName << endl;

    disconnect( mAddressProvider, SIGNAL(addresseeFound(QString,KABC::Addressee)),
             this, SLOT(slotMailAddresseeFound(QString, KABC::Addressee)));
    disconnect( ReportGenerator::self(), SIGNAL( pdfAvailable( const QString& ) ),0,0 );

    // Do a special way for thunderbird, as I could not get it to run
    // with the KToolInvocation
    KEMailSettings emailSettings;
    if( emailSettings.getSetting(KEMailSettings::ClientProgram).contains("thunderbird") ) {
        QString prog = emailSettings.getSetting(KEMailSettings::ClientProgram);
        QStringList args;

        args.append("-compose");
        QString tmp;
        if( !mailReceiver.isEmpty() ) {
            tmp = QString("to=%1,").arg(mailReceiver);
        }
        tmp += QString("attachment='file://%1'").arg(_pdfFileName);
        args.append(tmp);

        kDebug() << "Starting thunderbird: " << prog << args;

        if (!QProcess::startDetached(prog, args)) {
            kDebug() << "Failed to start thunderbird composer!";
        }
    } else {
        // Use KDE Invocation tool for all other mailers, good luck.
        // works with KMail at least.
        KUrl mailTo;
        mailTo.setProtocol("mailto");
        if ( ! mailReceiver.isEmpty() ) {
            mailTo.addQueryItem( "to", mailReceiver );
        }
        mailTo.addQueryItem("attach", _pdfFileName);

        kDebug() << "Use this mailto: " << mailTo << endl;

        KToolInvocation::self()->invokeMailer( mailTo, "Kraft", true );
    }
    _pdfFileName.clear();
}

/*
 * id    : document ID
 * archID: database ID of archived document
 */
void Portal::slotPrintDocument( const QString& id,  const dbID& archID )
{
  if ( archID.isOk() ) {
    slotStatusMsg(i18n("Printing archived document...") );
    ReportGenerator *repGen = ReportGenerator::self();
    connect( repGen, SIGNAL( pdfAvailable( const QString& ) ),
             this,  SLOT( slotOpenPdf( const QString& ) ) );

    repGen->createPdfFromArchive( id, archID ); // work on document identifier.
  }
}

void Portal::slotOpenPdf( const QString& fileName )
{
    disconnect( ReportGenerator::self(), SIGNAL( pdfAvailable( const QString& ) ),0,0 );
    KUrl url( fileName );
    KRun::runUrl( url, "application/pdf", this );

    // save pdf into a <customer>/<dockind> structure
    if( _currentDoc ) {
        QString uid = _currentDoc->addressUid();
        QString docType = _currentDoc->docType();

        if( !uid.isEmpty() ) {
            KStandardDirs stdDirs;

            QString outputDir = KraftSettings::self()->pdfOutputDir();
            if ( outputDir.isEmpty() ) {
                outputDir = stdDirs.saveLocation( "data", "kraft/archivePdf", true );
            }

            if ( ! outputDir.endsWith( "/" ) ) outputDir += QLatin1String("/");
            QDir customerDir(outputDir + QString("%1/%2").arg(uid).arg(docType));
            if( !customerDir.exists() ) {
                customerDir.mkpath( customerDir.absolutePath());
            }
            if( customerDir.exists() ) {
                QFileInfo fi(fileName);
                QString target = customerDir.canonicalPath() + QLatin1Char('/') + fi.fileName();
                QFileInfo tfi(target);
                if( tfi.exists() ) {
                    QFile::remove(target);
                }
                QFile::copy( fileName, target );
            }
        }
    }
}

void Portal::slotOpenDocument( const QString& id )
{
  slotStatusMsg( i18n("Opening document %1").arg(id ) );
  kDebug() << "Opening document " << id;
  if( !id.isEmpty() ) {
    DocumentMan *docman = DocumentMan::self();
    DocGuardedPtr doc = docman->openDocument( id );
    createView( doc );
  }

  slotStatusMsg(i18n("Ready."));
}

void Portal::slotDocumentSelected( const QString& doc )
{
  // kDebug() << "a doc was selected: " << doc << endl;
  if( doc.isEmpty() ) {
    actViewDocument->setEnabled( false );
    actOpenDocument->setEnabled( false );
    actPrintDocument->setEnabled( false );
    actCopyDocument->setEnabled( false );
    actFollowDocument->setEnabled( false );

    actMailDocument->setEnabled( false );
  } else {
    actViewDocument->setEnabled( true );
    actOpenDocument->setEnabled( true );
    actPrintDocument->setEnabled( true );
    actCopyDocument->setEnabled( true );
    actMailDocument->setEnabled( true );
    actFollowDocument->setEnabled( true );

    actOpenArchivedDocument->setEnabled( false );
  }
}

void Portal::slotArchivedDocExecuted()
{

  // ArchDocDigest dig = m_portalView->docDigestView()->currentArchiveDoc();

  // slotOpenArchivedDoc( dig );
}

void Portal::slotArchivedDocSelected( const ArchDocDigest& )
{
  // slotDocumentSelected( QString() );
  actOpenArchivedDocument->setEnabled( true );
  actViewDocument->setEnabled( false );
  actOpenDocument->setEnabled( false );
  actPrintDocument->setEnabled( false );
  actMailDocument->setEnabled( false );
}

void Portal::slotEditTagTemplates()
{
  TagTemplatesDialog dia( this );

  if ( dia.exec() ) {
    kDebug() << "Editing of tag templates succeeded!" << endl;

  }
}

void Portal::slotReconfigureDatabase()
{
  kDebug() << "Reconfiguring the Database";

  SetupAssistant assi(this);
  if( assi.init( SetupAssistant::Reinit ) ) {
    assi.exec();
  }
}

void Portal::createView( DocGuardedPtr doc )
{
  // FIXME: We allow only one view for the first time.
  // Later allow one write view and other read onlys.
  if ( !doc ) return;

  // if there is a read only view already, close it and open a
  // editor
  if( mViewMap.contains(doc)) {
      if( (mViewMap[doc])->type() == KraftViewBase::ReadOnly ) {
          KraftViewBase *view = mViewMap[doc];
          mViewMap.remove(doc);
          delete view;
      }
  }

  if( !mViewMap.contains(doc) ){
      KraftView *view = new KraftView( this );
      view->setup( doc );
      view->redrawDocument();
      QSize s = KraftSettings::self()->docViewSize();
      if ( !s.isValid() ) {
          s.setWidth( 640 );
          s.setHeight( 400 );
      }
      view->setInitialSize( s );
      view->slotSwitchToPage( KraftDoc::Positions );
      view->show();

      connect( view, SIGNAL( viewClosed( bool, DocGuardedPtr ) ),
               this, SLOT( slotViewClosed( bool, DocGuardedPtr ) ) );
      mViewMap[doc] = view;
  } else {
      mViewMap[doc]->raise();
      // pop first view to front
      kDebug() << "There is already a view for this doc!" << endl;
  }
}

void Portal::createROView( DocGuardedPtr doc )
{
    if ( !doc ) return;

    if( !mViewMap.contains(doc)) {
        KraftViewRO *view = new KraftViewRO( this );
        view->setup( doc );
        // view->redrawDocument();
        QSize s = KraftSettings::self()->rODocViewSize();
        if ( !s.isValid() ) {
            s.setWidth( 640 );
            s.setHeight( 400 );
        }
        view->setInitialSize( s );
        view->show();
        mViewMap[doc] = view;

        connect( view, SIGNAL( viewClosed( bool, DocGuardedPtr ) ),
                 this, SLOT( slotViewClosed( bool, DocGuardedPtr ) ) );
    } else {
        mViewMap[doc]->raise();
    }

}

void Portal::slotViewClosed( bool success, DocGuardedPtr doc )
{
    // doc is only valid on success!
    if ( doc )  {
        KraftViewBase *view = mViewMap[doc];
        if( success && view->type() == KraftViewBase::ReadWrite ) {
            DocDigestView *dv = m_portalView->docDigestView();
            dv->slotUpdateView();
        }
        if( mViewMap.contains(doc)) {
            mViewMap.remove(doc);
            view->deleteLater();
        }
        kDebug() << "A view was closed saving and doc is new: " << doc->isNew() << endl;
        delete doc;
    } else {
        kDebug() << "A view was closed canceled" << endl;
    }
}

void Portal::slotFileQuit()
{
  closeEvent(0);
}

void Portal::closeEvent( QCloseEvent *event )
{
    slotStatusMsg(i18n("Exiting..."));
    // close the first window, the list makes the next one the first again.
    // This ensures that queryClose() is called on each window to ask for closing

    //We have to delete katalogviews ourself otherwise the application keeps running in the background
    QMap<QString, KatalogView *>::iterator i;
    for (i = mKatalogViews.begin(); i != mKatalogViews.end(); ++i) {
        KatalogView *view = i.value();
        kDebug() << "Windowstate" << view->windowState();
        i.value()->deleteLater();
    }

    QListIterator<KMainWindow*> it( memberList() );
    while( it.hasNext() ) {
        KMainWindow *w = it.next();
        // only close the window if the closeEvent is accepted.
        if(!w->close())
            break;
    }

    if(event) {
    	KXmlGuiWindow::closeEvent(event);
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
  statusBar()->clearMessage();
  statusBar()->changeItem(text, ID_STATUS_MSG);
}

/** Show the  window with floskeltemplates */
void Portal::slotShowTemplates(){
}

void Portal::slotOpenKatalog(const QString& kat)
{
    kDebug() << "opening Katalog " << kat << endl;

    if ( mKatalogViews.contains( kat ) ) {
      // bring up the katalog view window.
      kDebug() << "Katalog " << kat << " already open in a view" << endl;

      mKatalogViews.value(kat)->show();
      mKatalogViews.value(kat)->raise();

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
        kDebug() << katView;
        katView->init(kat);
        katView->show();
        mKatalogViews.insert(kat, katView);
        KatalogMan::self()->registerKatalogListView( kat, katView->getListView() );
      }
      QApplication::restoreOverrideCursor();
    }
}

void Portal::slotOpenKatalog()
{
    kDebug() << "opening katalog!" << endl;
    KatalogView *katView = new TemplKatalogView(); //this);
    katView->show();
}

void Portal::slotKatalogToXML(const QString& katName)
{
    kDebug() << "Generating XML for catalog " << katName << endl;

    Katalog *kat = KatalogMan::self()->getKatalog(katName);

    if(kat) {
        kat->writeXMLFile();
    }
}

QString Portal::textWrap( const QString& t, int width )
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
            pos = t.indexOf( ' ', start+width );
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
  _prefsDialog = new PrefsDialog(this);
  connect( _prefsDialog, SIGNAL(finished(int)), SLOT(slotPrefsDialogFinished(int)) );
  connect( _prefsDialog, SIGNAL(newOwnIdentity(const QString&, KABC::Addressee)),
           SLOT(slotReceivedMyAddress(QString,KABC::Addressee)));
  _prefsDialog->setMyIdentity( myContact );

  _prefsDialog->open();
}

void Portal::slotPrefsDialogFinished( int result )
{
  if( result == QDialog::Accepted) {

  }
  _prefsDialog->deleteLater();
}

QWidget* Portal::mainWidget()
{
  if( m_portalView && m_portalView->currentPage() )
     return m_portalView->currentPage()->widget();
  return 0;
}
