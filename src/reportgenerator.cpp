
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
#include "texttemplate.h"

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
  kdDebug() << "Report BASE:\n" << templ << endl;

  if ( ! templ.isEmpty() ) {
    KTempFile temp( QString(), ".trml" );

    QTextStream *s = temp.textStream();
    s->setEncoding( QTextStream::UnicodeUTF8 );
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

QString ReportGenerator::findTemplate( const QString& type )
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

  return tmplFile;
}

#define TAG( THE_TAG )  QString( "%1").arg( THE_TAG )

QString ReportGenerator::fillupTemplateFromArchive( const dbID& id )
{
  ArchDoc archive( id );

  QString tmplFile = findTemplate( archive.docType() );

  if ( tmplFile.isEmpty() ) {
    return QString();
  }

  // create a text template
  TextTemplate tmpl( tmplFile );

  /* replace the placeholders */
  /* A placeholder has the format <!-- %VALUE --> */

  ArchDocPositionList posList = archive.positions();
  QString loopResult;
  QString h;

  ArchDocPositionList::iterator it;
  int specialPosCnt = 0;

  for ( it = posList.begin(); it != posList.end(); ++it ) {
    ArchDocPosition pos (*it);
    tmpl.createDictionary( "POSITIONS" );
    tmpl.setValue( "POSITIONS", TAG( "POS_NUMBER" )
                   , pos.posNumber() );
    tmpl.setValue( "POSITIONS", "POS_TEXT",
                   rmlString( pos.text(), QString( "%1text" ).arg( pos.kind().lower() ) ) );

    h.setNum( pos.amount(), 'f', 2 );
    tmpl.setValue( "POSITIONS", "POS_AMOUNT", h );
    tmpl.setValue( "POSITIONS", "POS_UNIT", pos.unit() );
    tmpl.setValue( "POSITIONS", "POS_UNITPRICE", pos.unitPrice().toString() );
    tmpl.setValue( "POSITIONS", "POS_TOTAL", pos.overallPrice().toString() );
    tmpl.setValue( "POSITIONS", "POS_KIND", pos.kind().lower() );

    if ( !pos.kind().isEmpty() ) {
      specialPosCnt++;
    }
  }
  if ( specialPosCnt ) {
    tmpl.createDictionary( "SPECIAL_POS" );
    tmpl.setValue( "SPECIAL_POS", "COUNT", QString::number( specialPosCnt ) );
  }

  /* now replace stuff in the whole document */
  tmpl.setValue( TAG( "DATE" ), KGlobal().locale()->formatDate( archive.date(), true ) );
  tmpl.setValue( TAG( "DOCTYPE" ), archive.docType() );
  tmpl.setValue( TAG( "ADDRESS" ), archive.address() );
  tmpl.setValue( TAG( "DOCID" ),   archive.ident() );
  tmpl.setValue( TAG( "SALUT" ),   archive.salut() );
  tmpl.setValue( TAG( "GOODBYE" ), archive.goodbye() );
  tmpl.setValue( TAG( "PRETEXT" ),   rmlString( archive.preText() ) );
  tmpl.setValue( TAG( "POSTTEXT" ),  rmlString( archive.postText() ) );
  tmpl.setValue( TAG( "BRUTTOSUM" ), archive.bruttoSum().toString() );
  tmpl.setValue( TAG( "NETTOSUM" ),  archive.nettoSum().toString() );

  h.setNum( archive.vat(), 'f', 1 );
  tmpl.setValue( TAG( "VAT" ), h );
  tmpl.setValue( TAG( "VATSUM" ), archive.vatSum().toString() );

  // tmpl.setValue( TAG( "IMAGE" ), archive.


  KABC::Addressee contact;
  contact = KABC::StdAddressBook::self()->whoAmI();

  tmpl.setValue( TAG( "MY_NAME" ), contact.realName() );
  tmpl.setValue( TAG( "MY_ORGANISATION" ), contact.organization() );
  tmpl.setValue( TAG( "MY_URL" ), contact.url().prettyURL() );
  tmpl.setValue( TAG( "MY_EMAIL" ), contact.preferredEmail() );
  tmpl.setValue( TAG( "MY_PHONE" ), contact.phoneNumber( KABC::PhoneNumber::Work ).number() );
  tmpl.setValue( TAG( "MY_FAX" ), contact.phoneNumber( KABC::PhoneNumber::Fax ).number() );
  tmpl.setValue( TAG( "MY_CELL" ), contact.phoneNumber( KABC::PhoneNumber::Cell ).number() );

  KABC::Address address;
  address = contact.address( KABC::Address::Work );
  tmpl.setValue( TAG( "MY_POSTBOX" ),
              address.postOfficeBox() );

  tmpl.setValue( TAG( "MY_EXTENDED" ),
                 address.extended() );
  tmpl.setValue( TAG( "MY_STREET" ),
                 address.street() );
  tmpl.setValue( TAG( "MY_LOCALITY" ),
                 address.locality() );
  tmpl.setValue( TAG( "MY_REGION" ),
                 address.region() );
  tmpl.setValue( TAG( "MY_POSTCODE" ),
                 address.postalCode() );
  tmpl.setValue( TAG( "MY_COUNTRY" ),
                 address.country() );
  tmpl.setValue( TAG( "MY_REGION" ),
                 address.region() );
  tmpl.setValue( TAG( "MY_LABEL" ),
                 address.label() );

  return tmpl.expand();
}

QString ReportGenerator::escapeTrml2pdfXML( const QString& str ) const
{
  QString re( QStyleSheet::escape( str ) );

  // FIXME: Workaround for broken trml2pdf which needs double escaped
  //        & characters to work properly.
  return re.replace( QChar( '&' ), "&amp;" );
}


QString ReportGenerator::rmlString( const QString& str, const QString& paraStyle ) const
{
  QString rml;

  QString style( paraStyle );
  if ( style.isEmpty() ) style = "text";

  QStringList li = QStringList::split( "\n", escapeTrml2pdfXML( str ) );
  rml = QString( "<para style=\"%1\">" ).arg( style );
  rml += li.join( QString( "</para><para style=\"%1\">" ).arg( style ) ) + "</para>";
  kdDebug() << "Returning " << rml << endl;
  return rml;
}

#if 0
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
          text.replace( reg.cap( 0 ), file );
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
#endif

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
