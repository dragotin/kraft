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

using namespace Mirall;

class ownCloudSync : public QObject
{
    Q_OBJECT
public:
    explicit ownCloudSync(QObject *parent = 0);
    
    bool startSync( const QString& path );

signals:
    
public slots:

private slots:
    void slotSyncFinished( const SyncResult& );
    void slotCredentialsFetched();

private:
    Folder *_syncFolder;
    QString _srcPath;
};

#endif // OWNCLOUDSYNC_H
