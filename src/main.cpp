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

#include <klocalizedstring.h>
#include <QDebug>
#include <QApplication>
#include <KAboutData>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QStandardPaths>
#include <QSplashScreen>

#include "version.h"
#include "portal.h"
#include "defaultprovider.h"

int main(int argc, char *argv[])
{
    KAboutData aboutData( QLatin1String("kraft"),
                          QLatin1String("Kraft"),
                          KRAFT_VERSION,
                          ki18n("Business documents for the small enterprise").toString(),
                          KAboutLicense::GPL,
                          ki18n("Copyright © 2004–2017 Klaas Freitag" ).toString() );

    aboutData.addAuthor(QLatin1String("Klaas Freitag"), ki18n("Developer").toString(), QLatin1String("kraft@freisturz.de"));
    aboutData.addAuthor(QLatin1String("Johannes Spielhagen"), ki18n( "Graphics and Artwork" ).toString(),
                        QLatin1String("kraft@spielhagen.de"), QLatin1String("http://www.michal-spielhagen.de") );
    aboutData.addAuthor(QLatin1String("Thomas Richard"), ki18n("Developer").toString(), QLatin1String("thomas.richard@proan.be"));

    aboutData.setBugAddress( "http://sourceforge.net/p/kraft/bugs/" );

    KLocalizedString::setApplicationDomain("kraft");
    Q_INIT_RESOURCE(kraft);

    QString logoFile = DefaultProvider::self()->locateFile( "pics/kraftapp_logo.png" );
    if( ! logoFile.isEmpty() ) {
        QImage img( logoFile );
        aboutData.setProgramLogo( QVariant( img ) );
    }
    aboutData.setOtherText( QLatin1String("Kraft is free software for persons in small businesses\nwriting correspondence like offers and invoices to their customers" ) );

    aboutData.setVersion( KRAFT_VERSION );
    aboutData.setHomepage( "http://www.volle-kraft-voraus.de" );

    QApplication app(argc, argv);
    QCommandLineParser parser;

    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);


    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("d"), i18n("Open document with doc number <number>"), QLatin1String("number")));

    // Register the supported options
    if (app.isSessionRestored()) {
        RESTORE(Portal);
    } else {
        QString splashFile = DefaultProvider::self()->locateFile("pics/kraftsplash.png" );
        QSplashScreen *splash = 0;

        if( !splashFile.isEmpty()) {
            QPixmap pixmap( splashFile );

            splash = new QSplashScreen( pixmap, Qt::WindowStaysOnTopHint );
            splash->setMask(pixmap.mask());
            splash->show();
        }

        Portal *kraftPortal = new Portal( 0, &parser, "kraft main window" );
        kraftPortal->show();

        if( splash ) {
            splash->finish( kraftPortal );
            splash->deleteLater();
        } else {
            // qDebug () << "Could not find splash screen";
        }
    }

    return app.exec();
}
