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

#include <kstandarddirs.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <ksplashscreen.h>
#include <kdebug.h>

#include "version.h"
#include "portal.h"

int main(int argc, char *argv[])
{
  KAboutData aboutData( "kraft", "kraft", ki18n("Kraft"),
                        KRAFT_VERSION,
                        ki18n("Business documents for the small enterprise"),
                        KAboutData::License_GPL,
                        ki18n("Copyright © 2004–2014 Klaas Freitag" ) );

  aboutData.addAuthor(ki18n("Klaas Freitag"), ki18n( "Developer" ), "freitag@kde.org");
  aboutData.addAuthor(ki18n("Johannes Spielhagen"), ki18n( "Graphics and Artwork" ),
                        "kraft@spielhagen.de", "http://www.michal-spielhagen.de" );
  aboutData.addAuthor(ki18n("Thomas Richard"), ki18n("Developer"), "thomas.richard@proan.be");

  aboutData.setBugAddress( "http://sourceforge.net/p/kraft/bugs/" );
  KStandardDirs stdDirs;
  QString logoFile = stdDirs.findResource( "data",  "kraft/pics/kraftapp_logo.png" );
  if( ! logoFile.isEmpty() ) {
    QImage img( logoFile );
    aboutData.setProgramLogo( QVariant( img ) );
  }
  aboutData.setOtherText( ki18n("Kraft is free software for persons in small businesses\n"
          "writing correspondence like offers and invoices to their customers" ) );

  aboutData.setVersion( KRAFT_VERSION );
  aboutData.setHomepage( "http://www.volle-kraft-voraus.de" );

  KCmdLineArgs::init( argc, argv, &aboutData );


  KCmdLineOptions options;
  options.add( "d <number>", ki18n("Open document with doc number <number>") );

   // Register the supported options
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app;

  if (app.isSessionRestored())
  {
    RESTORE(Portal);
  } else {
    KStandardDirs stdDirs;
    QString splashFile = stdDirs.findResource( "data", "kraft/pics/kraftsplash.png" );
    KSplashScreen *splash = 0;

    if( !splashFile.isEmpty()) {
      QPixmap pixmap( splashFile );

      splash = new KSplashScreen( pixmap, Qt::WindowStaysOnTopHint );
      splash->setMask(pixmap.mask());
      splash->show();
    }

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    Portal *kraftPortal = new Portal( 0, args, "kraft main window" );
    kraftPortal->show();

    if( splash ) {
      splash->finish( kraftPortal->mainWidget() );
      splash->deleteLater();
    } else {
      kDebug() << "Could not find splash screen";
    }
  }

  return app.exec();
}
