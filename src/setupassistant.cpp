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

#include <QDebug>

#include "setupassistant.h"
#include "databasesettings.h"
#include "defaultprovider.h"
#include "kraftdb.h"
#include "addressselectorwidget.h"
#include "kraftsettings.h"


WelcomePage::WelcomePage(QWidget *parent)
  :QWizardPage(parent)
{
    setTitle( i18n("Welcome to the Kraft Setup Assistant"));
    QVBoxLayout *vbox = new QVBoxLayout;
    setLayout( vbox );

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
    :QWizardPage(parent)
{
    setTitle(i18n("Select the Database Backend"));
    QVBoxLayout *vbox = new QVBoxLayout;
    setLayout( vbox );

    QWidget *w = new QWidget;
    vbox->addWidget( w );

    ui.setupUi(w);

    registerField("SelectedDbDriverSqlite", ui.mRbSqlite3);
    registerField("SelectedDbDriverMySql",  ui.mRbMySQL);

}

int DbSelectPage::nextId() const
{
    if( ui.mRbSqlite3->isChecked() ) {
        return SetupAssistant::sqlitePageNo;
    } else {
        return SetupAssistant::mySqlPageNo;
    }
}

QString DbSelectPage::selectedDriver() const
{
    QString re = "QSQLITE";
    if( field("SelectedDbDriverMySql").toBool() ) {
        re = "QMYSQL";
    }
    return re;
}

// ---------------------------------------------------------------------------

SqLiteDetailsPage::SqLiteDetailsPage(QWidget *parent)
    :QWizardPage(parent)
{
    setTitle(i18n("Sqlite File Name"));
    QVBoxLayout *vbox = new QVBoxLayout;
    setLayout(vbox);

    QWidget *w = new QWidget;
    vbox->addWidget( w );

    ui.setupUi(w);

    // ui.mFileUrl->setText(DatabaseSettings::self()->dbFile());
    registerField("DefaultSqliteStorage", ui.mRbDefault );
    registerField("SqliteStorageFile", ui._fileName);

    // Preset the sqlite storage
    if( ! DatabaseSettings::self()->dbFile().isEmpty()) {
        ui.mRbCustom->setChecked(true);
        setField("SqliteStorageFile", DatabaseSettings::self()->dbFile());
    }
}

QUrl SqLiteDetailsPage::url()
{
    QString fileName;
    if( ui.mRbDefault->isChecked() ) {
        fileName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    } else {
        fileName = ui._fileName->text();
    }
    if( ! fileName.endsWith("/")) fileName += QLatin1String("/");
    fileName += QLatin1String("kraft.db");

    return QUrl::fromLocalFile(fileName);
}

int SqLiteDetailsPage::nextId() const
{
    if( KraftDB::self()->databaseExists() ) {
        return SetupAssistant::upgradeDbPageNo;
    } else {
        return SetupAssistant::createDbPageNo;
    }
}

// ---------------------------------------------------------------------------

MysqlDetailsPage::MysqlDetailsPage(QWidget *parent)
  :QWizardPage(parent)
{
    setTitle(i18n("MySql Detail Information"));
    QVBoxLayout *vbox = new QVBoxLayout;
    setLayout( vbox );
    QWidget *w = new QWidget;
    vbox->addWidget( w );

    ui.setupUi(w);
    registerField("MySqlHost",   ui.mMysqlHost);
    registerField("MySqlUser",   ui.mMysqUser );
    registerField("MySqlDbName", ui.mMysqlDbName);
    registerField("MySqlPwd",    ui.mMysqlPwd );
    reloadSettings();
}

void MysqlDetailsPage::reloadSettings()
{
    setField("MySqlHost",   DatabaseSettings::self()->dbServerName());
    setField("MySqlUser",   DatabaseSettings::self()->dbUser());
    setField("MySqlDbName", DatabaseSettings::self()->dbDatabaseName());
    setField("MySqlPwd", DatabaseSettings::self()->dbPassword());
}

int MysqlDetailsPage::nextId() const
{
    if( KraftDB::self()->databaseExists() ) {
        return SetupAssistant::upgradeDbPageNo;
    } else {
        return SetupAssistant::createDbPageNo;
    }
}

// ---------------------------------------------------------------------------

CreateDbPage::CreateDbPage(QWidget *parent)
  :QWizardPage(parent)
{
    setTitle(i18n("Create Database"));
    QVBoxLayout *vbox = new QVBoxLayout;
    setLayout( vbox );

    QWidget *w = new QWidget;
    vbox->addWidget( w );

    ui.setupUi(w);

    registerField("CreateDbStatusText", ui.mCreateStatus);
}

void CreateDbPage::setStatusText( const QString& t )
{
    ui.mCreateStatus->setText( t );
}

void CreateDbPage::setFillCmdsCount( int cnt )
{
    mFills = 0;
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
    mCreates = 0;
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
    // qDebug () << "############### success: " << msg;
    ui.mCreateStatus->setText( msg );
}

void CreateDbPage::slotCountCreateProgress( bool res )
{
    if( res ) {
        mCreates++;
        ui.mCreateProgress->setValue( mCreates );
        ui.mCreateCounter->setText( i18n("%1/%2").arg(mCreates).arg( ui.mCreateProgress->maximum() ) );
    }
}

void CreateDbPage::slotCountFillProgress( bool res )
{
    if( res ) {
        mFills++;
        ui.mFillProgress->setValue( mFills );
        ui.mFillCounter->setText( i18n("%1/%2").arg(mFills).arg( ui.mFillProgress->maximum() ) );
    }
}

// ---------------------------------------------------------------------------

UpgradeDbPage::UpgradeDbPage(QWidget *parent)
  :QWizardPage(parent)
{
    setTitle(i18n("Upgrade the Database"));
    QVBoxLayout *vbox = new QVBoxLayout;
    setLayout( vbox );
    //TODO PORT QT5   vbox->setSpacing( QDialog::spacingHint() );
    //TODO PORT QT5   vbox->setMargin( QDialog::marginHint() );

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
    mUpgrades = 0;
    ui.mUpgradeProgress->setMaximum( cnt );
    ui.mUpgradeProgress->setValue( 0 );
    updateCounter();
}

void UpgradeDbPage::updateCounter()
{
    ui.mUpgradeCounter->setText( i18n("%1/%2").arg(mUpgrades).arg( ui.mUpgradeProgress->maximum() ));
}

void UpgradeDbPage::slotCountFillProgress( bool res )
{
    if( res ) {
        mUpgrades++;
        ui.mUpgradeProgress->setValue( mUpgrades );
        updateCounter();
    }
}


// ---------------------------------------------------------------------------

OwnAddressPage::OwnAddressPage(QWidget *parent)
  :QWizardPage(parent)
{
    setTitle(i18n("Your Own Identity"));
    QVBoxLayout *vbox = new QVBoxLayout;

    QLabel *l = new QLabel;
    l->setText( i18n("Select your own address from the address book. It is set as a consigner on the documents.") );
    vbox->addWidget( l );

    mAddresses = new AddressSelectorWidget(this);
    vbox->addWidget( mAddresses );
    this->setLayout( vbox );

    connect( mAddresses, SIGNAL( addressSelected(KContacts::Addressee)),
             SLOT( gotMyAddress( KContacts::Addressee ) ) );
}

OwnAddressPage::~OwnAddressPage()
{
    delete mAddresses;
}

void OwnAddressPage::gotMyAddress(const KContacts::Addressee& addressee)
{
    mMe = addressee;
}

void OwnAddressPage::saveOwnName()
{
    if( ! mMe.isEmpty() ) {
        KraftSettings::self()->setUserName( mMe.name() );
        KraftSettings::self()->setUserUid( mMe.uid() );
        KraftSettings::self()->writeConfig();
    }
}

// ---------------------------------------------------------------------------
FinalStatusPage::FinalStatusPage(QWidget *parent)
    :QWizardPage(parent)
{
    setTitle(i18n("Final Status"));
    QVBoxLayout *vbox = new QVBoxLayout;
    setLayout( vbox );
    //TODO PORT QT5   vbox->setSpacing( QDialog::spacingHint() );
    //TODO PORT QT5   vbox->setMargin( QDialog::marginHint() );

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
  :QWizard( parent ),
  mMode( Reinit )
{

    setPage( welcomePageNo,  new WelcomePage);
    setPage( dbSelectPageNo, new DbSelectPage);
    setPage( mySqlPageNo,    new MysqlDetailsPage);
    setPage( sqlitePageNo,   new SqLiteDetailsPage);
    setPage( createDbPageNo, new CreateDbPage);
    setPage( upgradeDbPageNo,   new UpgradeDbPage);
    setPage( ownAddressPageNo,  new OwnAddressPage);
    setPage( finalStatusPageNo, new FinalStatusPage);

    connect( this, SIGNAL( currentIdChanged( int) ),
             this, SLOT( slotCurrentPageChanged( int) ) );

    resize( QSize( 450, 260 ) );

}

/*
 * Current Database Setup Wizard        +----------------+    check if db already exists
   -----------------------------      > |  MySQL Page    ----------------------------------+
                                    -/  +--------+-------+                                 v
   +------------+   +--------------/             |            +----------------+    +---------------+
   |  Welcome   --->|  DB Select   |         +--->+---------> | create DB Page +--->|  upgrade DB   |
   +------------+   +--------------\         |                +----------------+    +-----------+---+
                                    -\   +---+------------+                                 ^   |
                                      >  |  SQLite Page   ----------------------------------+   |
                                         +----------------+                                     |
                                                                                                |
                                                               +---------------+    +-----------v---+
                                                               | Final Status  |<---+  Own Address  |
                                                               +---------------+    +---------------+

 */

QString SetupAssistant::defaultSqliteFilename() const
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QFileInfo fi(path, "kraft.db");

    if( !fi.dir().exists() ) {
        if (! fi.dir().mkpath(path) ) {
            qDebug() << "Failed to create directory "<< path << "for sqlite db";
            return QString();
        }
    }
    return fi.filePath();
}

void SetupAssistant::slotCurrentPageChanged( int currId )
{
    if( currId == dbSelectPageNo ) {
        handleDatabaseBackendSelect(); // does nothing currently
    } else if( currId == mySqlPageNo ) {
        // get the mysql datails
        handleMysqlDetails();
    } else if( currId == mySqlPageNo) {
        // get the sqlite filename
        handleSqLiteDetails();
    } else if( currId == createDbPageNo ) {
        if( mSqlBackendDriver == QLatin1String("QMYSQL") ) {
            if(!KraftDB::self()->dbConnect( QLatin1String("QMYSQL"),
                                            field("MySqlDbName").toString(),
                                            field("MySqlUser").toString(),
                                            field("MySqlHost").toString(),
                                            field("MySqlPwd").toString() ) ) {
                setField("CreateDbStatusText", i18n( "<p>Can't connect to your database. Are you sure your credentials are correct and the database exists?</p>") );
                return;
            }
        } else {
            QString filename = field("SqliteStorageFile").toString();
            if(filename.isEmpty()) {
                filename = defaultSqliteFilename();
                setField("SqliteStorageFile", filename);
            }
            if( !KraftDB::self()->dbConnect( QLatin1String("QSQLITE"), filename ) ) {
                setField("CreateDbStatusText", i18n("<p>Can't open your database file, check the permissions and such."));
            }
        }

        if( !KraftDB::self()->databaseExists() ) {
            // qDebug () << "Start to create the database";
            startDatabaseCreation();
        } else {
            // qDebug () << "CreateDB-Page: Database already existing";
            setField("CreateDbStatusText", i18n( "<p>The database is already existing, no action needs to be taken here.</p>"
                                                 "<p>Please hit <b>next</b> to proceed.</p>" ) );
        }
    }

    if( currId == upgradeDbPageNo ) {
        if( KraftDB::self()->databaseExists() ) {
            // qDebug () << "start to update the database";
            startDatabaseUpdate();
        } else {
            // qDebug () << "Strange problem at dbupdate: DB does not exist";
        }
    }

    if( currId == finalStatusPageNo )  {
        finalizePage();
    }
}

void SetupAssistant::done( int result )
{
    // store the stakeholders own name for picking the sender address
    qobject_cast<OwnAddressPage*>(page(ownAddressPageNo))->saveOwnName();

    const QString selectedDriver = qobject_cast<DbSelectPage*>(page(dbSelectPageNo))->selectedDriver();
    DatabaseSettings::self()->setDbDriver( selectedDriver );
    if( selectedDriver == QLatin1String("QSQLITE") ) {
        const QString file = field("SqliteStorageFile").toString();
        DatabaseSettings::self()->setDbFile(file);
    }
    if( selectedDriver == "QMYSQL" ) {
        DatabaseSettings::self()->setDbDatabaseName( field("MySqlDbName").toString() );
        DatabaseSettings::self()->setDbUser( field("MySqlUser").toString() );
        DatabaseSettings::self()->setDbServerName( field("MySqlHost").toString() );
        DatabaseSettings::self()->setDbPassword( field("MySqlPwd").toString() );
    }
    DatabaseSettings::self()->writeConfig();
    qDebug () << "Database backend config written";
    QWizard::done(result);
}

void SetupAssistant::finalizePage()
{
  QString txt;

  if( mErrors.isEmpty() ) {
    txt = i18n( "<p>The database setup was successfully completed.</p>" );
    txt += i18n("<p>You can start to work with Kraft now. Please do not forget to</p>");
    txt += "<ul>";
    txt += i18n("<li>adjust various settings in the Kraft Preferences dialog.</li>" );
    txt += i18n("<li>Check the Catalog chapter list.</li>" );
    txt += i18n("<li>Make your business and have fun.</li>" );
    txt += "</ul>";
    txt += i18n("<p>If you press <i>Finish</i> now, the new database configuration is stored in Krafts configuration.</p>");
  } else {
    foreach( QString err, mErrors ) {
      txt += "<p>" + err + "</p>";
    }
  }
  // qDebug() << "this is the status text: " << txt;
  qobject_cast<FinalStatusPage*>(page(finalStatusPageNo))->slotSetStatusText( txt );
}

void SetupAssistant::startDatabaseUpdate()
{
    CreateDbPage *mCreateDbPage = qobject_cast<CreateDbPage*>(page(createDbPageNo));
    UpgradeDbPage *mUpgradeDbPage = qobject_cast<UpgradeDbPage*>(page(upgradeDbPageNo));

    if( ! KraftDB::self()->isOk() ) {
        mCreateDbPage->setStatusText( i18n("The Database can not be connected. Please check the database credentials."));
        button(NextButton)->setEnabled(false);
        return;
    }

    if( !KraftDB::self()->databaseExists() ) {
        mCreateDbPage->setStatusText( i18n("The database core tables do not exist. Please check initial setup."));
        button(NextButton)->setEnabled(false);
        return;
    }
    button(NextButton)->setEnabled(true);

    if( KraftDB::self()->currentSchemaVersion() == KraftDB::self()->requiredSchemaVersion() ) {
        mUpgradeDbPage->slotSetStatusText( i18n("Database is up to date. No upgrade is required."));
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
        // qDebug () << "######### Reading " << migrateFilename;
        mUpgradeDbPage->slotSetStatusText( i18n("Reading upgrade command file %1").arg( migrateFilename ) );
        SqlCommandList cmds = KraftDB::self()->parseCommandFile( migrateFilename );
        overallCmdCount += cmds.count();
        commandLists << cmds;
    }
    mUpgradeDbPage->slotSetOverallCount( overallCmdCount );

    // qDebug () << "4.";
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
            // qDebug () << "Only performned " << goodCmds << " out of " << cmds.count();
            errors = true;
            break;
        } else {
            // qDebug () << goodCmds << " commands performed well!";
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
    CreateDbPage *mCreateDbPage = qobject_cast<CreateDbPage*>(page(createDbPageNo));
    UpgradeDbPage *mUpgradeDbPage = qobject_cast<UpgradeDbPage*>(page(upgradeDbPageNo));

    if( ! KraftDB::self()->isOk() ) {
        mCreateDbPage->setStatusText( i18n("The Database can not be connected. Please check the database credentials!"));
        button(NextButton)->setEnabled(false);
        return;
    }
    button(NextButton)->setEnabled(true);

    mCreateDbPage->setStatusText( i18n("Parse Create Commands...") );
    SqlCommandList createCommands = KraftDB::self()->parseCommandFile( "create_schema.sql");

    QString dbFill( "fill_schema_en.sql" );

    if ( DefaultProvider::self()->locale()->country() == QLocale::Germany	) {
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
        // qDebug () << "NOT all create commands succeeded!";
        res = false;
    } else {
        // qDebug () << creates << "(=All) create commands succeeded!";

        // lets do the fillup
        disconnect( KraftDB::self(), SIGNAL(processedSqlCommand(bool)),0,0 );

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
    disconnect( KraftDB::self(), SIGNAL(statusMessage( const QString&)),0 ,0 );
    disconnect( KraftDB::self(), SIGNAL(processedSqlCommand(bool)),0 ,0 );
}

void SetupAssistant::handleDatabaseBackendSelect() const
{
}

void SetupAssistant::handleSqLiteDetails()
{
    DbSelectPage *mDbSelectPage = qobject_cast<DbSelectPage*>(page(dbSelectPageNo));

    QString file = field("SqliteStorageFile").toString();

    qDebug () << "The SqlLite database file is " << file;

    mSqlBackendDriver = mDbSelectPage->selectedDriver();
    // qDebug () << "The database driver is " << mSqlBackendDriver;
    if( KraftDB::self()->dbConnect( mSqlBackendDriver, file ) ) {
        // FIXME error handling
    }
}

void SetupAssistant::handleMysqlDetails()
{
    DbSelectPage *mDbSelectPage = qobject_cast<DbSelectPage*>(page(dbSelectPageNo));
    mSqlBackendDriver = mDbSelectPage->selectedDriver();

    QString hostName = field("MySqlHost").toString();
    QString databaseName = field("MySqlDbName").toString();
    QString userName = field("MySqlUser").toString();
    QString password = field("MySqlPwd").toString();

    if( KraftDB::self()->dbConnect( mSqlBackendDriver, databaseName, userName, hostName, password ) ) {
        //  FIXME error handling
    }
}

bool SetupAssistant::init( Mode mode )
{
    bool startDialog = false;
    QString text;
    QString configOrigin;
    mMode = mode;

    text = i18n("This assistant guides you through the basic settings of your Kraft installation.");

    bool hitNextClosing = true;

    if( mMode == Reinit ) {
        startDialog = true;
    } else if( mode == Update ) {
        if( QStandardPaths::locate(QStandardPaths::GenericConfigLocation, "kraft/kraftdatabaserc" ).isEmpty() ) {
            // migration failed and we do not have a config file. All from scratch
            configOrigin = i18n("There was no database configuration found.");
        } else {
            configOrigin = i18n("A valid current database configuration file was found.");
            // qDebug () << "A standard KDE Platform 4.x database config file is there.";
        }

        if( KraftDB::self()->dbConnect() )  { // try to connect with default values
            // qDebug () << "The database can be opened!";
            if( KraftDB::self()->databaseExists() ) {
                // qDebug () << "The database exists.";

                if( KraftDB::self()->currentSchemaVersion() != KraftDB::self()->requiredSchemaVersion() ) {
                    // qDebug () << "Need a database schema update.";
                    startDialog = true;
                } else {
                    // qDebug () << "Database Schema is OK. Nothing to do for StartupAssistant";
                }
            } else {
                // qDebug () << "The database is not existing. It needs to be recreated.";
                startDialog = true;

                text = i18n( "<p>The database can be opened, but does not contain valid content.</p>"
                             "<p>A new database can be created automatically from scratch.</p>");
            }
        } else {
            // unable to connect to the database at all
            startDialog = true;
            hitNextClosing = false;
            text = i18n( "<p>Kraft could not connect to the configured database.<p>" );
            if( KraftDB::self()->qtDriver().toUpper() == "QMYSQL" ) {
                text += i18n( "<p>Please check the database server setup and restart Kraft to connect." );
            } else {
                text += i18n("<p>Please check the database file.");
            }
            text += " " + i18n( "or create a new database by hitting <b>next</b>.</p>" );
        }
    }

    if( startDialog ) {
        WelcomePage *welcomePage = qobject_cast<WelcomePage*>(page(welcomePageNo));

        if( hitNextClosing )
            text += "<p>Please hit next and follow the instructions.</p>";
        welcomePage->setWelcomeText( configOrigin + text );
    }
    return startDialog ;
}

SetupAssistant::~SetupAssistant()
{

}

