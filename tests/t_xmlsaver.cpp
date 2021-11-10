#include <QTest>

#include <QTemporaryDir>


#include "documentsaverxml.h"

class T_XmlSaver: public QObject {
    Q_OBJECT
private slots:
    void initTestCase()
    {
        if (_dir.isValid()) {
            // dir.path() returns the unique directory path
            QDir d {_dir.path()};
            Q_ASSERT(d.mkpath(d.path()+"/2020/1"));
        }
    }

    void tmpPath()
    {
        qDebug() << "Path: " << _dir.path();
    }
private:
    QString _docTypeName;

    QTemporaryDir _dir;

};

QTEST_MAIN(T_XmlSaver)
#include "t_xmlsaver.moc"

