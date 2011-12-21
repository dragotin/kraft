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

#include <QPixmap>

int main(int argc, char *argv[])
{
  KAboutData aboutData( "kplant", "kplant",
                        ki18n("KDE Plant Catalog Viewer"), KPLANT_VERSION,
                        ki18n("The KDE Plant Catalog Viewer"),
                        KAboutData::License_GPL,
                        ki18n("Copyright © 2005–2010 Klaas Freitag") );
  KCmdLineArgs::init( argc, argv, &aboutData );

  KApplication app;

      Brunsviewer *brunsView = new Brunsviewer();

      brunsView->show();
  return app.exec();
}
