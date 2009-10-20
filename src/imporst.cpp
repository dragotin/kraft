/***************************************************************************
          imporst.cpp  -  main program for import testing
                             -------------------
    begin                : oct. 2008
    copyright            : (C) 2008 by Klaas Freitag
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
#include <kdebug.h>

#include "importfilter.h"
#include <kapplication.h>

static const char *description =
	I18N_NOOP("Kraft Import Test App");


static KCmdLineOptions options[] =
{
  // { "+[File]", I18N_NOOP("file to open"), 0 },
  { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[])
{

  KAboutData aboutData( "imporst", I18N_NOOP("Kraft import test app"),
                        "0.1", description, KAboutData::License_GPL,
                        "(c) 2008 Klaas Freitag", 0, 0, "freitag@kde.org");
  aboutData.addAuthor("Klaas Freitag",0, "freitag@kde.org");
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KApplication app;

  DocPositionImportFilter importer;
  if ( ! importer.readDefinition( "woerlein_txt.ftr" ) ) {
    kDebug() << "Unable to import the definition!" << importer.error();
  }
  if ( ! importer.parseDefinition() ) {
    kDebug() << "** Error in definition parsing: " << importer.error();
  }
  importer.debugDefinition();

  DocPositionList list = importer.import( "/tmp/pflanzliste.txt" );
  kDebug() << "********* List of " << list.count() << " items imported";
  return app.exec();
}
