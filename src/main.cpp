/***************************************************************************
                          main.cpp  -
                             -------------------
    begin                : Mit Dez 31 19:24:05 CET 2003
    copyright            : (C) 2003 by Klaas Freitag
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
#include <QPixmap>
#include <QBitmap>
#include <QImage>
#include <QPalette>

#include <QDebug>
#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QStandardPaths>
#include <QSplashScreen>
#include <QScopedPointer>

#include <KLocalizedString>

#include "version.h"
#include "portal.h"
#include "defaultprovider.h"

int main(int argc, char *argv[])
{
    KLocalizedString::setApplicationDomain("kraft");
    Q_INIT_RESOURCE(kraft);

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/kraft/global/32-apps-kraft.png"));
    app.setApplicationName("kraft");

    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("d"), i18n("Open document with doc number <number>"), QLatin1String("number")));
    parser.process(app);

    // Register the supported options
    QScopedPointer<Portal> kraftPortal;
    if (app.isSessionRestored()) {
        // FIXME: Add session management
        // RESTORE(Portal);
    } else {
        QString splashFile = DefaultProvider::self()->locateFile("pics/kraftsplash.png" );
        QSplashScreen *splash = nullptr;

        if( !splashFile.isEmpty()) {
            QPixmap pixmap( splashFile );

            splash = new QSplashScreen( pixmap, Qt::WindowStaysOnTopHint );
            splash->setMask(pixmap.mask());
            splash->show();
        }

        kraftPortal.reset( new Portal( nullptr, &parser, "kraft main window" ));
        kraftPortal->show();

        if( splash ) {
            splash->finish( kraftPortal.data() );
            splash->deleteLater();
        } else {
            // qDebug () << "Could not find splash screen";
        }
    }

    return app.exec();
}
