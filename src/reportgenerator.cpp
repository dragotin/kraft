/***************************************************************************
                       archiveman.cpp  - Archive Manager
                             -------------------
    begin                : July 2006
    copyright            : (C) 2006 by Klaas Freitag
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
#include <qsqlcursor.h>
#include <qsqlrecord.h>
#include <qsqlindex.h>

#include <kstaticdeleter.h>
#include <kdebug.h>
#include <kprocess.h>

#include "reportgenerator.h"
#include "kraftdoc.h"
#include "kraftdb.h"
#include "unitmanager.h"
#include "dbids.h"
#include "kraftsettings.h"
#include "katalogsettings.h"

static KStaticDeleter<ReportGenerator> selfDeleter;

ReportGenerator* ReportGenerator::mSelf = 0;
KProcess* ReportGenerator::mProcess = 0;

ReportGenerator *ReportGenerator::self()
{
  if ( !mSelf ) {
    selfDeleter.setObject( mSelf, new ReportGenerator() );
  }
  return mSelf;
}

ReportGenerator::ReportGenerator()
{

}

ReportGenerator::~ReportGenerator()
{
  kdDebug() << "ReportGen is destroyed!" << endl;
}

void ReportGenerator::docPreview( const dbID& dbId )
{
    if( ! mProcess ) {
	mProcess = new KProcess;
	connect( mProcess, SIGNAL( processExited(  KProcess * ) ),
		 this,     SLOT( slotViewerClosed( KProcess * ) ) );
    } else {
      mProcess->clearArguments();
    }

    const QString ncbin = KraftSettings::nCReportBinary();
    kdDebug() << "Setting ncreport binary: " << ncbin << endl;

    const QString reportFile = "/home/kf/office/kraft/reports/invoice.xml";
    dbID id( dbId );

    if (  ! ncbin.isEmpty() ) {
      *mProcess << ncbin;
      *mProcess << "-f" << reportFile;
      *mProcess << "-U" << KatalogSettings::dbUser();
      *mProcess << "-p" << KatalogSettings::dbPassword();
      *mProcess << "-D" << KatalogSettings::dbFile();
      *mProcess << "-add-parameter" << QString( "%1,docID" ).arg( KProcess::quote( id.toString() ) );
      *mProcess << "-O" << "preview";

      mProcess->start( KProcess::NotifyOnExit );
    }
}

void ReportGenerator::slotViewerClosed( KProcess* )
{
  kdDebug() << "Viewer closed down" << endl;
}

#include "reportgenerator.moc"
