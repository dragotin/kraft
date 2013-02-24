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
    void slotCredentialsFetched(bool );

private:
    Folder *_syncFolder;
    QString _srcPath;
};

#endif // OWNCLOUDSYNC_H
