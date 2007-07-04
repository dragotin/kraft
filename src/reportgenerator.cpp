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
#include <qregexp.h>
#include <qstylesheet.h>

#include <kstaticdeleter.h>
#include <kdebug.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kabc/stdaddressbook.h>

#include "reportgenerator.h"
#include "kraftdoc.h"
#include "kraftdb.h"
#include "unitmanager.h"
#include "dbids.h"
#include "kraftsettings.h"
#include "katalogsettings.h"
#include "docposition.h"
#include "einheit.h"
#include "archiveman.h"
#include "archdoc.h"
#include "documentman.h"

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

/*
 * docID: document ID
 *  dbId: database ID of the archived doc.
 */
void ReportGenerator::createPdfFromArchive( const QString& docID, dbID archId )
{
  const QString templ = fillupTemplateFromArchive( archId );
  // kdDebug() << "Report BASE:\n" << templ << endl;

  if ( ! templ.isEmpty() ) {
    KTempFile temp( QString(), ".trml" );

    QTextStream *s = temp.textStream();
    *s << templ;
    temp.close();

    kdDebug() << "Wrote rml to " << temp.name() << endl;

    QString dId( docID );

    if ( docID.isEmpty() ) {
      dId = ArchiveMan::self()->documentID( archId );
    }
    runTrml2Pdf( temp.name(), dId, archId.toString() );
  }
}

QString ReportGenerator::readTemplate( const QString& type )
{
  KStandardDirs stdDirs;
  QString templFileName = QString( type ).lower()+ ".trml";
  QString findFile = "kraft/reports/" + templFileName;

  QString tmplFile = stdDirs.findResource( "data", findFile );
  QString re;

  if ( tmplFile.isEmpty() ) {
    findFile = "kraft/reports/invoice.trml";
    tmplFile = stdDirs.findResource( "data", findFile );
    if ( tmplFile.isEmpty() ) {
      KMessageBox::error( 0, i18n("A document template named %1 could not be loaded."
                                  "Please check the installation." ).arg( templFileName ) ,
                          i18n( "Template not found" ) );
      return QString();
    } else {
      kdDebug() << templFileName << " not found, reverting to invoice.trml" << endl;
    }
  }

  kdDebug() << "Loading create file from " << findFile << endl;
  QFile f( tmplFile );
  if ( !f.open( IO_ReadOnly ) ) {
    kdError() << "Could not open " << tmplFile << endl;
    return QString();
  }

  QTextStream ts( &f );
  re = ts.read();
  f.close();

  return re;
}

#define TAG( FOO )  QString( "<!-- %1 -->").arg( FOO )

QString ReportGenerator::fillupTemplateFromArchive( const dbID& id )
{
  ArchDoc archive( id );

  QString tmpl = readTemplate( archive.docType() );

  if ( tmpl.isEmpty() ) {
    return QString();
  }
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

  ArchDocPositionList posList = archive.positions();
  QString loopResult;
  QString h;

  if ( ! loop.isEmpty() ) {

    ArchDocPositionList::iterator it;
    for ( it = posList.begin(); it != posList.end(); ++it ) {
      ArchDocPosition pos (*it);

      QString loopPart = loop;
      replaceTag( loopPart,
                  TAG( "POS_NUMBER" ),
                  pos.posNumber() );

      replaceTag( loopPart,
                  TAG( "POS_TEXT" ),
                  pos.text(),  true ); // multiline

      h.setNum( pos.amount(), 'f', 2 );
      replaceTag( loopPart,
                  TAG( "POS_AMOUNT" ),
                  h );

      replaceTag( loopPart,
                  TAG( "POS_UNIT" ),
                  pos.unit() );

      replaceTag( loopPart,
                  TAG( "POS_UNITPRICE" ),
                  pos.unitPrice().toString() );

      replaceTag( loopPart,
                  TAG( "POS_TOTAL" ),
                  pos.overallPrice().toString() );

      loopResult.append( loopPart );
    }
  }

  tmpl.replace( posStart+22, posEnd-posStart-22, loopResult );

  /* now replace stuff in the whole document */
  replaceTag( tmpl,
              TAG( "DATE" ),
              KGlobal().locale()->formatDate( archive.date(), true ) );
  replaceTag( tmpl,
              TAG( "DOCTYPE" ),
              archive.docType() );
  replaceTag( tmpl,
              TAG( "ADDRESS" ),
              archive.address() );
  replaceTag( tmpl,
              TAG( "DOCID" ),
              archive.ident() );
  replaceTag( tmpl,
              TAG( "SALUT" ),
              archive.salut() );
  replaceTag( tmpl,
              TAG( "GOODBYE" ),
              archive.goodbye() );
  replaceTag( tmpl,
              TAG( "PRETEXT" ),
              archive.preText(), true ); // multiline
  replaceTag( tmpl,
              TAG( "POSTTEXT" ),
              archive.postText(), true ); // multiline

  replaceTag( tmpl,
              TAG( "BRUTTOSUM" ),
              archive.bruttoSum().toString() );
  replaceTag( tmpl,
              TAG( "NETTOSUM" ),
              archive.nettoSum().toString() );

  h.setNum( archive.vat(), 'f', 1 );
  replaceTag( tmpl,
              TAG( "VAT" ),
              h );
  replaceTag( tmpl,
              TAG( "VATSUM" ),
              archive.vatSum().toString() );

  replaceTag( tmpl,
              TAG( "IMAGE" ) );

  tmpl = replaceOwnAddress( tmpl );

  return tmpl;
}


QString ReportGenerator::replaceOwnAddress( QString& tmpl )
{
  KABC::Addressee contact;
  contact = KABC::StdAddressBook::self()->whoAmI();

  replaceTag( tmpl,
              TAG( "MY.NAME" ),
              contact.realName() );

  replaceTag( tmpl,
              TAG( "MY.ORGANISATION" ),
              contact.organization() );

  replaceTag( tmpl,
              TAG( "MY.URL" ),
              contact.url().prettyURL() );

  replaceTag( tmpl,
              TAG( "MY.EMAIL" ),
              contact.preferredEmail() );

  replaceTag( tmpl,
              TAG( "MY.PHONE" ),
              contact.phoneNumber( KABC::PhoneNumber::Work ).number() );

  replaceTag( tmpl,
              TAG( "MY.FAX" ),
              contact.phoneNumber( KABC::PhoneNumber::Fax ).number() );

  replaceTag( tmpl,
              TAG( "MY.CELL" ),
              contact.phoneNumber( KABC::PhoneNumber::Cell ).number() );

  KABC::Address address;
  address = contact.address( KABC::Address::Work );
  replaceTag( tmpl,
              TAG( "MY.POSTBOX" ),
              address.postOfficeBox() );
  replaceTag( tmpl,
              TAG( "MY.EXTENDED" ),
              address.extended() );
  replaceTag( tmpl,
              TAG( "MY.STREET" ),
              address.street() );
  replaceTag( tmpl,
              TAG( "MY.LOCALITY" ),
              address.locality() );
  replaceTag( tmpl,
              TAG( "MY.REGION" ),
              address.region() );
  replaceTag( tmpl,
              TAG( "MY.POSTCODE" ),
              address.postalCode() );
  replaceTag( tmpl,
              TAG( "MY.COUNTRY" ),
              address.country() );
  replaceTag( tmpl,
              TAG( "MY.REGION" ),
              address.region() );
  replaceTag( tmpl,
              TAG( "MY.LABEL" ),
              address.label() );


  return tmpl;
}

QString ReportGenerator::escapeTrml2pdfXML( const QString& str ) const
{
  QString re( QStyleSheet::escape( str ) );

  // FIXME: Workaround for broken trml2pdf which needs double escaped
  //        & characters to work properly.
  return re.replace( QChar( '&' ), "&amp;" );
}


QString ReportGenerator::rmlString( const QString& str ) const
{
  QString rml;

  QStringList li = QStringList::split( "\n", escapeTrml2pdfXML( str ) );
  rml = "<para style=\"text\">" + li.join( "</para><para style=\"text\">" ) + "</para>";
  kdDebug() << "Returning " << rml << endl;
  return rml;
}

int ReportGenerator::replaceTag( QString& text, const QString& tag,  const QString& rep, bool multiline )
{

  if ( tag == TAG( "IMAGE" ) ) {
    kdDebug() << "Replacing image tag" << endl;
    QRegExp reg( "<!-- IMAGE\\(\\s*(\\S+)\\s*\\) -->" );

    KStandardDirs stdDirs;
    int pos = 0;

    while ( pos >= 0 ) {
      pos = reg.search( text, pos );
      const QString filename = reg.cap( 1 );
      kdDebug() << "Found position: " << pos << endl;

      if ( ! filename.isEmpty() ) {
        QString findFile = "kraft/reports/images/" + filename;
        QString file = stdDirs.findResource( "data", findFile );
        if ( file.isEmpty() ) {
          kdDebug() << "can not find findFile " << findFile << endl;
          pos = -1; // Better break here to avoid infinite loop.
        } else {
          text.replace( reg, file );
        }
      }
    }
  } else {
    if ( multiline ) {
      text.replace( tag, rmlString( rep ), false );
    } else {
      text.replace( tag, escapeTrml2pdfXML( rep ), false );
    }
  }
  return 0;
}


void ReportGenerator::runTrml2Pdf( const QString& rmlFile, const QString& docID, const QString& archId )
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

  QString rmlbin = KraftSettings::self()->trml2PdfBinary();

  if ( rmlbin == "trml2pdf" || ! QFile::exists( rmlbin ) ) {
    QStringList pathes;
#if 0
    pathes << "/usr/local/bin/trml2pdf";
    pathes << "/usr/bin/trml2pdf";
    pathes << "/usr/local/bin/trml2pdf.py";
    pathes << "/usr/bin/trml2pdf.py";
#endif
    KStandardDirs stdDirs;
    pathes = stdDirs.systemPaths();

    for ( QStringList::Iterator it = pathes.begin(); it != pathes.end(); ++it ) {
      QString cPath = ( *it ) + "/trml2pdf";
      if ( QFile::exists( cPath ) ) {
        rmlbin = cPath;
        kdDebug() << "Found trml2pdf in filesystem: " << rmlbin << endl;

      }
    }

    if ( rmlbin == "trml2pdf" || ! QFile::exists( rmlbin ) ) {

      KMessageBox::error( 0, i18n("The utility trml2pdf could not be found, but is required to create documents."
                                  "Please make sure the package is installed accordingly." ),
                          i18n( "Document Generation Error" ) );
      return;
    } else {
      kdDebug() << "Using rml2pdf script: " << rmlbin << endl;
    }

  }

  QString outputDir = ArchiveMan::self()->pdfBaseDir();
  QString filename = ArchiveMan::self()->archiveFileName( docID, archId, "pdf" );
  mOutFile = QString( "%1/%2" ).arg( outputDir ).arg( filename );

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
  // kdDebug() << "==> Datablock of size " << len << endl;
  mTargetStream.writeRawBytes( buffer, len );
}

void ReportGenerator::slotRecStderr( KProcess *, char * buffer, int len )
{
  mErrors.append( QString::fromUtf8( buffer,  len ) );
}

void ReportGenerator::slotViewerClosed( KProcess *p )
{
  mFile.close();
  kdDebug() << "Trml2pdf Process finished with status " << p->exitStatus() << endl;

  if ( p->exitStatus() == 0 ) {
    emit pdfAvailable( mOutFile );
    mOutFile = QString();
#if 0
    KURL url( mOutFile );
    KRun::runURL( url, "application/pdf" );
#endif
  } else {
    // KMessageBox::detailedError (QWidget *parent, const QString &text, const QString &details, const QString &caption=QString::null, int options=Notify)
    KMessageBox::detailedError ( 0,
                                 i18n( "Could not generate the pdf file. The trml2pdf script failed." ),
                                 mErrors,
                                 i18n( "rml2pdf Error" ) );
  }
}

#include "reportgenerator.moc"
