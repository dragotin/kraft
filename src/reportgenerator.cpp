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
#include <qfile.h>

#include <kstaticdeleter.h>
#include <kdebug.h>
#include <kprocess.h>
#include <kstandarddirs.h>

#include "reportgenerator.h"
#include "kraftdoc.h"
#include "kraftdb.h"
#include "unitmanager.h"
#include "dbids.h"
#include "kraftsettings.h"
#include "katalogsettings.h"
#include "docposition.h"
#include "einheit.h"

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

void ReportGenerator::createRml( DocGuardedPtr doc )
{
  const QString templ = getTemplate( doc );
  kdDebug() << "Report BASE:\n" << templ << endl;

}

#define TAG( FOO )  QString( "<!-- %1 -->").arg( FOO )

QString ReportGenerator::getTemplate( DocGuardedPtr doc )
{
  KStandardDirs stdDirs;
  QString findFile = "kraft/reports/" + doc->docType().lower() + ".trml";

  QString tmplFile = stdDirs.findResource( "data", findFile );
  kdDebug() << "Loading create file from " << findFile << endl;

  QFile f( tmplFile );
  if ( !f.open( IO_ReadOnly ) ) {
    kdError() << "Could not open " << tmplFile << endl;
    return QString();
  }

  QTextStream ts( &f );
  QString tmpl = ts.read();

  /* replace the placeholders */
  /* A placeholder has the format <!-- %VALUE --> */

  /* find the position loop */
  int posStart = tmpl.find(
    TAG( "POSITION_LOOP" )
    );
  int posEnd   = tmpl.find(
    TAG( "POSITION_LOOP_END" )
    );

  QString loop;
  if ( posStart > 0 && posEnd > 0 && posStart < posEnd ) {
    loop = tmpl.mid( posStart+22,  posEnd-posStart-22 );
    kdDebug() << "Loop part: " << loop << endl;
  }

  DocPositionList posList = doc->positions();
  QString loopResult;
  QString h;

  if ( ! loop.isEmpty() ) {
    DocPositionBase  *dpbase;
    for( dpbase = posList.first(); dpbase; dpbase = posList.next() ) {
      DocPosition *dpb = static_cast<DocPosition*>( dpbase );

      QString loopPart = loop;
      replaceTag( loopPart,
                  TAG( "POS_NUMBER" ),
                  dpb->position() );

      replaceTag( loopPart,
                  TAG( "POS_TEXT" ),
                  dpb->text() );

      h.setNum( dpb->amount(), 'f', 2 );
      replaceTag( loopPart,
                  TAG( "POS_AMOUNT" ),
                  h );

      h = dpb->unit().einheit( dpb->amount() );
      replaceTag( loopPart,
                  TAG( "POS_UNIT" ),
                  h );

      replaceTag( loopPart,
                  TAG( "POS_UNITPRICE" ),
                  dpb->unitPrice().toString() );

      replaceTag( loopPart,
                  TAG( "POS_TOTAL" ),
                  dpb->overallPrice().toString() );

      loopResult.append( loopPart );
    }
  }

  tmpl.replace( posStart+22, posEnd-posStart-22, loopResult );

  /* now replace stuff in the whole document */
  replaceTag( tmpl,
              TAG( "DATE" ),
              KGlobal().locale()->formatDate( doc->date() ) );
  replaceTag( tmpl,
              TAG( "DOCTYPE" ),
              doc->docType() );
  replaceTag( tmpl,
              TAG( "ADDRESS" ),
              doc->address() );
  replaceTag( tmpl,
              TAG( "DOCID" ),
              doc->ident() );
  replaceTag( tmpl,
              TAG( "SALUT" ),
              doc->salut() );
  replaceTag( tmpl,
              TAG( "GOODBYE" ),
              doc->goodbye() );
  replaceTag( tmpl,
              TAG( "PRETEXT" ),
              doc->preText() );
  replaceTag( tmpl,
              TAG( "POSTTEXT" ),
              doc->postText() );

  replaceTag( tmpl,
              TAG( "BRUTTOSUM" ),
              doc->bruttoSum().toString() );
  replaceTag( tmpl,
              TAG( "NETTOSUM" ),
              doc->nettoSum().toString() );

  h.setNum( doc->vat(), 'f', 1 );
  replaceTag( tmpl,
              TAG( "VAT" ),
              h );
  replaceTag( tmpl,
              TAG( "VATSUM" ),
              doc->vatSum().toString() );

  return tmpl;
}

int ReportGenerator::replaceTag( QString& text, const QString& tag,  const QString& rep )
{
  text.replace( tag, rep, false );
  return 0;
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
