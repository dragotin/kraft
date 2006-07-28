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

static KStaticDeleter<ReportGenerator> selfDeleter;

ReportGenerator* ReportGenerator::mSelf = 0;

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

}

void ReportGenerator::docPreview( const dbID& dbId )
{
    if( ! mProcess ) {
	mProcess = new KProcess;
	connect( mProcess, SIGNAL( processExited(  KProcess * ) ),
		 this,     SLOT( slotViewerClosed( KProcess * ) ) );
    } 
    
    *mProcess << KraftSettings::nCReportBinary();

    mProcess->start();
	
}

void ReportGenerator::slotViewerClosed( KProcess* )
{

}


#include "reportgenerator.moc"
