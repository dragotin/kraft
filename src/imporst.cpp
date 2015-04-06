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


#include <kaboutdata.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ksplashscreen.h>
#include <QDebug>
#include <QApplication>
#include <KAboutData>
#include <KLocalizedString>
#include <QCommandLineParser>

#include "importfilter.h"


static const char *description =
	I18N_NOOP("Kraft Import Test App");


static QCommandLineParser parser[] =
    QApplication app(argc, argv); // TODO: move this to before the KAboutData initialization
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    //PORTING SCRIPT: adapt aboutdata variable if necessary
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);
{
  // { "+[File]", I18N_NOOP("file to open"), 0 },
  { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[])
{

  KAboutData aboutData( "imporst", I18N_NOOP("Kraft import test app"),
                        "0.1", description, KAboutLicense::GPL,
                        "(c) 2008 Klaas Freitag", 0, 0, "freitag@kde.org");
  aboutData.addAuthor("Klaas Freitag",0, "freitag@kde.org");


  DocPositionImportFilter importer;
  if ( ! importer.readDefinition( "woerlein_txt.ftr" ) ) {
    // qDebug () << "Unable to import the definition!" << importer.error();
  }
  if ( ! importer.parseDefinition() ) {
    // qDebug () << "** Error in definition parsing: " << importer.error();
  }
  importer.debugDefinition();

  DocPositionList list = importer.import( "/tmp/pflanzliste.txt" );
  // qDebug () << "********* List of " << list.count() << " items imported";
  return app.exec();
}
