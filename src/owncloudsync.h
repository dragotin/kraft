/***************************************************************************
 *                    owncloudync - sync to ownCloud 
 *                            -------------------
 *    begin                : feb 20, 2013
 *    copyright            : (C) 2013 by Klaas Freitag
 *    email                : freitag@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef OWNCLOUDSYNC_H
#define OWNCLOUDSYNC_H

#include <QObject>
#include <mirall/folder.h>
#include <mirall/connectionvalidator.h>

using namespace Mirall;

class ownCloudSync : public QObject
{
    Q_OBJECT
public:
    explicit ownCloudSync(QObject *parent = 0);
    
    bool startSync( const QString& path );

    void setSyncDir( const QString& );
signals:
    
public slots:

private slots:
    void slotCheckConnection();
    void slotCredentialsFetched();
    void runValidator();
    void slotConnectionValidatorResult(ConnectionValidator::Status status);

    // void slotSyncStarted();
    // void slotSyncFinished( const SyncResult& );

private:
    Folder *_syncFolder;
    QString _syncDir;
    ConnectionValidator *_conValidator;
    bool _startupNetworkError;
    bool _userTriggeredConnect;
};

#endif // OWNCLOUDSYNC_H
