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
#include <QLocale>
#include <QStatusBar>
#include <QStandardPaths>


// include files for KDE
#include <QDebug>
#include <kcontacts/addressee.h>
#include <kactioncollection.h>
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
: KXmlGuiWindow( parent ),
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
  // disable actions at startup
  editCut->setEnabled(false);
  editCopy->setEnabled(false);
  editPaste->setEnabled(false);

  mAddressProvider = new AddressProvider( this );

  const QByteArray state = QByteArray::fromBase64( KraftSettings::self()->portalState().toAscii() );
  restoreState(state);
  const QByteArray geo = QByteArray::fromBase64( KraftSettings::self()->portalGeometry().toAscii() );
  restoreGeometry(geo);

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

  // KF5: Set shortcuts appropiately.
  actNewDocument = actionCollection()->addAction( "document_new", this, SLOT( slotNewDocument()) );
  actNewDocument->setText( i18n("Create Document") );
  // actNewDocument->setShortcut( QStandardShortcut::shortcut(KStandardShortcut::New) );
  actNewDocument->setIcon( QIcon::fromTheme("document-new"));

  actCopyDocument = actionCollection()->addAction( "document_copy", this, SLOT( slotCopyDocument()) );
  actCopyDocument->setText( i18n("Copy Document"));
  // actCopyDocument->setShortcut( KStandardShortcut::shortcut(KStandardShortcut::Copy) );
  actCopyDocument->setIcon( QIcon::fromTheme( "document-edit"));

  actFollowDocument = actionCollection()->addAction( "document_follow", this, SLOT( slotFollowUpDocument() ) );
  actFollowDocument->setText( i18n("Follow Document" ));
  // actFollowDocument->setShortcut( KShortcut( Qt::CTRL + Qt::Key_F ));
  actFollowDocument->setIcon( QIcon::fromTheme( "document-edit"));

  actPrintDocument = actionCollection()->addAction( "document_print", this, SLOT( slotPrintDocument()) );
  actPrintDocument->setText( i18n("Print Document"));
  // actPrintDocument->setShortcut( KStandardShortcut::shortcut(KStandardShortcut::Print) );
  actPrintDocument->setIcon( QIcon::fromTheme("document-print"));

  actOpenArchivedDocument = actionCollection()->addAction( "archived_open", this, SLOT( slotArchivedDocExecuted()) );
  actOpenArchivedDocument->setText( i18n("Open Archived Document"));
  actOpenArchivedDocument->setShortcut( QKeySequence(Qt::CTRL + Qt::Key_A) );

  actViewDocument  = actionCollection()->addAction( "document_view", this, SLOT( slotViewDocument()));
  actViewDocument->setText(i18n("Show Document"));
  actViewDocument->setShortcut( QKeySequence(Qt::CTRL+Qt::Key_R) );
  actViewDocument->setIcon( QIcon::fromTheme("document-preview" ));

  actOpenDocument = actionCollection()->addAction( "document_open", this, SLOT( slotOpenDocument()) );
  actOpenDocument->setText( i18n("Edit Document"));
  actOpenDocument->setShortcut( QKeySequence(Qt::CTRL+Qt::Key_O) );
  actOpenDocument->setIcon( QIcon::fromTheme("document-open" ));

  actMailDocument = actionCollection()->addAction( "document_mail", this, SLOT( slotMailDocument()) );
  actMailDocument->setText(i18n("&Mail Document"));
  actMailDocument->setShortcut( QKeySequence(Qt::CTRL + Qt::Key_M ));
  actMailDocument->setIcon( QIcon::fromTheme("mail-forward"));

  actEditTemplates = actionCollection()->addAction( "edit_tag_templates", this, SLOT( slotEditTagTemplates() ) );
  actEditTemplates->setText("Edit Tag Templates");
  actEditTemplates->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_E ));

  QAction *reconfDb = actionCollection()->addAction( "reconfigure_db", this, SLOT( slotReconfigureDatabase() ) );
  reconfDb->setText("Redo Initial Setup...");
  reconfDb->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_R ));

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
    m_portalView = new PortalView( this, "PortalMainView" );
    QVector<QMenu*> menus = m_portalView->docDigestView()->contextMenus();
    foreach( QMenu *menu, menus ) {
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
        if( ! myUid.isEmpty() ) {
            // qDebug () << "Got My UID: " << myUid;
            connect( mAddressProvider, SIGNAL( lookupResult(QString,KContacts::Addressee)),
                    this, SLOT( slotReceivedMyAddress(QString, KContacts::Addressee)) );

            AddressProvider::LookupState state = mAddressProvider->lookupAddressee( myUid );
            KContacts::Addressee contact;
            switch( state ) {
            case AddressProvider::LookupFromCache:
                contact = mAddressProvider->getAddresseeFromCache(myUid);
                slotReceivedMyAddress(myUid, contact);
                break;
            case AddressProvider::LookupNotFound:
            case AddressProvider::ItemError:
            case AddressProvider::BackendError:
                // Try to read from stored vcard.
                slotReceivedMyAddress(myUid, contact);
                break;
            case AddressProvider::LookupOngoing:
            case AddressProvider::LookupStarted:
                // Not much to do, just wait
                break;
            }
        } else {
            // check if the vcard can be read
            QString file = QStandardPaths::writableLocation( QStandardPaths::AppDataLocation );
            file += "/myidentity.vcd";
            QFile f(file);
            if( f.exists() ) {
                if( f.open( QIODevice::ReadOnly )) {
                    QByteArray data = f.readAll();

                    VCardConverter converter;
                    Addressee::List list = converter.parseVCards( data );

                    if( list.count() > 0 ) {
                        slotReceivedMyAddress(QString::null, list.at(0));
                    }
                }
            }
        }

        slotStatusMsg( i18n( "Ready." ) );
    }
}

void Portal::slotReceivedMyAddress( const QString& uid, const KContacts::Addressee& contact )
{
    disconnect( mAddressProvider, SIGNAL(lookupResult(QString,KContacts::Addressee)),
                this, SLOT(slotReceivedMyAddress(QString, KContacts::Addressee)));

    if( contact.isEmpty() ) {
        if( !uid.isEmpty() ) {
            const QString err = mAddressProvider->errorMsg(uid);
            qDebug () << "My-Contact is empty: " << err;
        }
        return;
    }

    myContact = contact;

    if( !uid.isEmpty() ) {
        KraftSettings::self()->setUserUid( contact.uid() );
        KraftSettings::self()->writeConfig();
    }

    // qDebug () << "Received my address: " << contact.realName() << "(" << uid << ")";
    ReportGenerator::self()->setMyContact( contact );
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

  // qDebug () << "doc identifier: "<< doc->docIdentifier() << endl;
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
    // qDebug () << "Document created from id " << id << ", saved with id " << doc->docID().toString() << endl;
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

  // qDebug () << "archived doc selected: " << file << endl;
  slotOpenPdf( file );


  busyCursor( false );
}

void Portal::slotPrintDocument()
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
  QString locId = m_portalView->docDigestView()->currentDocumentId();
  // qDebug () << "Mailing document " << locId << endl;

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
        // qDebug () << "Filename to mail is empty!";
        return;
    }

    QFileInfo fi(fileName);

    _pdfFileName = fi.canonicalFilePath();

    // get the email.
    if( !_clientId.isEmpty() && mAddressProvider ) {
        connect( mAddressProvider, SIGNAL(lookupResult(QString,KContacts::Addressee)),
                 this, SLOT(slotMailAddresseeFound(QString, KContacts::Addressee)));
        mAddressProvider->lookupAddressee(_clientId);
        _clientId.clear();
    } else {
        slotMailAddresseeFound( QString::null, KContacts::Addressee() );
    }

}

void Portal::slotMailAddresseeFound( const QString& uid, const KContacts::Addressee& contact )
{
    Q_UNUSED(uid);

    QString mailReceiver;
    if( !contact.isEmpty() ) {
        mailReceiver = contact.fullEmail(); // the prefered email
    }

    // qDebug () << "Found mail address " << mailReceiver << " for " << uid;

    // qDebug () << "Mailing away " << _pdfFileName << endl;

    disconnect( mAddressProvider, SIGNAL(lookupResult(QString,KContacts::Addressee)),
             this, SLOT(slotMailAddresseeFound(QString, KContacts::Addressee)));
    disconnect( ReportGenerator::self(), SIGNAL( pdfAvailable( const QString& ) ),0,0 );

    QString mailAgent("thunderbird");
    if( mailAgent.contains("thunderbird") ) {
        QString prog = "/usr/bin/thunderbird";
        QStringList args;

        args.append("-compose");
        QString tmp;
        if( !mailReceiver.isEmpty() ) {
            tmp = QString("to=%1,").arg(mailReceiver);
        }
        tmp += QString("attachment='file://%1'").arg(_pdfFileName);
        args.append(tmp);

        // qDebug () << "Starting thunderbird: " << prog << args;

        if (!QProcess::startDetached(prog, args)) {
            // qDebug () << "Failed to start thunderbird composer!";
        }
    } else {
       // FIXME porting
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
    QUrl url( fileName );
    QDesktopServices::openUrl(url);

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
  slotStatusMsg( i18n("Opening document %1").arg(id ) );
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
        // KatalogView *view = i.value();
        // qDebug () << "Windowstate" << view->windowState();
        i.value()->deleteLater();
    }

    QListIterator<KMainWindow*> it( memberList() );
    while( it.hasNext() ) {
        KMainWindow *w = it.next();
        // only close the window if the closeEvent is accepted.
        if(!w->close())
            break;
    }

    const QByteArray state = saveState().toBase64();
    KraftSettings::self()->setPortalState(state);
    const QByteArray geo = saveGeometry().toBase64();
    KraftSettings::self()->setPortalGeometry(geo);

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

      KatalogView *katView = 0;
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

QWidget* Portal::mainWidget()
{
     return m_portalView;
}
