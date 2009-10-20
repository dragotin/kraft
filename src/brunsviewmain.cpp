/***************************************************************************
          brunsviewmain.cpp  -  main program for the plant viewer
                             -------------------
    begin                : nov. 2005
    copyright            : (C) 2005 by Klaas Freitag
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

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ksplashscreen.h>

#include "version.h"
#include "brunsviewer.h"
#include "brunskatalogview.h"

//Added by qt3to4:
#include <QPixmap>

#if 0
static const char *description =
	I18N_NOOP("KDE Plant Catalog Viewer");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE
	
	
static KCmdLineOptions options[] =
{
  // { "+[File]", I18N_NOOP("file to open"), 0 },
  { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};
#endif

int main(int argc, char *argv[])
{
  KAboutData aboutData( "kplant", "kplant",
                        ki18n("KDE Plant Catalog Viewer"), KPLANT_VERSION,
                        ki18n("The KDE Plant Catalog Viewer"),
                        KAboutData::License_GPL,
                        ki18n("Copyright (c) 2005-2009 Klaas Freitag") );
  KCmdLineArgs::init( argc, argv, &aboutData );

  KApplication app;

#if 0

	KAboutData aboutData( "kplant", I18N_NOOP("KDE Plant Catalog Viewer"),
		KPLANT_VERSION, description, KAboutData::License_GPL,
		"(c) 2005 Klaas Freitag", 0, 0, "freitag@kde.org");
	aboutData.addAuthor("Klaas Freitag",0, "freitag@kde.org");
	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KApplication app;
  KStandardDirs stdDirs;
  QString splashFile = stdDirs.findResource( "data", "kplant/pics/kplant_splash.png" );
  KSplashScreen *splash = 0;
  
  if( ! splashFile.isNull() ) {
    QPixmap pixmap( splashFile );
    splash = new KSplashScreen( pixmap );
    splash->show();
  }

  BrunsKatalogView *lv = 0;
  
  if (app.isSessionRestored())
  {
    RESTORE( Brunsviewer );
  }
  else 
  {
#endif
      const QString kat = i18n("BRUNS Pflanzenkatalog 2005");
      Brunsviewer *brunsView = new Brunsviewer();

      brunsView->show();
#if 0
      BrunsKatalogView *lv = new BrunsKatalogView( );
      lv->init( kat );
      lv->show();
#endif
#if 0
  if( splash ) {
    splash->finish( lv );
    delete splash;
  }
#endif
  return app.exec();
}  
