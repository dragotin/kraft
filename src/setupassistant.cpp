/***************************************************************************
        setupassistant  - assistant to setup kraft from scratch
                             -------------------
    begin                : 2009-12-26
    copyright            : (C) 2009 by Klaas Freitag
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

#include <kdebug.h>
#include <kstandarddirs.h>

#include "setupassistant.h"
#include "katalogsettings.h"
#include "defaultprovider.h"
#include "kraftdb.h"


WelcomePage::WelcomePage(QWidget *parent)
  :QWidget(parent)
{
  QVBoxLayout *vbox = new QVBoxLayout;
  parent->setLayout( vbox );
  vbox->setSpacing( KDialog::spacingHint() );
  vbox->setMargin( KDialog::marginHint() );

  QWidget *w = new QWidget;
  vbox->addWidget( w );

  ui.setupUi(w);

}

void WelcomePage::setWelcomeText( const QString& txt )
{
  ui.mStatusText->setText( txt );
}

// ---------------------------------------------------------------------------

DbSelectPage::DbSelectPage(QWidget *parent)
  :QWidget(parent)
{
  QVBoxLayout *vbox = new QVBoxLayout;
  parent->setLayout( vbox );
  vbox->setSpacing( KDialog::spacingHint() );
  vbox->setMargin( KDialog::marginHint() );

  QWidget *w = new QWidget;
  vbox->addWidget( w );

  ui.setupUi(w);
}

QString DbSelectPage::selectedDriver()
{
  QString re = "QSQLITE";
  if( ui.mRbMySQL->isChecked() ) {
    re = "QMYSQL";
  }
  return re;
}

// ---------------------------------------------------------------------------

SqLiteDetailsPage::SqLiteDetailsPage(QWidget *parent)
  :QWidget(parent)
{
  QVBoxLayout *vbox = new QVBoxLayout;
  parent->setLayout( vbox );
  vbox->setSpacing( KDialog::spacingHint() );
  vbox->setMargin( KDialog::marginHint() );

  QWidget *w = new QWidget;
  vbox->addWidget( w );

  ui.setupUi(w);

  ui.mFileUrl->setMode( KFile::File | KFile::LocalOnly );

  connect( ui.mFileUrl, SIGNAL( textChanged( const QString& )), this, SLOT( slotSelectCustom() ) );
}

void SqLiteDetailsPage::slotSelectCustom()
{
  ui.mRbCustom->setChecked(true);
}

KUrl SqLiteDetailsPage::url()
{
  if( ui.mRbDefault->isChecked() ) {
    return KUrl( KStandardDirs::locateLocal( "appdata", "sqlite/kraft.db") );
  }
  return ui.mFileUrl->url();
}

// ---------------------------------------------------------------------------

MysqlDetailsPage::MysqlDetailsPage(QWidget *parent)
  :QWidget(parent)
{
  QVBoxLayout *vbox = new QVBoxLayout;
  parent->setLayout( vbox );
  vbox->setSpacing( KDialog::spacingHint() );
  vbox->setMargin( KDialog::marginHint() );

  QWidget *w = new QWidget;
  vbox->addWidget( w );

  ui.setupUi(w);
}

QString MysqlDetailsPage::dbName()
{
  return ui.mMysqlDbName->text();
}

QString MysqlDetailsPage::dbUser()
{
  return ui.mMysqUser->text();
}

QString MysqlDetailsPage::dbServer()
{
  return ui.mMysqlHost->text();
}

QString MysqlDetailsPage::dbPasswd()
{
  return ui.mMysqlPwd->text();
}
// ---------------------------------------------------------------------------

CreateDbPage::CreateDbPage(QWidget *parent)
  :QWidget(parent)
{
  QVBoxLayout *vbox = new QVBoxLayout;
  parent->setLayout( vbox );
  vbox->setSpacing( KDialog::spacingHint() );
  vbox->setMargin( KDialog::marginHint() );

  QWidget *w = new QWidget;
  vbox->addWidget( w );

  ui.setupUi(w);
}

void CreateDbPage::setStatusText( const QString& t )
{
  ui.mCreateStatus->setText( t );
}

void CreateDbPage::setFillCmdsCount( int cnt )
{
  ui.mFillProgress->setMaximum( cnt );
  ui.mFillProgress->setValue( 0 );
  ui.mFillCounter->setText( i18n("0/%1").arg(cnt));
}
void CreateDbPage::setFillCmdsCurrent( int cnt )
{
  ui.mFillProgress->setValue( cnt );
}

void CreateDbPage::setCreateCmdsCount( int cnt )
{
  ui.mCreateProgress->setMaximum( cnt );
  ui.mCreateProgress->setValue( 0 );
  ui.mCreateCounter->setText( i18n("0/%1").arg(cnt));
}

void CreateDbPage::setCreateCmdsCurrent( int cnt )
{
  ui.mCreateProgress->setValue( cnt );
}

void CreateDbPage::slotStatusMessage( const QString& msg )
{
  kDebug() << "############### success: " << msg;
  ui.mCreateStatus->setText( msg );
}

void CreateDbPage::slotCountCreateProgress( bool res )
{
  if( res ) {
    int cnt = ui.mCreateProgress->value();
    ui.mCreateProgress->setValue( cnt+1 );
    ui.mCreateCounter->setText( i18n("%1/%2").arg( cnt +1).arg( ui.mCreateProgress->maximum() ) );
  }
}

void CreateDbPage::slotCountFillProgress( bool res )
{
  if( res ) {
    int cnt = ui.mFillProgress->value();
    ui.mFillProgress->setValue( cnt+1 );
    ui.mFillCounter->setText( i18n("%1/%2").arg( cnt +1).arg( ui.mFillProgress->maximum() ) );
  }
}

// ---------------------------------------------------------------------------

UpgradeDbPage::UpgradeDbPage(QWidget *parent)
  :QWidget(parent)
{
  QVBoxLayout *vbox = new QVBoxLayout;
  parent->setLayout( vbox );
  vbox->setSpacing( KDialog::spacingHint() );
  vbox->setMargin( KDialog::marginHint() );

  QWidget *w = new QWidget;
  vbox->addWidget( w );

  ui.setupUi(w);
}

void UpgradeDbPage::slotSetStatusText( const QString& txt )
{
  ui.mUpgradeStatus->setText( txt );
}

void UpgradeDbPage::slotSetOverallCount( int cnt )
{
  ui.mUpgradeProgress->setMaximum( cnt );
  ui.mUpgradeProgress->setValue( 0 );
  updateCounter();
}

void UpgradeDbPage::updateCounter()
{
  ui.mUpgradeCounter->setText( i18n("%1/%2").arg( ui.mUpgradeProgress->value()).arg( ui.mUpgradeProgress->maximum() ));
}

void UpgradeDbPage::slotCountFillProgress( bool res )
{
  if( res ) {
    int cnt = ui.mUpgradeProgress->value();
    ui.mUpgradeProgress->setValue( cnt+1 );
    updateCounter();
  }
}


// ---------------------------------------------------------------------------

FinalStatusPage::FinalStatusPage(QWidget *parent)
  :QWidget(parent)
{
  QVBoxLayout *vbox = new QVBoxLayout;
  parent->setLayout( vbox );
  vbox->setSpacing( KDialog::spacingHint() );
  vbox->setMargin( KDialog::marginHint() );

  QWidget *w = new QWidget;
  vbox->addWidget( w );

  ui.setupUi(w);
  ui.mStatusText->setTextFormat( Qt::RichText );
}

void FinalStatusPage::slotSetStatusText( const QString& txt )
{
  ui.mStatusText->setText( txt );
}

// ---------------------------------------------------------------------------

SetupAssistant::SetupAssistant( QWidget *parent )
  :KAssistantDialog( parent ),
  mMode( Reinit )
{
  QWidget *w = new QWidget;
  mWelcomePageItem = addPage( w, i18n("Welcome to the Kraft Setup Assistant"));
  mWelcomePage = new WelcomePage(w);

  w = new QWidget;
  mDbSelectPageItem = addPage( w, i18n("Select the Database Backend"));
  mDbSelectPage = new DbSelectPage(w);

  w = new QWidget;
  mMysqlDetailsPageItem = addPage( w, i18n("Enter MySQL Database Setup Details"));
  mMysqlDetailsPage = new MysqlDetailsPage(w);

  w = new QWidget;
  mSqLiteDetailsPageItem = addPage( w, i18n("Enter SqLite Filename"));
  mSqLiteDetailsPage = new SqLiteDetailsPage(w);

  w = new QWidget;
  mCreateDbPageItem = addPage( w, i18n("Creating the Database..."));
  mCreateDbPage= new CreateDbPage(w);

  w = new QWidget;
  mUpgradeDbPageItem = addPage( w, i18n("Upgrade the Database Schema"));
  mUpgradeDbPage = new UpgradeDbPage(w);

  w = new QWidget;
  mFinalStatusPageItem = addPage( w, i18n("Setup Finished."));
  mFinalStatusPage = new FinalStatusPage(w);

  connect( this, SIGNAL( currentPageChanged( KPageWidgetItem*,KPageWidgetItem*) ),
           this, SLOT( slotCurrentPageChanged( KPageWidgetItem*,KPageWidgetItem*) ) );
  connect( this, SIGNAL( slotButtonClicked(int) ), this, SLOT( slotButtonClicked(int) ) );
}

void SetupAssistant::next( )
{
  KPageWidgetItem *item = currentPage();
  kDebug() << "Next was hit with " << item->name();

  if( item == mWelcomePageItem ) {
    kDebug() << "Nothing to do for the Welcome-Page";
  } else if( item == mDbSelectPageItem ) {
    handleDatabaseBackendSelect();
  } else if( item == mMysqlDetailsPageItem ) {
    // get the mysql datails
  } else if( item == mSqLiteDetailsPageItem ) {
    // get the sqlite filename
    handleSqLiteDetails();
  }

  KAssistantDialog::next();
}

void SetupAssistant::back()
{
  KAssistantDialog::back();
}

void SetupAssistant::slotCurrentPageChanged( KPageWidgetItem *current, KPageWidgetItem* /* previous */)
{
  if( current == mCreateDbPageItem ) {
    if( ! KraftDB::self()->databaseExists() ) {
      kDebug() << "Start to create the database";
      startDatabaseCreation();
    } else {
      kDebug() << "CreateDB-Page: Database already existing";
      mCreateDbPage->setStatusText( i18n( "<p>The database is already existing, no action needs to be taken here.</p>"
                                          "<p>Please hit <b>next</b> to proceed.</p>" ) );
    }
  }

  if( current == mUpgradeDbPageItem ) {
    if( KraftDB::self()->databaseExists() ) {
      kDebug() << "start to update the database";
      startDatabaseUpdate();
    } else {
      kDebug() << "Strange problem at dbupdate: DB does not exist";
    }
  }

  if( current == mFinalStatusPageItem )  {
    finalizePage();
  }
}

void SetupAssistant::slotButtonClicked( int buttCode )
{
  if( buttCode == KDialog::User1 ) { // Button "Finished"
    KatalogSettings::self()->setDbDriver( mDbSelectPage->selectedDriver() );
    if( mDbSelectPage->selectedDriver() == "QSQLITE" ) {
      KatalogSettings::self()->setDbFile( mSqLiteDetailsPage->url().pathOrUrl() ); // The sqLite file name
    }
    if( mDbSelectPage->selectedDriver() == "QMYSQL" ) {
      KatalogSettings::self()->setDbDatabaseName( mMysqlDetailsPage->dbName() );
      KatalogSettings::self()->setDbUser( mMysqlDetailsPage->dbUser() );
      KatalogSettings::self()->setDbServerName( mMysqlDetailsPage->dbServer() );
      KatalogSettings::self()->setDbPassword( mMysqlDetailsPage->dbPasswd() );
    }
    KatalogSettings::self()->writeConfig();
  }
  KAssistantDialog::slotButtonClicked( buttCode );

}

void SetupAssistant::finalizePage()
{
  QString txt;

  if( mErrors.isEmpty() ) {
    txt = i18n( "<p>The database setup was successfully completed.</p> " );
    txt += i18n("<p>You can start to work with Kraft now. Please do not forget to");
    txt += i18n("<ul><li>Enter your own address in the KAddressBook</li>");
    txt += i18n("<li>Adjust various settings in the Kraft Preferences dialog.</li>" );
    txt += i18n("<li>Check the Catalog chapter list.</li>" );
    txt += "</ul></p>";
    txt += i18n("<p>If you press <i>Finish</i> now, the new database configuration is stored in Krafts configuration.</p>");
  } else {
    foreach( QString err, mErrors ) {
      txt += "<p>" + err + "</p>";
    }
  }
  mFinalStatusPage->slotSetStatusText( txt );
}

void SetupAssistant::startDatabaseUpdate()
{
  if( ! KraftDB::self()->isOk() ) {
    mCreateDbPage->setStatusText( i18n("The Database can not be connected. Please check the database credentials!"));
    enableButton( KDialog::User2, false );
    return;
  }

  if( !KraftDB::self()->databaseExists() ) {
    mCreateDbPage->setStatusText( i18n("The database core tables do not exist, please check initial setup!"));
    enableButton( KDialog::User2, false );
    return;
  }
  enableButton( KDialog::User2, true );

  if( KraftDB::self()->currentSchemaVersion() == KraftDB::self()->requiredSchemaVersion() ) {
    mUpgradeDbPage->slotSetStatusText( i18n("Database is up to date, no upgrade required."));
    return;
  }

  // Database really needs update
  mUpgradeDbPage->slotSetStatusText( i18n("Parse Update Commands..."));

  int overallCmdCount = 0;
  QList<SqlCommandList> commandLists;
  int currentVer = KraftDB::self()->currentSchemaVersion();
  if( currentVer == -1 ) currentVer = 1; // set to initial version

  while ( currentVer < KraftDB::self()->requiredSchemaVersion() ) {
    ++currentVer;
    const QString migrateFilename = QString( "%1_dbmigrate.sql" ).arg( currentVer );
    kDebug() << "######### Reading " << migrateFilename;
    mUpgradeDbPage->slotSetStatusText( i18n("Reading upgrade command file %1").arg( migrateFilename ) );
    SqlCommandList cmds = KraftDB::self()->parseCommandFile( migrateFilename );
    overallCmdCount += cmds.count();
    commandLists << cmds;
  }
  mUpgradeDbPage->slotSetOverallCount( overallCmdCount );

  kDebug() << "4.";
  connect( KraftDB::self(), SIGNAL( statusMessage( const QString& ) ),
           mUpgradeDbPage,  SLOT( slotSetStatusText( const QString& ) ) );

  connect( KraftDB::self(), SIGNAL( processedSqlCommand( bool ) ),
           mUpgradeDbPage, SLOT( slotCountFillProgress( bool ) ) );

  int doneOverallCmds =  0;
  bool errors = false;

  currentVer = KraftDB::self()->currentSchemaVersion();
  foreach( SqlCommandList cmds, commandLists ) {  
    currentVer++;
    int goodCmds = KraftDB::self()->processSqlCommands( cmds );
    doneOverallCmds += goodCmds;
    if( goodCmds != cmds.count() ) {
      kDebug() << "Only performned " << goodCmds << " out of " << cmds.count();
      errors = true;
      break;
    } else {
      kDebug() << goodCmds << " commands performed well!";
      KraftDB::self()->setSchemaVersion( QString::number( currentVer ));
    }
  }

  if( errors ) {
    mUpgradeDbPage->slotSetStatusText( i18n("The Upgrade failed!") );;
  } else {
    mUpgradeDbPage->slotSetStatusText( i18n("The Upgrade succeeded, the current schema version is %1!")
                                       .arg( KraftDB::self()->requiredSchemaVersion() ) );;
  }

  disconnect( mUpgradeDbPage, SLOT( slotSetStatusText( const QString& )));
}


void SetupAssistant::startDatabaseCreation()
{
  if( ! KraftDB::self()->isOk() ) {
    mCreateDbPage->setStatusText( i18n("The Database can not be connected. Please check the database credentials!"));
    enableButton( KDialog::User2, false );
    return;
  }
  enableButton( KDialog::User2, true );

  mCreateDbPage->setStatusText( i18n("Parse Create Commands...") );
  SqlCommandList createCommands = KraftDB::self()->parseCommandFile( "create_schema.sql");

  QString dbFill( "fill_schema_en.sql" );

  if ( DefaultProvider::self()->locale()->country() == "de" ) {
    dbFill = "fill_schema_de.sql";
  }
  mCreateDbPage->setStatusText( i18n( "Parse database fillup commands..." ) );

  SqlCommandList fillCommands = KraftDB::self()->parseCommandFile( dbFill );
  mCreateDbPage->setCreateCmdsCount( createCommands.count() );
  mCreateDbPage->setCreateCmdsCurrent( 0 );
  mCreateDbPage->setFillCmdsCount( fillCommands.count() );
  mCreateDbPage->setFillCmdsCurrent( 0 );

  connect( KraftDB::self(), SIGNAL( statusMessage( const QString& ) ),
           mCreateDbPage, SLOT( slotStatusMessage( const QString& ) ) );
  connect( KraftDB::self(), SIGNAL( processedSqlCommand( bool ) ),
           mCreateDbPage, SLOT( slotCountCreateProgress( bool ) ) );

  mCreateDbPage->setStatusText( i18n( "Processing database creation commands...") );

  int creates = KraftDB::self()->processSqlCommands( createCommands );

  bool res = true;
  if( creates != createCommands.count() ) {
    kDebug() << "NOT all create commands succeeded!";
    res = false;
  } else {
    kDebug( ) << creates << "(=All) create commands succeeded!";

    // lets do the fillup
    connect( KraftDB::self(), SIGNAL( statusMessage( const QString& ) ),
             mCreateDbPage, SLOT( slotStatusMessage( const QString& ) ) );

    disconnect( mCreateDbPage, SLOT( slotCountCreateProgress(bool) ) );

    connect( KraftDB::self(), SIGNAL( processedSqlCommand( bool ) ),
             mCreateDbPage, SLOT( slotCountFillProgress( bool ) ) );

    mCreateDbPage->setStatusText( i18n( "Process database fillup commands..." ) );
    creates = KraftDB::self()->processSqlCommands( fillCommands );
  }

  if( res ) {
    mCreateDbPage->setStatusText( i18n( "Successfully finished commands." ) );
  } else {
    mCreateDbPage->setStatusText( i18n( "Failed to perform all commands." ) );
    // FIXME: Disable next button
  }
  disconnect( mCreateDbPage, SLOT( slotCountFillProgress(bool) ) );
  disconnect( mCreateDbPage, SLOT( slotStatusMessage( const QString&) ) );
}

void SetupAssistant::handleDatabaseBackendSelect()
{
  kDebug() << "Set backend driver type " << mDbSelectPage->selectedDriver();
  if( mDbSelectPage->selectedDriver() == "QSQLITE" ) {
    setAppropriate( mMysqlDetailsPageItem, false );
    setAppropriate( mSqLiteDetailsPageItem, true );
  } else {
    setAppropriate( mMysqlDetailsPageItem, true );
    setAppropriate( mSqLiteDetailsPageItem, false );
  }
}

void SetupAssistant::handleSqLiteDetails()
{
  QString file = mSqLiteDetailsPage->url().pathOrUrl();
  kDebug() << "The SqlLite database file is " << file;

  QString driver = mDbSelectPage->selectedDriver();
  kDebug() << "The database driver is " << driver;
  KraftDB::self()->dbConnect( driver, file );

  kDebug() << "############ database opened: "<< KraftDB::self()->isOk();
  bool dbExists = KraftDB::self()->databaseExists();

  kDebug() << "Database exists: " << dbExists;
  if( dbExists ) {
    kDebug() << "Database exists, no create needed";
    setAppropriate( mCreateDbPageItem, false );
  } else {
    setAppropriate( mCreateDbPageItem, true );
  }
  kDebug() << "required Schema version: " << KraftDB::self()->requiredSchemaVersion();
}

bool SetupAssistant::init( Mode mode )
{
  bool startDialog = false;
  QString text;

  mMode = mode;

  if( mMode == Reinit ) {
    startDialog = true;

    text = i18n("The Database is going to be reset.");
  } else if( mode == Update ) {
    KraftDB::self()->dbConnect(); // try to connect with default values
    if( KraftDB::self()->databaseExists() ) {
      kDebug() << "The database exists.";

      if( KraftDB::self()->currentSchemaVersion() != KraftDB::self()->requiredSchemaVersion() ) {
        kDebug() << "Need a database schema update.";
        startDialog = true;
      } else {
        kDebug() << "Database Schema is ok, nothing to do for StartupAssistant";
      }
    } else {
      kDebug() << "The database is not existing, need to start update!";
      startDialog = true;

      text = i18n( "<p>There was no valid database configuration found.</p>"
                   "<p>A new database can be created automatically from scratch.</p>");
    }
  }

  if( startDialog ) {
    text += "<p>Please follow the instructions in this assistant</p>";
    mWelcomePage->setWelcomeText( text );
  }
  return startDialog ;
}

void SetupAssistant::createDatabase( bool doIt )
{
  setAppropriate( mDbSelectPageItem, doIt );
}


SetupAssistant::~SetupAssistant()
{

}

