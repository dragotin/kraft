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

#include <kstandarddirs.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <ksplashscreen.h>

#include "version.h"
#include "portal.h"

static const char *description =
	I18N_NOOP("Kraft helps craftsmen and other small"
                  "business people in their daily office work" );
	
	
static KCmdLineOptions options[] =
{
  { "+[File]", I18N_NOOP("file to open"), 0 },
  { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[])
{

	KAboutData aboutData( "kraft", I18N_NOOP("Kraft"),
		KRAFT_VERSION, description, KAboutData::License_GPL,
		"(c) 2004-2006 Klaas Freitag", 0, 0, "freitag@kde.org");
	aboutData.addAuthor("Klaas Freitag",0, "freitag@kde.org");
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
    QString splashFile = stdDirs.findResource( "data", "kraft/pics/kraft_splash.png" );
    QPixmap pixmap( splashFile );
    KSplashScreen *splash = new KSplashScreen( pixmap );
    splash->show();

    Portal *kange = new Portal();
    kange->show();

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
		
		if (args->count())
		{
      
		}
		else
		{
		  
		}
		args->clear();
    splash->finish( kange->mainWidget() );
    delete splash;
  }

  return app.exec();
}  
