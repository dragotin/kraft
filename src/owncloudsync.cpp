/***************************************************************************
 *                    owncloudync - sync to ownCloud 
 *                            -------------------
 *    begin                : feb 20, 2013
 *    copyright            : (C) 2013 by Klaas Freitag
 *    email                : freitag@kde.org
 ****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ****************************************************************************/
#include "owncloudsync.h"
#include "mirall/folder.h"
#include "creds/abstractcredentials.h"
#include "mirall/mirallconfigfile.h"
#include "mirall/connectionvalidator.h"
#include "mirall/account.h"
#include "mirall/accountsettings.h"
#include "mirall/folderman.h"

#include <KDebug>

ownCloudSync::ownCloudSync(QObject *parent) :
    QObject(parent),
    _syncFolder(0)
{
   //  qApp->setownCloudSyncName( QLatin1String("ownCloud")); // FIXME!

    Account *account = Account::restore();
    if (account) {
        // account->setSslErrorHandler(new SslDialogErrorHandler);
        AccountManager::instance()->setAccount(account);
    }
}

void ownCloudSync::setSyncDir( const QString& path )
{
    _syncDir = path;
}

void ownCloudSync::slotCheckConnection()
{
    Account *account = AccountManager::instance()->account();
    if( account ) {
        AbstractCredentials* credentials(account->credentials());

        if (! credentials->ready()) {
            connect( credentials, SIGNAL(fetched()),
                     this, SLOT(slotCredentialsFetched()));
            credentials->fetch(account);
        } else {
            runValidator();
        }
    } else {
        // the call to checkAccountsExists opens the setup wizard
        // if the config does not exist. Nothing to do here.
    }
}

void ownCloudSync::slotCredentialsFetched()
{
    Account *account = AccountManager::instance()->account();
    disconnect(account->credentials(), SIGNAL(fetched()),
               this, SLOT(slotCredentialsFetched()));
    runValidator();
}

void ownCloudSync::runValidator()
{
    _conValidator = new ConnectionValidator(AccountManager::instance()->account());
    connect( _conValidator, SIGNAL(connectionResult(ConnectionValidator::Status)),
             this, SLOT(slotConnectionValidatorResult(ConnectionValidator::Status)) );
    _conValidator->checkConnection();
}

void ownCloudSync::slotConnectionValidatorResult(ConnectionValidator::Status status)
{
    qDebug() << "Connection Validator Result: " << _conValidator->statusString(status);
    QStringList startupFails;

    if( status == ConnectionValidator::Connected ) {
        FolderMan *folderMan = FolderMan::instance();
        qDebug() << "######## Connection and Credentials are ok!";

        _syncFolder = folderMan->addFolder( QLatin1String("Kraft-Folder"), _syncDir, "kraft" );

        folderMan->setSyncEnabled(true);
        // queue up the sync for all folders.
        folderMan->slotScheduleAllFolders();

    } else {
        // if we have problems here, it's unlikely that syncing will work.
        FolderMan::instance()->setSyncEnabled(false);

        startupFails = _conValidator->errors();
        _startupNetworkError = _conValidator->networkError();
        if (_userTriggeredConnect) {
#if 0
            if(_connectionMsgBox.isNull()) {
                _connectionMsgBox = new QMessageBox(QMessageBox::Warning, tr("Connection failed"),
                                      _conValidator->errors().join(". ").append('.'), QMessageBox::Ok, 0);
                _connectionMsgBox->setAttribute(Qt::WA_DeleteOnClose);
                _connectionMsgBox->open();
                _userTriggeredConnect = false;
            }
#endif
        }
        QTimer::singleShot(30*1000, this, SLOT(slotCheckConnection()));
    }

    // FIXME: report startup errors stored in startupFails...

    _conValidator->deleteLater();
}

bool ownCloudSync::startSync( const QString& path )
{
    _syncFolder->startSync();
    return true;
}

#if 0
void ownCloudSync::slotSyncStarted()
{

}

void ownCloudSync::slotSyncFinished( const SyncResult& )
{

}
#endif
