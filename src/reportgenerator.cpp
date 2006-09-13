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
#include <qtextstream.h>

#include <kstaticdeleter.h>
#include <kdebug.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kurl.h>
#include <krun.h>

#include "reportgenerator.h"
#include "kraftdoc.h"
#include "kraftdb.h"
#include "unitmanager.h"
#include "dbids.h"
#include "kraftsettings.h"
#include "katalogsettings.h"
#include "docposition.h"
#include "einheit.h"
#include <kmessagebox.h>

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
  // kdDebug() << "Report BASE:\n" << templ << endl;

  KTempFile temp( QString(), ".trml" );

  QTextStream *s = temp.textStream();
  *s << templ;
  temp.close();

  kdDebug() << "Wrote rml to " << temp.name() << endl;
  runTrml2Pdf( temp.name(),  doc->ident() );
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


void ReportGenerator::runTrml2Pdf( const QString& rmlFile, const QString& id )
{
  if( ! mProcess ) {
    mProcess = new KProcess;
    connect( mProcess, SIGNAL( processExited(  KProcess * ) ),
             this,     SLOT( slotViewerClosed( KProcess * ) ) );

    connect( mProcess,  SIGNAL( wroteStdin( KProcess* ) ),
             this,  SLOT( slotWroteStdin( KProcess* ) ) );

    connect( mProcess,  SIGNAL( receivedStdout( KProcess *, char *, int ) ),
             this,  SLOT( slotRecStdout( KProcess *, char *, int ) ) );

    connect( mProcess,  SIGNAL( receivedStderr( KProcess *, char *, int ) ),
             this,  SLOT( slotRecStderr( KProcess *, char *, int ) ) );
  } else {
    mProcess->clearArguments();
  }

  mErrors = QString();

  const QString rmlbin = KraftSettings::self()->trml2PdfBinary();
  kdDebug() << "Using trml2pdf: " << rmlbin << endl;

  KStandardDirs stdDirs;
  QString outputDir = KraftSettings::self()->pdfOutputDir();
  if ( ! outputDir.endsWith( "/" ) ) outputDir += "/";
  mOutFile = outputDir + id + ".pdf";
  kdDebug() << "Writing output to " << mOutFile << endl;

  *mProcess << rmlbin;
  *mProcess << rmlFile;

  mFile.setName( mOutFile );
  if ( mFile.open( IO_WriteOnly ) ) {
    mTargetStream.setDevice( &mFile );
    mProcess->start( KProcess::NotifyOnExit, KProcess::AllOutput );
  }
}

void ReportGenerator::slotWroteStdin( KProcess* )
{
  kdDebug() << "Writing on stdin finished!" << endl;
}

void ReportGenerator::slotRecStdout( KProcess *, char * buffer, int len)
{
  QString buf = QString::fromUtf8( buffer,  len );
  kdDebug() << "==> Datablock of size " << len << endl;
  mTargetStream << buf << endl;
}

void ReportGenerator::slotRecStderr( KProcess *, char * buffer, int len )
{
  mErrors.append( QString::fromUtf8( buffer,  len ) );
}

void ReportGenerator::docPreview( const dbID& )
{
    if( ! mProcess ) {
	mProcess = new KProcess;
	connect( mProcess, SIGNAL( processExited(  KProcess * ) ),
		 this,     SLOT( slotViewerClosed( KProcess * ) ) );
    } else {
      mProcess->clearArguments();
    }


#if 0
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
#endif
}

void ReportGenerator::slotViewerClosed( KProcess *p )
{
  mFile.close();
  kdDebug() << "Trml2pdf Process finished with status " << p->exitStatus() << endl;

  if ( p->exitStatus() == 0 ) {
    KURL url( mOutFile );
    KRun::runURL( url, "application/pdf" );
  } else {
    // KMessageBox::detailedError (QWidget *parent, const QString &text, const QString &details, const QString &caption=QString::null, int options=Notify)
    KMessageBox::detailedError ( 0,
                                 i18n( "Could not generate the pdf file. The trml2pdf script failed." ),
                                 mErrors,
                                 i18n( "rml2pdf Error" ) );
  }
}

#include "reportgenerator.moc"
