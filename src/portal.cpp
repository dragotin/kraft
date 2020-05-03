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
#include <QPainter>
#include <QApplication>
#include <QCursor>
#include <QTimer>
#include <QSqlDatabase>
#include <QSqlError>
#include <QAction>
#include <QIcon>
#include <QMessageBox>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QLocale>
#include <QStatusBar>
#include <QStandardPaths>
#include <QToolBar>
#include <QDesktopServices>


// include files for KDE
#include <QDebug>
#include <kcontacts/addressee.h>
#include <kcontacts/vcardconverter.h>

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
#include "materialkatalogview.h"
#include "prefsdialog.h"
#include "documentman.h"
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
#include "alldocsview.h"


Portal::Portal(QWidget *parent, QCommandLineParser *commandLineParser, const char* name)
: QMainWindow( parent ),
  mCmdLineArgs( commandLineParser )
{
  setObjectName( name );
  ///////////////////////////////////////////////////////////////////
  // call inits to invoke all other construction parts
  initStatusBar();
  initActions();
  initView();

  setAttribute( Qt::WA_QuitOnClose );
  ///////////////////////////////////////////////////////////////////

  mAddressProvider = new AddressProvider( this );

  const QByteArray state = QByteArray::fromBase64( KraftSettings::self()->portalState().toAscii() );
  restoreState(state);
  const QByteArray geo = QByteArray::fromBase64( KraftSettings::self()->portalGeometry().toAscii() );
  restoreGeometry(geo);

  // setAutoSaveSettings();
  QTimer::singleShot( 0, this, SLOT( slotStartupChecks() ) );
}

void Portal::initActions()
{
    QIcon newIcon;
    newIcon = QIcon::fromTheme( "application-exit");
    _actFileQuit = new QAction(newIcon, i18n("&Quit"), this);
    _actFileQuit->setShortcuts(QKeySequence::Quit);
    connect(_actFileQuit, &QAction::triggered, this, &QWidget::close);

    newIcon = QIcon::fromTheme( "edit-cut");
    _actEditCut = new QAction(newIcon, i18n("&Cut"), this);
    _actEditCut->setShortcuts(QKeySequence::Cut);
    connect(_actEditCut, &QAction::triggered, this, &Portal::slotEditCut);

    newIcon = QIcon::fromTheme( "edit-copy");
    _actEditCopy = new QAction(newIcon, i18n("C&opy"), this);
    _actEditCopy->setShortcuts(QKeySequence::Copy);
    connect(_actFileQuit, &QAction::triggered, this, &Portal::slotEditCopy);

    newIcon = QIcon::fromTheme( "edit-paste");
    _actEditPaste = new QAction(newIcon, i18n("&Paste"), this);
    _actEditPaste->setShortcuts(QKeySequence::Paste);
    connect(_actEditPaste, &QAction::triggered, this, &Portal::slotEditPaste);

    newIcon = QIcon::fromTheme( "settings-configure");
    _actPreferences = new QAction(newIcon, i18n("&Settings"), this);
    _actPreferences->setShortcuts(QKeySequence::Preferences);
    connect(_actPreferences, &QAction::triggered, this, &Portal::preferences);

    newIcon = QIcon::fromTheme( "document-new");
    _actNewDocument = new QAction(newIcon, i18n("&Create Document"), this);
    _actNewDocument->setShortcuts(QKeySequence::New);
    connect(_actNewDocument, &QAction::triggered, this, &Portal::slotNewDocument);

    _actCopyDocument = new QAction(newIcon, i18n("&Copy Document"), this);
    // _actCopyDocument->setShortcuts();
    connect(_actCopyDocument, &QAction::triggered, this, &Portal::slotCopyCurrentDocument);

    _actFollowDocument = new QAction(newIcon, i18n("Create &Followup Document"), this);
    _actFollowDocument->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_F ));
    connect(_actFollowDocument, &QAction::triggered, this, &Portal::slotFollowUpDocument);

    newIcon = QIcon::fromTheme( "document-print");
    _actPrintDocument = new QAction(newIcon, i18n("Print Document"), this);
    _actPrintDocument->setShortcut( QKeySequence::Print);
    connect(_actPrintDocument, &QAction::triggered, this, &Portal::slotPrintCurrentDocument);

    newIcon = QIcon::fromTheme( "document-view");
    _actViewDocument = new QAction(newIcon, i18n("Show Document"), this);
    _actViewDocument->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_R ));
    connect(_actViewDocument, &QAction::triggered, this, &Portal::slotViewCurrentDocument);

    newIcon = QIcon::fromTheme( "document-edit");
    _actOpenDocument = new QAction(newIcon, i18n("Edit Document"), this);
    _actOpenDocument->setShortcut( QKeySequence::Open );
    connect(_actOpenDocument, &QAction::triggered, this, &Portal::slotOpenCurrentDocument);
    
    newIcon = QIcon::fromTheme( "document-edit");
    _actOpenArchivedDocument = new QAction(newIcon, i18n("Open Archived Document"), this);
    _actOpenArchivedDocument->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_A ));
    connect(_actOpenArchivedDocument, &QAction::triggered, this, &Portal::slotArchivedDocExecuted);

    newIcon = QIcon::fromTheme( "mail-forward");
    _actMailDocument = new QAction(newIcon, i18n("Mail Document"), this);
    _actMailDocument->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_M ));
    connect(_actMailDocument, &QAction::triggered, this, &Portal::slotMailDocument);

    newIcon = QIcon::fromTheme( "settings-configure");
    _actEditTemplates= new QAction(newIcon, i18n("Edit Tag Templates"), this);
    _actEditTemplates->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_E ));
    connect(_actEditTemplates, &QAction::triggered, this, &Portal::slotEditTagTemplates);
    
    newIcon = QIcon::fromTheme( "settings-configure");
    _actReconfDb = new QAction(newIcon, i18n("Redo Initial Setup..."), this);
    _actReconfDb->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_R ));
    connect(_actReconfDb, &QAction::triggered, this, &Portal::slotReconfigureDatabase);

    newIcon = QIcon::fromTheme( "help-about");
    _actHandbook = new QAction(newIcon, i18n("Kraft Handbook..."), this);
    _actHandbook->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_H ));
    connect(_actHandbook, &QAction::triggered, this, &Portal::slotHandbook);

    _actAboutQt = new QAction(newIcon, i18n("About Qt..."), this);
    _actAboutQt->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_Q ));
    connect(_actAboutQt, &QAction::triggered, this, &Portal::slotAboutQt);

    _actAboutKraft = new QAction(newIcon, i18n("About Kraft..."), this);
    _actAboutKraft->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_K ));
    connect(_actAboutKraft, &QAction::triggered, this, &Portal::slotAboutKraft);

    _actFileQuit->setStatusTip(i18n("Quits the application"));

    _actEditCut->setStatusTip(i18n("Cuts the selected section and puts it to the clipboard"));
    _actEditCopy->setStatusTip(i18n("Copies the selected section to the clipboard"));
    _actEditPaste->setStatusTip(i18n("Pastes the clipboard contents to current position"));

    _actNewDocument->setStatusTip( i18n( "Creates a new Document" ) );
    _actPrintDocument->setStatusTip( i18n( "Print and archive this Document" ) );
    _actCopyDocument->setStatusTip( i18n( "Creates a new document which is a copy of the selected document" ) );
    _actFollowDocument->setStatusTip( i18n( "Create a followup document for the current document" ) );
    _actOpenDocument->setStatusTip( i18n( "Opens the document for editing" ) );
    _actViewDocument->setStatusTip( i18n( "Opens a read only view on the document." ) );
    _actMailDocument->setStatusTip( i18n( "Send document per mail" ) );
    _actEditTemplates->setStatusTip( i18n("Edit the available tag templates which can be assigned to document items.") );
    _actReconfDb->setStatusTip( i18n( "Configure the Database Kraft is working on." ) );
    _actOpenArchivedDocument->setStatusTip( i18n( "Open a viewer on an archived document" ) );

    _actOpenDocument->setEnabled( false );
    _actViewDocument->setEnabled( false );
    _actPrintDocument->setEnabled( false );
    _actCopyDocument->setEnabled( false );
    _actFollowDocument->setEnabled( false );
    _actMailDocument->setEnabled( false );

    _actOpenArchivedDocument->setEnabled( false );
    // use the absolute path to your kraftui.rc file for testing purpose in createGUI();
    QString prjPath = QString::fromUtf8(qgetenv("KRAFT_HOME"));

    QMenu *fileMenu = menuBar()->addMenu(i18n("&File"));
    fileMenu->addAction(_actFileQuit);

#if 0
    QMenu *editMenu = menuBar()->addMenu(i18n("&Edit"));
    editMenu->addAction(_actEditCopy);
    editMenu->addAction(_actEditCut);
    editMenu->addAction(_actEditPaste);
#endif
    QMenu *docMenu = menuBar()->addMenu(i18n("&Document"));
    docMenu->addAction(_actViewDocument);
    docMenu->addAction(_actOpenDocument);
    docMenu->addAction(_actOpenArchivedDocument);
    docMenu->addSeparator();
    docMenu->addAction(_actNewDocument);
    docMenu->addAction(_actCopyDocument);
    docMenu->addAction(_actFollowDocument);
    docMenu->addSeparator();
    docMenu->addAction(_actPrintDocument);
    docMenu->addAction(_actMailDocument);

    QToolBar *toolBar = addToolBar(i18n("Kraft"));

    QMenu *prefsMenu = menuBar()->addMenu(i18n("&Preferences"));
    prefsMenu->addAction(_actEditTemplates);
    prefsMenu->addAction(_actReconfDb);
    prefsMenu->addSeparator();
    QMenu *submen = prefsMenu->addMenu(i18n("Toolbars"));
    submen->addAction(toolBar->toggleViewAction());
    prefsMenu->addSeparator();
    prefsMenu->addAction(_actPreferences);

    QMenu *helpMenu = menuBar()->addMenu(i18n("&Help"));
    helpMenu->addAction(_actHandbook);
    helpMenu->addSeparator();
    helpMenu->addAction(_actAboutKraft);
    helpMenu->addAction(_actAboutQt);

    // Toolbar
    toolBar->setObjectName("PortalToolbar");
    toolBar->addAction(_actNewDocument);
    toolBar->addAction(_actCopyDocument);
    toolBar->addAction(_actFollowDocument);
    toolBar->addAction(_actPrintDocument);
    toolBar->addAction(_actMailDocument);


    // initial enablements
    _actEditCut->setEnabled(false);
    _actEditCopy->setEnabled(false);
    _actEditPaste->setEnabled(false);


}


void Portal::initStatusBar()
{
  ///////////////////////////////////////////////////////////////////
  // STATUSBAR
  // TODO: add your own items you need for displaying current application status.
  statusBar()->showMessage(i18n("Ready."));
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
    m_portalView.reset(new PortalView( this, "PortalMainView" ));
    QVector<QMenu*> menus = m_portalView->docDigestView()->contextMenus();
    foreach( QMenu *menu, menus ) {
      menu->setTitle( i18n("Document Actions"));
      menu->addSection(i18n("Document Actions"));
      menu->addAction( _actViewDocument );
      menu->addAction( _actOpenDocument );
      menu->addAction( _actOpenArchivedDocument );
      menu->addSeparator();
      menu->addAction( _actNewDocument );
      menu->addAction( _actCopyDocument );
      menu->addAction( _actFollowDocument );
      menu->addSeparator();
      menu->addAction( _actPrintDocument );
      menu->addAction( _actMailDocument );
    }

    connect( m_portalView.data(), SIGNAL(openKatalog( const QString&)),
             this, SLOT(slotOpenKatalog(const QString&)));
    connect( m_portalView.data(), SIGNAL(katalogToXML(const QString& )),
             this, SLOT(slotKatalogToXML(const QString&)));

    // document related connections
    connect( m_portalView.data(), SIGNAL( createDocument() ),
             this, SLOT( slotNewDocument() ) );
    connect( m_portalView.data(), SIGNAL( copyDocument( const QString& ) ),
             this, SLOT( slotCopyDocument( const QString& ) ) );
    connect( m_portalView.data(), SIGNAL( openDocument( const QString& ) ),
             this, SLOT( slotOpenDocument( const QString& ) ) );
    connect( m_portalView.data(), SIGNAL( viewDocument( const QString& ) ),
             this, SLOT( slotViewDocument( const QString& ) ) );
    connect( m_portalView.data(), SIGNAL( openArchivedDocument( const ArchDocDigest& ) ),
             this, SLOT( slotOpenArchivedDoc( const ArchDocDigest& ) ) );
    connect( m_portalView.data(),  SIGNAL( documentSelected( const QString& ) ),
             this,  SLOT( slotDocumentSelected( const QString& ) ) );
    connect( m_portalView.data(),  SIGNAL( archivedDocSelected( const ArchDocDigest& ) ),
             this,  SLOT( slotArchivedDocSelected( const ArchDocDigest& ) ) );
    setCentralWidget(m_portalView.data());
}

void Portal::slotStartupChecks()
{
    const QString dbName = DatabaseSettings::self()->dbDatabaseName();

    SetupAssistant assi(this);
    if( assi.init( SetupAssistant::Update) ) {
        assi.exec();
    }

    if( ! KraftDB::self()->isOk() ) {
        QSqlError err = KraftDB::self()->lastError();
        // qDebug () << "The last sql error id: " << err.type() << endl;

        QString text;

        if ( err.text().contains( "Can't connect to local MySQL server through socket" ) ) {
            text = i18n( "Kraft can not connect to the specified MySQL server. "
                         "Please check the Kraft database settings, check if the server is "
                         "running and verify if a database with the name %1 exits!" , dbName );
        } else if ( err.text().contains( "Unknown database '" + dbName + "' QMYSQL3: Unable to connect" ) ) {
            text = i18n( "The database with the name %1 does not exist on the database server. "
                         "Please make sure the database exists and is accessible by the user "
                         "running Kraft.", dbName );
        } else if ( err.text().contains( "Driver not loaded" ) ) {
            text = i18n( "The Qt database driver could not be loaded. That probably means, that "
                         "they are not installed. Please make sure the Qt database packages are "
                         "installed and try again." );
        } else {
            text = i18n( "There is a database problem: %1", err.text() );
        }

        m_portalView->systemInitError( m_portalView->ptag( text, "problem" ) );

        // disable harmfull actions
        _actNewDocument->setEnabled( false );
        _actPrintDocument->setEnabled( false );
        _actCopyDocument->setEnabled( false );
        _actFollowDocument->setEnabled(false);
        _actOpenDocument->setEnabled( false );
        _actViewDocument->setEnabled( false );
        _actOpenArchivedDocument->setEnabled( false );
        _actMailDocument->setEnabled( false );

        slotStatusMsg( i18n( "Database Problem." ) );
    } else {
        // Database interaction is ok after this point.
        m_portalView->slotBuildView();
        m_portalView->fillCatalogDetails();
        m_portalView->fillSystemDetails();

        slotStatusMsg( i18n( "Check commandline actions" ) );

        if ( mCmdLineArgs ) {
            // KF5: Proper command line option

            // QString docId = mCmdLineArgs->getOption( "d" ); //  <documentId>" );
            QString docId;
            if ( ! docId.isEmpty() ) {
                // qDebug () << "open a archived document: " << docId << endl;
                slotPrintDocument( QString(), dbID( docId.toInt() ) );
            }
        }

        // Fetch my address
        const QString myUid = KraftSettings::self()->userUid();
        bool useManual = false;

        if( ! myUid.isEmpty() ) {
            KContacts::Addressee contact;
            // qDebug () << "Got My UID: " << myUid;
            connect( mAddressProvider, SIGNAL( lookupResult(QString,KContacts::Addressee)),
                    this, SLOT( slotReceivedMyAddress(QString, KContacts::Addressee)) );

            AddressProvider::LookupState state = mAddressProvider->lookupAddressee( myUid );
            switch( state ) {
            case AddressProvider::LookupFromCache:
                contact = mAddressProvider->getAddresseeFromCache(myUid);
                slotReceivedMyAddress(myUid, contact);
                break;
            case AddressProvider::LookupNotFound:
            case AddressProvider::ItemError:
            case AddressProvider::BackendError:
                // Try to read from stored vcard.
                useManual = true;
                break;
            case AddressProvider::LookupOngoing:
            case AddressProvider::LookupStarted:
                // Not much to do, just wait
                break;
            }
        } else {
            // in case there is no uid in the settings file, try to use the manual address.
            useManual = true;
        }

        if( useManual ) {
            // check if the vcard can be read
            QString file = QStandardPaths::writableLocation( QStandardPaths::AppDataLocation );
            file += "/myidentity.vcd";
            QFile f(file);
            if( f.exists() ) {
                if( f.open( QIODevice::ReadOnly )) {
                    const QByteArray data = f.readAll();
                    VCardConverter converter;
                    Addressee::List list = converter.parseVCards( data );

                    if( list.count() > 0 ) {
                        KContacts::Addressee c = list.at(0);
                        c.insertCustom(CUSTOM_ADDRESS_MARKER, "manual");
                        slotReceivedMyAddress(QString(), c);
                    }
                }
            }
        }

        connect( &_reportGenerator, &ReportGenerator::docAvailable,
                 this, &Portal::slotDocConverted);
        connect( &_reportGenerator, &ReportGenerator::failure,
                 this, &Portal::slotDocConvertionFail);

        slotStatusMsg( i18n( "Ready." ) );
    }
}

void Portal::slotReceivedMyAddress( const QString& uid, const KContacts::Addressee& contact )
{
    disconnect( mAddressProvider, SIGNAL(lookupResult(QString,KContacts::Addressee)),
                this, SLOT(slotReceivedMyAddress(QString, KContacts::Addressee)));

    if( contact.isEmpty() ) {
        if( !uid.isEmpty() ) {
            // FIXME: Read the stored Address and compare the uid
            const QString err = mAddressProvider->errorMsg(uid);
            qDebug () << "My-Contact could not be found:" << err;
        }
        return;
    }

    myContact = contact;

    // qDebug () << "Received my address: " << contact.realName() << "(" << uid << ")";
    _reportGenerator.setMyContact( myContact );

    QString name = myContact.formattedName();
    if( !name.isEmpty() ) {
        name = i18n("Welcome to Kraft, %1", name);
        statusBar()->showMessage(name, 30*1000);
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
    QApplication::setOverrideCursor( QCursor( Qt::BusyCursor ) );
  } else {
    QApplication::restoreOverrideCursor();
  }
}

void Portal::slotNewDocument()
{
  slotStatusMsg(i18n("Creating new document..."));

  KraftWizard wiz;
  wiz.init(true);
  if ( wiz.exec() ) {
    DocumentMan *docman = DocumentMan::self();
    DocGuardedPtr doc = docman->createDocument( wiz.docType() );

    doc->setDate( wiz.date() );
    doc->setAddressUid( wiz.addressUid() );
    doc->setWhiteboard( wiz.whiteboard() );
    createView( doc );
  }
  slotStatusMsg(i18n("Ready."));
}

void Portal::slotFollowUpDocument()
{
    const QString locId = m_portalView->docDigestView()->currentDocumentId();

    DocGuardedPtr sourceDoc = DocumentMan::self()->openDocument( locId );

    DocType dt( sourceDoc->docType() );

    KraftWizard wiz;
    wiz.init( false, i18nc("Dialog title of the followup doc dialog, followed by the id of the  source doc",
                           "Create follow up document for %1", sourceDoc->ident()));

    QStringList followers = dt.follower();
    if ( followers.count() > 0 ) {
        // only if there are currently followers defined, if not the default wiht
        // all doc types works.
        wiz.setAvailDocTypes( dt.follower() );
    }

    // qDebug () << "doc identifier: "<< doc->docIdentifier() << endl;
    wiz.setDocToFollow( sourceDoc );
    DocPositionList posToCopy;
    delete sourceDoc;

    if ( wiz.exec() ) {
        QString selectedId = wiz.copyItemsFromPredecessor();
        if(!selectedId.isEmpty()) {
            DocGuardedPtr copyDoc = DocumentMan::self()->openDocument( selectedId );
            posToCopy = copyDoc->positions();
            delete copyDoc;
        }

        DocGuardedPtr doc = DocumentMan::self()->createDocument(wiz.docType(), locId, posToCopy);
        doc->setDate( wiz.date() );
        doc->setWhiteboard( wiz.whiteboard() );
        createView( doc );
    }
}

void Portal::slotCopyCurrentDocument()
{
  const QString locId = m_portalView->docDigestView()->currentDocumentId();
  slotCopyDocument( locId );
}

void Portal::slotCopyDocument( const QString& id )
{
  if ( id.isEmpty() ) {
    return;
  }
  QString oldDocIdent;
  DocGuardedPtr oldDoc = DocumentMan::self()->openDocument( id );
  if(oldDoc) {
      const DocType dt = oldDoc->docType();
      oldDocIdent = i18nc("Title of the new doc dialog, %1 is the source doc id",
                          "Create new Document as Copy of %1", oldDoc->ident());
      delete oldDoc;
  }

  KraftWizard wiz;
  wiz.init(true, oldDocIdent);
  if ( wiz.exec() ) {
    DocGuardedPtr doc = DocumentMan::self()->copyDocument(id);
    doc->setDate( wiz.date() );
    doc->setDocType( wiz.docType() );
    doc->setWhiteboard( wiz.whiteboard() );
    if(doc->addressUid() != wiz.addressUid() ) {
        doc->setAddress(QString());
    }
    doc->setAddressUid( wiz.addressUid() );
    doc->saveDocument();
    m_portalView->docDigestView()->slotUpdateView();
    // qDebug () << "Document created from id " << id << ", saved with id " << doc->docID().toString() << endl;
  }
}

void Portal::slotOpenCurrentDocument()
{
  QString locId = m_portalView->docDigestView()->currentDocumentId();
  slotOpenDocument( locId );
}

void Portal::slotViewCurrentDocument()
{
  QString locId = m_portalView->docDigestView()->currentDocumentId();
  slotViewDocument( locId );
}

void Portal::slotViewDocument( const QString& id )
{
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

  // qDebug () << "archived doc selected: " << file << endl;
  slotOpenPdf( file );


  busyCursor( false );
}

void Portal::slotPrintCurrentDocument()
{
  QString locId = m_portalView->docDigestView()->currentDocumentId();
  // qDebug () << "printing document " << locId << endl;

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
  const QString locId = m_portalView->docDigestView()->currentDocumentId();
  // qDebug () << "Mailing document " << locId << endl;

  slotStatusMsg( i18n( "Generating PDF for EMail" ) );
  DocumentMan *docman = DocumentMan::self();
  DocGuardedPtr docPtr = docman->openDocument( locId );
  QString ident;
  if ( docPtr ) {
    ident = docPtr->ident();

    ArchiveMan *archman = ArchiveMan::self();
    dbID archID = archman->archiveDocument( docPtr );

    _clientId = docPtr->addressUid();
    busyCursor( true );

    _reportGenerator.createDocument(ReportFormat::PDFMail, ident, archID );
    busyCursor( false );
  }
  slotStatusMsg( i18n( "Ready." ) );
}

void Portal::slotDocConvertionFail(const QString& failString)
{
    QMessageBox::warning(this, i18n("Doc Generation Error"),failString);
}

void Portal::slotDocConverted(ReportFormat format, const QString& file, const KContacts::Addressee& customerContact)
{
    if (format == ReportFormat::PDF) {
        slotOpenPdf(file);
    } else if (format == ReportFormat::PDFMail) {
        openInMailer(file, customerContact);
    }
}

void Portal::openInMailer(const QString& fileName, const KContacts::Addressee& contact)
{
   QString mailReceiver;
    if( !contact.isEmpty() ) {
        mailReceiver = contact.fullEmail(); // the prefered email
    }

    QStringList args;
    QString prog;

    if( KraftSettings::self()->mailUA().startsWith("xdg") ) {
        args.append( "--utf8");
        args.append( "--attach");
        args.append(_pdfFileName );
        if( !mailReceiver.isEmpty() ) {
            args.append( mailReceiver);
        }
        prog = QLatin1String("/usr/bin/xdg-email");
    } else {
        // Fallback to thunderbird
        prog = QLatin1String("/usr/bin/thunderbird");

        args.append("-compose");
        QString tmp;
        if( !mailReceiver.isEmpty() ) {
            tmp = QString("to=%1,").arg(mailReceiver);
        }
        tmp += QString("attachment='file://%1'").arg(fileName);
        args.append(tmp);
    }
    qDebug () << "Starting mailer: " << prog << args;

    if (!QProcess::startDetached(prog, args)) {
        qDebug () << "Failed to start thunderbird composer!";
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

    _reportGenerator.createDocument(ReportFormat::PDF, id, archID ); // work on document identifier.
  }
}

void Portal::slotOpenPdf( const QString& fileName )
{
    QUrl url( fileName );
    QDesktopServices::openUrl(url);
}

/*
 * A special document tree is built up here.
 */
void Portal::savePdfInCustomerStructure(const QString& fileName)
{
    // save pdf into a <customer>/<dockind> structure
    if( _currentDoc ) {
        QString uid = _currentDoc->addressUid();
        QString docType = _currentDoc->docType();

        if( !uid.isEmpty() ) {
            QString outputDir = KraftSettings::self()->pdfOutputDir();
            if ( outputDir.isEmpty() ) {
                outputDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
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
  slotStatusMsg( i18n("Opening document %1", id ) );
  // qDebug () << "Opening document " << id;
  if( !id.isEmpty() ) {
    DocumentMan *docman = DocumentMan::self();
    DocGuardedPtr doc = docman->openDocument( id );
    createView( doc );
  }

  slotStatusMsg(i18n("Ready."));
}

void Portal::slotDocumentSelected( const QString& doc )
{
    // qDebug() << "a doc was selected: " << doc << endl;
    bool enable = !doc.isEmpty();
    _actViewDocument->setEnabled( enable );
    _actOpenDocument->setEnabled( enable );
    _actPrintDocument->setEnabled( enable );
    _actCopyDocument->setEnabled( enable );
    _actMailDocument->setEnabled( enable );
    _actFollowDocument->setEnabled( enable );

    _actOpenArchivedDocument->setEnabled( enable );

}

void Portal::slotArchivedDocExecuted()
{
  ArchDocDigest dig = m_portalView->docDigestView()->currentLatestArchivedDoc();
  slotOpenArchivedDoc( dig );
}

void Portal::slotArchivedDocSelected( const ArchDocDigest& )
{
  // slotDocumentSelected( QString() );
  _actOpenArchivedDocument->setEnabled( true );
  _actViewDocument->setEnabled( false );
  _actOpenDocument->setEnabled( false );
  _actPrintDocument->setEnabled( false );
  _actMailDocument->setEnabled( false );
}

void Portal::slotEditTagTemplates()
{
  TagTemplatesDialog dia( this );

  if ( dia.exec() ) {
    // qDebug () << "Editing of tag templates succeeded!" << endl;

  }
}

void Portal::slotReconfigureDatabase()
{
  // qDebug () << "Reconfiguring the Database";

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
      const QByteArray geo = QByteArray::fromBase64( KraftSettings::self()->docEditGeometry().toAscii() );
      view->restoreGeometry(geo);
      view->setup( doc );
      view->redrawDocument();
      view->slotSwitchToPage( KraftDoc::Positions );
      view->show();

      connect( view, SIGNAL( viewClosed( bool, DocGuardedPtr ) ),
               this, SLOT( slotViewClosed( bool, DocGuardedPtr ) ) );
      connect( view, &KraftViewBase::openROView, this, &Portal::slotViewDocument );

      mViewMap[doc] = view;
  } else {
      mViewMap[doc]->raise();
      // pop first view to front
      // qDebug () << "There is already a view for this doc!" << endl;
  }
}

void Portal::createROView( DocGuardedPtr doc )
{
    if ( !doc ) return;

    if( !mViewMap.contains(doc)) {
        KraftViewRO *view = new KraftViewRO( this );
        view->setup( doc );
        // view->redrawDocument();
        const QByteArray geo = QByteArray::fromBase64( KraftSettings::self()->docViewROGeometry().toAscii() );
        view->restoreGeometry(geo);
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
        const QByteArray geo = view->saveGeometry().toBase64();
        if( success ) {
            if( view->type() == KraftViewBase::ReadWrite ) {
                AllDocsView *dv = m_portalView->docDigestView();
                dv->slotUpdateView();
                KraftSettings::self()->setDocEditGeometry(geo);
            } else {
                KraftSettings::self()->setDocViewROGeometry(geo);
            }
        }
        if( mViewMap.contains(doc)) {
            mViewMap.remove(doc);
            view->deleteLater();
        }

        // qDebug () << "A view was closed saving and doc is new: " << doc->isNew() << endl;
        delete doc;
    } else {
        // qDebug () << "A view was closed canceled" << endl;
    }
}

void Portal::closeEvent( QCloseEvent *event )
{
    slotStatusMsg(i18n("Exiting..."));
    // close the first window, the list makes the next one the first again.
    // This ensures that queryClose() is called on each window to ask for closing

    //We have to delete katalogviews ourself otherwise the application keeps running in the background
    QMap<QString, KatalogView *>::iterator i;
    for (i = mKatalogViews.begin(); i != mKatalogViews.end(); ++i) {
        // KatalogView *view = i.value();
        // qDebug () << "Windowstate" << view->windowState();
        i.value()->deleteLater();
    }

    // FIXME: Close the document windows.

    const QByteArray state = saveState().toBase64();
    KraftSettings::self()->setPortalState(state);
    const QByteArray geo = saveGeometry().toBase64();
    KraftSettings::self()->setPortalGeometry(geo);

    KraftSettings::self()->save();

    if( event )
        event->accept();
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


void Portal::slotStatusMsg(const QString &text)
{
  ///////////////////////////////////////////////////////////////////
  // change status message permanently
  statusBar()->clearMessage();
  statusBar()->showMessage(text);
}

/** Show the  window with floskeltemplates */
void Portal::slotShowTemplates(){
}

void Portal::slotOpenKatalog(const QString& kat)
{
    // qDebug () << "opening Katalog " << kat << endl;

    if ( mKatalogViews.contains( kat ) ) {
      // bring up the katalog view window.
      // qDebug () << "Katalog " << kat << " already open in a view" << endl;

      mKatalogViews.value(kat)->show();
      mKatalogViews.value(kat)->raise();

    } else {
      QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

      KatalogView *katView = nullptr;
      if( kat == MaterialKatalogView::MaterialCatalogName ) {

        /* Materialkatalog */
        katView = new MaterialKatalogView();
      } else {
        /* normaler Vorlagenkatalog */
        katView = new TemplKatalogView();
      }
      if ( katView ) {
        // qDebug () << katView;
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
    // qDebug () << "opening katalog!" << endl;
    KatalogView *katView = new TemplKatalogView(); //this);
    katView->show();
}

void Portal::slotKatalogToXML(const QString& katName)
{
    // qDebug () << "Generating XML for catalog " << katName << endl;

    Katalog *kat = KatalogMan::self()->getKatalog(katName);

    if(kat) {
        kat->writeXMLFile();
    }
}

QString Portal::textWrap( const QString& t, int width, int maxLines )
{
    QString re;
    int lines = 0;

    if( t.length() <= width )
    {
        re = t;
    }
    else
    {
        int start = 0;
        int pos = width;
        while( pos < t.length() && (lines < maxLines || maxLines < 0) )
        {
            pos = t.indexOf( QLatin1Char('\n'), start );
            if( pos > -1 && (pos-start) < width ) {
                re += t.mid(start, pos-start)+QLatin1Char('\n');
                start = pos+1;
            } else {
                pos = t.indexOf( QLatin1Char(' '), start+width );
                if( pos > -1 ) {
                    re += t.mid( start, pos-start)+QLatin1Char('\n');
                    start = pos+1;
                } else {
                    re += t.mid( start );
                    pos = t.length();
                }
            }
            lines++;
        }
        if( lines == maxLines && pos != t.length() ) {
            re += QStringLiteral("...");
        }
    }

    return re;
}

void Portal::preferences()
{
  _prefsDialog = new PrefsDialog(this);
  connect( _prefsDialog, SIGNAL(finished(int)), SLOT(slotPrefsDialogFinished(int)) );
  connect( _prefsDialog, SIGNAL(newOwnIdentity(const QString&, KContacts::Addressee)),
           SLOT(slotReceivedMyAddress(QString,KContacts::Addressee)));
  _prefsDialog->setMyIdentity( myContact, mAddressProvider->backendUp() );

  _prefsDialog->open();
}

void Portal::slotPrefsDialogFinished( int result )
{
  if( result == QDialog::Accepted) {

  }
  _prefsDialog->deleteLater();
}

void Portal::slotHandbook()
{
    QUrl url;

    QLocale *loc = DefaultProvider::self()->locale();

    QString hbLocale;
    if (loc) {
        hbLocale = loc->bcp47Name();
    }

    // find the localized version
    QString hbFile = DefaultProvider::self()->locateFile(QString("manual/kraft-%1.html").arg(hbLocale));

    // if not found, fall back to the english manual
    QFileInfo fi(hbFile);
    if (hbFile.isEmpty() || !fi.exists()) {
        hbFile = DefaultProvider::self()->locateFile(QStringLiteral("manual/kraft-en.html"));
    }

    if( !hbFile.isEmpty() ) {
        url = QUrl::fromLocalFile(hbFile);
        qDebug() << "opening manual url" << url.toString();
    }

    if (!url.isEmpty()) {
        QDesktopServices::openUrl(url);
    }
}

void Portal::slotAboutQt()
{
    QApplication::aboutQt();
}

void Portal::slotAboutKraft()
{
    m_portalView->displaySystemsTab();
}

QWidget* Portal::mainWidget()
{
     return m_portalView.data();
}
