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
<<<<<<< HEAD
=======
#include "creds/abstractcredentials.h"

>>>>>>> Work with more recent ocsync and mirall
#include "mirall/mirallconfigfile.h"

#include <QApplication>

#include <KDebug>

ownCloudSync::ownCloudSync(QObject *parent) :
    QObject(parent),
    _syncFolder(0)
{
    qApp->setApplicationName( QLatin1String("ownCloud")); // FIXME!
}

bool ownCloudSync::startSync( const QString& path )
{
    _srcPath = path;
    connect( CredentialStore::instance(), SIGNAL(fetchCredentialsFinished(bool)),
             SLOT(slotCredentialsFetched(bool)));

    CredentialStore::instance()->fetchCredentials();
    return true;
}

void ownCloudSync::slotCredentialsFetched()
{
    if( _srcPath.isEmpty() ) {
        kDebug() << "No src-path given!";
        return;
    }

    MirallConfigFile cfg;
    AbstractCredentials* credentials(cfg.getCredentials());

    disconnect(credentials, SIGNAL(fetched()),
               this, SLOT(slotCredentialsFetched()));

    qDebug() << "Successfully fetched credentials!";

    QString oCUrl = cfg.ownCloudUrl();

    QString kraftPath("kraft");
    _syncFolder = new Folder(QLatin1String("KraftFolder"), _srcPath, oCUrl+kraftPath );

    connect(_syncFolder, SIGNAL(syncFinished(SyncResult)),
            SLOT(slotSyncFinished(SyncResult)));

    _syncFolder->startSync( QStringList() );

}

bool ownCloudSync::startSync( const QString& path )
{
    MirallConfigFile cfg;
    AbstractCredentials* credentials(cfg.getCredentials());
    _srcPath = path;

    if (! credentials->ready()) {
       connect( credentials, SIGNAL(fetched()),
                this, SLOT(slotCredentialsFetched()));
       credentials->fetch();
    } else {
	// Credentials are here already.
    }
    return true;
}

void ownCloudSync::slotSyncFinished( const SyncResult& result )
{
    kDebug() << " *** ownCloud Sync-Result: " << result.statusString();
    qApp->setApplicationName( QLatin1String("Kraft"));
    if( result.status() == SyncResult::Success ) {

    }

    _syncFolder->deleteLater();
}
