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
#include <QTabWidget>

#include <kcontacts/address.h>
#include <kcontacts/vcardconverter.h>

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
    const QString fileName = DatabaseSettings::self()->dbFile();
    if( ! fileName.isEmpty()) {
        ui.mRbCustom->setChecked(true);
        setField("SqliteStorageFile", fileName);
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

bool SqLiteDetailsPage::validatePage()
{
    return qobject_cast<SetupAssistant*>(wizard())->handleSqLiteDetails();
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

bool MysqlDetailsPage::validatePage()
{
    bool re = qobject_cast<SetupAssistant*>(wizard())->handleMysqlDetails();
    return re;
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
    ui.mFillCounter->setText( i18n("0/%1", cnt) );
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
    ui.mCreateCounter->setText( i18n("0/%1", cnt));
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
        ui.mCreateCounter->setText( i18n("%1/%2", mCreates,
                                         ui.mCreateProgress->maximum() ) );
    }
}

void CreateDbPage::slotCountFillProgress( bool res )
{
    if( res ) {
        mFills++;
        ui.mFillProgress->setValue( mFills );
        ui.mFillCounter->setText( i18n("%1/%2", mFills, ui.mFillProgress->maximum() ) );
    }
}

int CreateDbPage::nextId() const
{
    return SetupAssistant::upgradeDbPageNo;
}
// ---------------------------------------------------------------------------

UpgradeDbPage::UpgradeDbPage(QWidget *parent)
  :QWizardPage(parent)
{
    setTitle(i18n("Upgrade the Database"));
    QVBoxLayout *vbox = new QVBoxLayout;
    setLayout( vbox );

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
    ui.mUpgradeCounter->setText( i18n("%1/%2", mUpgrades,
                                      ui.mUpgradeProgress->maximum() ));
}

void UpgradeDbPage::slotCountFillProgress( bool res )
{
    if( res ) {
        mUpgrades++;
        ui.mUpgradeProgress->setValue( mUpgrades );
        updateCounter();
    }
}

int UpgradeDbPage::nextId() const
{
    return SetupAssistant::ownAddressPageNo;
}

// ---------------------------------------------------------------------------

OwnAddressPage::OwnAddressPage(QWidget *parent)
  :QWizardPage(parent)
{
    setTitle(i18n("Your Company Address"));
    QVBoxLayout *vbox = new QVBoxLayout;
    QTabWidget *tabWidget = new QTabWidget;

    QLabel *l = new QLabel;
    l->setText( i18n("Select your companies address either from the address book or enter it manually. It is set as a consigner on the documents.") );
    vbox->addWidget( l );

    vbox->addWidget(tabWidget);

    // == The AddressSelector page
    QWidget *w = new QWidget;
    tabWidget->addTab(w, i18n("Select from Addressbook"));

    QVBoxLayout *vbox1 = new QVBoxLayout;

    mAddresses = new AddressSelectorWidget(this);
    vbox1->addWidget( mAddresses );
    w->setLayout( vbox1 );
    setLayout(vbox);

    connect( mAddresses, SIGNAL( addressSelected(KContacts::Addressee)),
             SLOT( gotMyAddress( KContacts::Addressee ) ) );

    // == The manual page
    QWidget *w1 = new QWidget;
    ui.setupUi(w1);
    int id = tabWidget->addTab(w1, i18n("Manual Entry"));
    ui.nameLabel->setText( KContacts::Addressee::formattedNameLabel() );
    ui.orgLabel->setText( KContacts::Addressee::organizationLabel());
    ui.streetLabel->setText(KContacts::Addressee::businessAddressStreetLabel());
    ui.postCodeLabel->setText(KContacts::Addressee::businessAddressPostalCodeLabel());
    ui.cityLabel->setText(KContacts::Addressee::businessAddressLocalityLabel());
    ui.phoneLabel->setText(KContacts::Addressee::businessPhoneLabel());
    ui.faxLabel->setText(KContacts::Addressee::businessFaxLabel());
    ui.mobileLabel->setText(KContacts::Addressee::mobilePhoneLabel());
    ui.emailLabel->setText(KContacts::Addressee::emailLabel());
    ui.websiteLabel->setText(KContacts::Addressee::urlLabel());

    if( !mAddresses->backendUp() ) {
        tabWidget->setCurrentIndex(id);
    }
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
        KraftSettings::self()->save();
    } else {
        // check for the manual.
        KContacts::Addressee add;
        add.setFormattedName(ui.leName->text());
        add.setOrganization(ui.leOrganization->text());
        KContacts::Address workAddress;

        workAddress.setStreet(ui.leStreet->text());
        workAddress.setPostalCode(ui.lePostcode->text());
        workAddress.setLocality(ui.leCity->text());
        workAddress.setType(KContacts::Address::Work);
        add.insertAddress(workAddress);

        add.insertPhoneNumber(PhoneNumber(ui.lePhone->text(), KContacts::PhoneNumber::Work));
        add.insertPhoneNumber(PhoneNumber(ui.leFax->text(), KContacts::PhoneNumber::Fax));
        add.insertPhoneNumber(PhoneNumber(ui.leMobile->text(), KContacts::PhoneNumber::Cell));
        ResourceLocatorUrl resUrl;
        resUrl.setUrl(QUrl(ui.leWebsite->text()));
        add.setUrl(resUrl);
        add.insertEmail(ui.leEmail->text(), true /* prefered */ );

        VCardConverter vcc;
        QByteArray vcard = vcc.createVCard(add);

        QString file = QStandardPaths::writableLocation( QStandardPaths::AppDataLocation );
        file += "/myidentity.vcd";
        QFile f ( file );
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            f.write(vcard);
            f.close();
            qDebug() << "Saved own identity to " << file;
        }
    }
}

int OwnAddressPage::nextId() const
{
    return SetupAssistant::finalStatusPageNo;
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

int FinalStatusPage::nextId() const
{
    return -1; // final page
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
    qDebug() << "Page changed to " << currId;

    if( currId == dbSelectPageNo ) {
    } else if( currId == mySqlPageNo ) {
        // TODO: set the mysql datails
    } else if( currId == sqlitePageNo) {
        // TODO set the sqlite filename
    } else if( currId == createDbPageNo ) {
        if( mSqlBackendDriver == QLatin1String("QMYSQL") ) {
            const QString dbName = field("MySqlDbName").toString();
            if(!KraftDB::self()->dbConnect( QLatin1String("QMYSQL"),
                                            dbName,
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
            if( !KraftDB::self()->dbConnect( QLatin1String("QSQLITE"), filename,
                                             QString(), QString(), QString()) ) {
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
    if( result > 0 ) {
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
            const QString host = field("MySqlHost").toString();
            DatabaseSettings::self()->setDbServerName( host );
            DatabaseSettings::self()->setDbPassword( field("MySqlPwd").toString() );
        }
        DatabaseSettings::self()->save();
        qDebug () << "Database backend config written";
    }
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
        // qDebug () << "######### Reading " << migrateFilename;
        const SqlCommandList cmds = KraftDB::self()->parseCommandFile( currentVer );
        commandLists.append(cmds);
        overallCmdCount += cmds.count();
        qDebug() << "Appending" << cmds.count() << "commands for version" << currentVer;
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
    for( SqlCommandList cmdList : commandLists ) {
        currentVer++;
        int goodCmds = KraftDB::self()->processSqlCommands( cmdList );
        doneOverallCmds += goodCmds;
        if( goodCmds != cmdList.count() ) {
            qDebug () << "Only performned " << goodCmds << " out of " << cmdList.count();
            errors = true;
            break;
        } else {
            qDebug () << goodCmds << " commands performed well, version is " << currentVer << ", listversion:" << cmdList.number();
            KraftDB::self()->setSchemaVersion( QString::number( currentVer ));
        }
    }

    if( errors ) {
        mUpgradeDbPage->slotSetStatusText( i18n("The Upgrade failed!") );;
    } else {
        mUpgradeDbPage->slotSetStatusText( i18n("The Upgrade succeeded, the current schema version is %1!",
                                           KraftDB::self()->requiredSchemaVersion() ) );;
    }

    disconnect( mUpgradeDbPage, SLOT( slotSetStatusText( const QString& )));
}


void SetupAssistant::startDatabaseCreation()
{
    CreateDbPage *mCreateDbPage = qobject_cast<CreateDbPage*>(page(createDbPageNo));

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

        if( creates != fillCommands.count() ) {
            qDebug() << "Could not execute all fill commands";
            res = false;
        }
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

bool SetupAssistant::handleSqLiteDetails()
{
    DbSelectPage *mDbSelectPage = qobject_cast<DbSelectPage*>(page(dbSelectPageNo));

    QString file = field("SqliteStorageFile").toString();

    qDebug () << "The SqlLite database file is " << file;

    mSqlBackendDriver = mDbSelectPage->selectedDriver();
    // qDebug () << "The database driver is " << mSqlBackendDriver;
    bool re = KraftDB::self()->dbConnect( mSqlBackendDriver, file,
                                          QString(), QString(), QString() );

    return re;
}

bool SetupAssistant::handleMysqlDetails()
{
    DbSelectPage *mDbSelectPage = qobject_cast<DbSelectPage*>(page(dbSelectPageNo));
    mSqlBackendDriver = mDbSelectPage->selectedDriver();

    QString hostName = field("MySqlHost").toString();
    QString databaseName = field("MySqlDbName").toString();
    QString userName = field("MySqlUser").toString();
    QString password = field("MySqlPwd").toString();

    return KraftDB::self()->dbConnect( mSqlBackendDriver, databaseName, userName, hostName, password );
}

bool SetupAssistant::init( Mode mode )
{
    bool startDialog = false;
    QString text;
    QString configOrigin;
    mMode = mode;

    text = QLatin1String("<p>")
            + i18n("This assistant guides you through the basic settings of your Kraft installation.")
            + QLatin1String("</p>");

    bool hitNextClosing = true;

    if( mMode == Reinit ) {
        startDialog = true;
    } else if( mode == Update ) {
        if( QStandardPaths::locate(QStandardPaths::GenericConfigLocation, "kraftdatabaserc" ).isEmpty() ) {
            // migration failed and we do not have a config file. All from scratch
            configOrigin = i18n("There was no database configuration found.");
        } else {
            configOrigin = i18n("A valid current database configuration file was found.");
            // qDebug () << "A standard KDE Platform 4.x database config file is there.";
        }

        const QString dbDriver = DatabaseSettings::self()->dbDriver().toUpper();
        QString dbName = DatabaseSettings::self()->dbDatabaseName();
        if( dbDriver == QLatin1String("QSQLITE")) {
            dbName = DatabaseSettings::self()->dbFile();
        }

        if( KraftDB::self()->dbConnect( dbDriver,
                                        dbName,
                                        DatabaseSettings::self()->dbUser(),
                                        DatabaseSettings::self()->dbServerName(),
                                        DatabaseSettings::self()->dbPassword()) )  { // try to connect with default values
            // qDebug () << "The database can be opened!";
            if( KraftDB::self()->databaseExists() ) {
                // qDebug () << "The database exists.";

                if( KraftDB::self()->currentSchemaVersion() < KraftDB::self()->requiredSchemaVersion() ) {
                    // qDebug () << "Need a database schema update.";
                    startDialog = true;
                    configOrigin += QLatin1String(" ") + i18n("The database schema version is too low. "
                                                              "It will be updated.");
                } else if( KraftDB::self()->currentSchemaVersion() > KraftDB::self()->requiredSchemaVersion() ) {
                    configOrigin += QLatin1Char(' ') + i18n("The current database schema version is too high. Leaving untouched! ");
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
            text = i18n( "<p>Kraft failed to connect to the configured database.</p>" );
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
            text += i18n("<p>Please hit next and follow the instructions.</p>");
        welcomePage->setWelcomeText( configOrigin + text );
    }
    return startDialog ;
}

SetupAssistant::~SetupAssistant()
{

}

