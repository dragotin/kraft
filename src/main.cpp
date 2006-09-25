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
#include <qpixmap.h>
#include <qbitmap.h>
#include <qimage.h>

#include <kstandarddirs.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <ksplashscreen.h>

#include "version.h"
#include "portal.h"

static const char *description =
I18N_NOOP("Kraft - Small business correspondence for the KDE desktop" );

static const char *otherText =
I18N_NOOP("Kraft is open source software for persons in small businesses\n"
          "writing correspondence like offers and invoices to their customers" );

static KCmdLineOptions options[] =
{
  { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[])
{
  KStandardDirs stdDirs;
  QString splashFile = stdDirs.findResource( "data", "kraft/pics/muckilogo_oS.png" );
  QImage logo( splashFile );

  KAboutData aboutData( "kraft", I18N_NOOP("Kraft"),
                        KRAFT_VERSION, description, KAboutData::License_GPL,
                        "(c) 2004-2006 Klaas Freitag", 0, 0, "freitag@kde.org");
  aboutData.addAuthor("Klaas Freitag", I18N_NOOP( "Developer" ), "freitag@kde.org");
  aboutData.addAuthor("Johannes Spielhagen", I18N_NOOP( "Graphics and Artwork" ),
                      "kraft@spielhagen.de", "http://www.michal-spielhagen.de" );
  aboutData.setProgramLogo( logo );
  aboutData.setOtherText( otherText );

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KApplication app;

  if (app.isRestored())
  {
    RESTORE(Portal);
  }
  else
  {

  KStandardDirs stdDirs;
  splashFile = stdDirs.findResource( "data", "kraft/pics/mucki_en_oS.png" );
  QImage img( splashFile );

  QPixmap pixmap;
    pixmap.convertFromImage( img );
    if ( !pixmap.mask() ) {
      QBitmap bm;
      if ( img.hasAlphaBuffer() ) {
        bm = img.createAlphaMask();
      } else {
        bm = img.createHeuristicMask();
      }
      pixmap.setMask( bm );
    } else {
      // kdDebug() << "Have a mask already!" << endl;
    }

    KSplashScreen *splash = new KSplashScreen( pixmap );
    splash->setMask( *pixmap.mask() );
    splash->show();

    Portal *kraftPortal = new Portal();
    kraftPortal->show();

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if (args->count())
    {

    }
    else
    {

    }
    args->clear();
    splash->finish( kraftPortal->mainWidget() );
    delete splash;
  }

  return app.exec();
}
