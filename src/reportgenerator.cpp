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

#include <QSqlRecord>
#include <QSqlIndex>
#include <QFile>
#include <QTextStream>
#include <QRegExp>
#include <QList>
#include <QTextDocument>

#include <kdebug.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <akonadi/contact/contactsearchjob.h>

#include "reportgenerator.h"
#include "kraftdoc.h"
#include "kraftdb.h"
#include "unitmanager.h"
#include "dbids.h"
#include "kraftsettings.h"
#include "docposition.h"
#include "einheit.h"
#include "archiveman.h"
#include "archdoc.h"
#include "documentman.h"
#include "texttemplate.h"
#include "defaultprovider.h"
#include "doctype.h"

KProcess* ReportGenerator::mProcess = 0;

ReportGenerator *ReportGenerator::self()
{
  K_GLOBAL_STATIC(ReportGenerator, mSelf);
  return mSelf;
}

ReportGenerator::ReportGenerator()
  :mArchDoc( 0 )
{
  connect( this, SIGNAL( templateGenerated( const QString& )),
           this, SLOT( slotConvertTemplate( const QString& )));
}

ReportGenerator::~ReportGenerator()
{
  kDebug() << "ReportGen is destroyed!";
}

/*
 * docID: document ID
 *  dbId: database ID of the archived doc.
 */
void ReportGenerator::createPdfFromArchive( const QString& docID, dbID archId )
{
  mDocId = docID;
  mArchId = archId;
  fillupTemplateFromArchive( archId );
  // Method fillupTemplateFromArchive raises a signal when the archive was
  // generated. This signal gets connected to slotConvertTemplate in the
  // constructor of this class.
}

void ReportGenerator::slotConvertTemplate( const QString& templ )
{
  kDebug() << "Report BASE:\n" << templ;

  if ( ! templ.isEmpty() ) {
    KTemporaryFile temp;
    temp.setSuffix( ".trml" );
    temp.setAutoRemove( false );

    if ( temp.open() ) {
      QTextStream s(&temp);
      // s.setCodec( QTextCodec::codecForLocale() );
      s << templ;
    } else {
      kDebug() << "ERROR: Could not open temporar file";
    }

    kDebug() << "Wrote rml to " << temp.fileName();

    QString dId( mDocId );

    if ( mDocId.isEmpty() ) {
      dId = ArchiveMan::self()->documentID( mArchId );
    }
    runTrml2Pdf( temp.fileName(), dId, mArchId.toString() );
  }
}

QString ReportGenerator::findTemplate( const QString& type )
{
  DocType dType( type );

  QString tmplFile = dType.templateFile();

  if ( tmplFile.isEmpty() ) {
    KMessageBox::error( 0, i18n("A document template named %1 could not be loaded. "
                                "Please check the installation." ).arg( dType.templateFile() ) ,
                        i18n( "Template not found" ) );
  }

  mMergeIdent = dType.mergeIdent();
  mWatermarkFile = dType.watermarkFile();

  return tmplFile;
}

#define TAG( THE_TAG )  QString( "%1").arg( THE_TAG )
#define DICT( THE_DICT )  QString( "%1").arg( THE_DICT )

void ReportGenerator::fillupTemplateFromArchive( const dbID& id )
{
  mArchDoc = new ArchDoc(id);

  // FIXME: Read all contacts, even if the customer is not in the address book
  // we need to read all for our own address.
  Akonadi::ContactSearchJob *job = new Akonadi::ContactSearchJob;
  connect( job, SIGNAL( result( KJob* ) ), SLOT( addressReceived( KJob* ) ) );
}

// Result Slot of the Contact Search above
void ReportGenerator::addressReceived( KJob* job )
{
  kDebug() << "Reading Akonadi Search Job!";
  KABC::Addressee addressee ;
  KABC::Addressee myAddress;

  if( ! job ) {
    kError() << "Akonadi Address search job failed!";
    return;
  }

  if ( job->error() ) {
    qDebug() << "Akonadi Contact Job Read Error: " << job->errorString();
    return;
  }

  QString myName = KraftSettings::self()->userName();
  kDebug() << "MY name is " << myName;

  Akonadi::ContactSearchJob *searchJob = qobject_cast<Akonadi::ContactSearchJob*>( job );
  KABC::Addressee::List contacts = searchJob->contacts();
  kDebug() << "Amount of address entries: " << contacts.size();

  // iterate over all found contacts and search for the customer address
  // and 'My' address
  foreach ( const KABC::Addressee &contact, contacts ) {
    if( contact.uid() == mArchDoc->clientUid() ) {
      addressee = contact;
    }
    kDebug() << "XXXXXXXXXX comparing " << contact.name() << " with " << myName;
    if( contact.name() == myName ) {
      myAddress = contact;
    }
    if( !( myAddress.isEmpty() || addressee.isEmpty() ) ) break;
  }

  QString tmplFile = findTemplate( mArchDoc->docType() );

  if ( tmplFile.isEmpty() ) {
    kDebug() << "tmplFile is emty!";
    return;
  } else {
    kDebug() << "Reading this template: " << tmplFile;
  }

  // create a text template
  TextTemplate tmpl( tmplFile );

  /* replace the placeholders */
  /* A placeholder has the format <!-- %VALUE --> */

  ArchDocPositionList posList = mArchDoc->positions();
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
                   rmlString( pos.text(), QString( "%1text" ).arg( pos.kind().toLower() ) ) );

    h.setNum( pos.amount(), 'f', 2 );
    tmpl.setValue( "POSITIONS", "POS_AMOUNT", h );
    tmpl.setValue( "POSITIONS", "POS_UNIT", pos.unit() );
    tmpl.setValue( "POSITIONS", "POS_UNITPRICE", pos.unitPrice().toString( mArchDoc->locale() ) );
    tmpl.setValue( "POSITIONS", "POS_TOTAL", pos.nettoPrice().toString( mArchDoc->locale() ) );
    tmpl.setValue( "POSITIONS", "POS_KIND", pos.kind().toLower() );

    if ( !pos.kind().isEmpty() ) {
      specialPosCnt++;
    }
  }
  if ( specialPosCnt ) {
    tmpl.createDictionary( "SPECIAL_POS" );
    tmpl.setValue( "SPECIAL_POS", "COUNT", QString::number( specialPosCnt ) );
  }

  /* now replace stuff in the whole document */
  tmpl.setValue( TAG( "DATE" ), mArchDoc->locale()->formatDate(
                   mArchDoc->date(), KLocale::ShortDate ) );
  tmpl.setValue( TAG( "DOCTYPE" ), mArchDoc->docType() );
  tmpl.setValue( TAG( "ADDRESS" ), mArchDoc->address() );

  tmpl.setValue( TAG( "CLIENT_NAME" ), addressee.realName() );
  tmpl.setValue( TAG( "CLIENT_ORGANISATION" ), addressee.organization() );
  tmpl.setValue( TAG( "CLIENT_URL" ), addressee.url().prettyUrl() );
  tmpl.setValue( TAG( "CLIENT_EMAIL" ), addressee.preferredEmail() );
  tmpl.setValue( TAG( "CLIENT_PHONE" ), addressee.phoneNumber( KABC::PhoneNumber::Work ).number() );
  tmpl.setValue( TAG( "CLIENT_FAX" ), addressee.phoneNumber( KABC::PhoneNumber::Fax ).number() );
  tmpl.setValue( TAG( "CLIENT_CELL" ), addressee.phoneNumber( KABC::PhoneNumber::Cell ).number() );
  KABC::Address clientAddress;
  clientAddress = addressee.address( KABC::Address::Pref );
  tmpl.setValue( TAG( "CLIENT_POSTBOX" ),
                 clientAddress.postOfficeBox() );
  tmpl.setValue( TAG( "CLIENT_EXTENDED" ),
                 clientAddress.extended() );
  tmpl.setValue( TAG( "CLIENT_STREET" ),
                 clientAddress.street() );
  tmpl.setValue( TAG( "CLIENT_LOCALITY" ),
                 clientAddress.locality() );
  tmpl.setValue( TAG( "CLIENT_REGION" ),
                 clientAddress.region() );
  tmpl.setValue( TAG( "CLIENT_POSTCODE" ),
                 clientAddress.postalCode() );
  tmpl.setValue( TAG( "CLIENT_COUNTRY" ),
                 clientAddress.country() );
  tmpl.setValue( TAG( "CLIENT_REGION" ),
                 clientAddress.region() );
  tmpl.setValue( TAG( "CLIENT_LABEL" ),
                 clientAddress.label() );

  tmpl.setValue( TAG( "DOCID" ),   mArchDoc->ident() );
  tmpl.setValue( TAG( "PROJECTLABEL" ),   mArchDoc->projectLabel() );
  tmpl.setValue( TAG( "SALUT" ),   mArchDoc->salut() );
  tmpl.setValue( TAG( "GOODBYE" ), mArchDoc->goodbye() );
  tmpl.setValue( TAG( "PRETEXT" ),   rmlString( mArchDoc->preText() ) );
  tmpl.setValue( TAG( "POSTTEXT" ),  rmlString( mArchDoc->postText() ) );
  tmpl.setValue( TAG( "BRUTTOSUM" ), mArchDoc->bruttoSum().toString( mArchDoc->locale() ) );
  tmpl.setValue( TAG( "NETTOSUM" ),  mArchDoc->nettoSum().toString( mArchDoc->locale() ) );


  h.setNum( mArchDoc->tax(), 'f', 1 );
  kDebug() << "Tax in archive document: " << h;
  if ( mArchDoc->reducedTaxSum().toLong() > 0 ) {
    tmpl.createDictionary( DICT( "SECTION_REDUCED_TAX" ) );
    tmpl.setValue( "SECTION_REDUCED_TAX", TAG( "REDUCED_TAX_SUM" ),
      mArchDoc->reducedTaxSum().toString( mArchDoc->locale() ) );
    h.setNum( mArchDoc->reducedTax(), 'f', 1 );
    tmpl.setValue( "SECTION_REDUCED_TAX", TAG( "REDUCED_TAX" ), h );
    tmpl.setValue( "SECTION_REDUCED_TAX", TAG( "REDUCED_TAX_LABEL" ), i18n( "reduced VAT" ) );
  }
  if ( mArchDoc->fullTaxSum().toLong() > 0 ) {
    tmpl.createDictionary( DICT( "SECTION_FULL_TAX" ) );
    tmpl.setValue( "SECTION_FULL_TAX", TAG( "FULL_TAX_SUM" ),
      mArchDoc->fullTaxSum().toString( mArchDoc->locale() ) );
    h.setNum( mArchDoc->tax(), 'f', 1 );
    tmpl.setValue( "SECTION_FULL_TAX", TAG( "FULL_TAX" ), h );
    tmpl.setValue( "SECTION_FULL_TAX", TAG( "FULL_TAX_LABEL" ), i18n( "VAT" ) );
  }

  h.setNum( mArchDoc->tax(), 'f', 1 );
  tmpl.setValue( TAG( "VAT" ), h );

  tmpl.setValue( TAG( "VATSUM" ), mArchDoc->taxSum().toString( mArchDoc->locale() ) );

  // My own contact data

  tmpl.setValue( TAG( "MY_NAME" ), myAddress.realName() );
  tmpl.setValue( TAG( "MY_ORGANISATION" ), myAddress.organization() );
  tmpl.setValue( TAG( "MY_URL" ), myAddress.url().prettyUrl() );
  tmpl.setValue( TAG( "MY_EMAIL" ), myAddress.preferredEmail() );
  tmpl.setValue( TAG( "MY_PHONE" ), myAddress.phoneNumber( KABC::PhoneNumber::Work ).number() );
  tmpl.setValue( TAG( "MY_FAX" ), myAddress.phoneNumber( KABC::PhoneNumber::Fax ).number() );
  tmpl.setValue( TAG( "MY_CELL" ), myAddress.phoneNumber( KABC::PhoneNumber::Cell ).number() );

  KABC::Address address;
  address = myAddress.address( KABC::Address::Pref );
  if( address.isEmpty() )
    address = myAddress.address(KABC::Address::Work );
  if( address.isEmpty() )
    address = myAddress.address(KABC::Address::Home );
  if( address.isEmpty() )
    address = myAddress.address(KABC::Address::Postal );

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

  QString output = tmpl.expand();

  emit templateGenerated( output );

}

QString ReportGenerator::escapeTrml2pdfXML( const QString& str ) const
{
  QString re( Qt::escape( str ) );

  // FIXME: Workaround for broken trml2pdf which needs double escaped
  //        & characters to work properly.
  return re.replace( QChar( '&' ), "&amp;" );
}


QString ReportGenerator::rmlString( const QString& str, const QString& paraStyle ) const
{
  QString rml;

  QString style( paraStyle );
  if ( style.isEmpty() ) style = "text";

  // QStringList li = QStringList::split( "\n", escapeTrml2pdfXML( str ) );
  QStringList li = escapeTrml2pdfXML( str ).split( "\n" );
  rml = QString( "<para style=\"%1\">" ).arg( style );
  rml += li.join( QString( "</para><para style=\"%1\">" ).arg( style ) ) + "</para>";
  kDebug() << "Returning " << rml;
  return rml;
}

QString ReportGenerator::findTrml2Pdf( )
{
  const QString rmlbinDefault = QString::fromLatin1( "trml2pdf" ); // FIXME: how to get the default value?
  QString rmlbin = KraftSettings::self()->trml2PdfBinary();
  kDebug() << "### Start searching rml2pdf bin: " << rmlbin;

  mHaveMerge = false;

  if ( rmlbinDefault == rmlbin  ) {
    QStringList pathes;
    KStandardDirs stdDirs;
    pathes = stdDirs.systemPaths();

    for ( QStringList::Iterator it = pathes.begin(); it != pathes.end(); ++it ) {
      QString cPath = ( *it ) + "/trml2pdf_kraft.sh";
      kDebug() << "### Checking cPath: " << cPath;
      if ( QFile::exists( cPath ) ) {
        rmlbin = cPath;
        kDebug() << "Found trml2pdf_kraft.sh in filesystem: " << rmlbin;
        mHaveMerge = true;
        break;
      }
    }

    if ( ! mHaveMerge ) {
      for ( QStringList::Iterator it = pathes.begin(); it != pathes.end(); ++it ) {
        QString cPath = ( *it ) + "/trml2pdf";
        if ( QFile::exists( cPath ) ) {
          rmlbin = cPath;
          kDebug() << "Found trml2pdf in filesystem: " << rmlbin;
          break;
        }
      }
    }
  }
  if ( rmlbinDefault == rmlbin  ) {
    kDebug() << "We have not found the script!";
    rmlbin = QString();
  }

  return rmlbin;
}


void ReportGenerator::runTrml2Pdf( const QString& rmlFile, const QString& docID, const QString& archId )
{
  if( ! mProcess ) {
    mProcess = new KProcess;
    mProcess->setOutputChannelMode( KProcess::SeparateChannels );
    connect( mProcess, SIGNAL( finished( int ) ),this, SLOT( trml2pdfFinished( int ) ) );
    connect( mProcess, SIGNAL( readyReadStandardOutput()), this, SLOT( slotReceivedStdout() ) );
    connect( mProcess, SIGNAL( readyReadStandardError()), this, SLOT( slotReceivedStderr() ) );
    connect( mProcess, SIGNAL( error ( QProcess::ProcessError )), this, SLOT( slotError( QProcess::ProcessError)));
  } else {
    mProcess->clearProgram();
  }
  QStringList prg;

  mErrors = QString();
  QString rmlbin = findTrml2Pdf();

  if ( ! QFile::exists( rmlbin ) ) {

    KMessageBox::error( 0, i18n("The utility to create PDF from the rml file could not be found, "
                                "but is required to create documents."
                                "Please make sure the package is installed accordingly." ),
                        i18n( "Document Generation Error" ) );
    return;
  }

  if ( mHaveMerge && mMergeIdent != "0" &&
       ( mWatermarkFile.isEmpty() || !QFile::exists( mWatermarkFile ) ) ) {

    KMessageBox::error( 0, i18n("The Watermark file to merge with the document could not be found. "
                                "Merge is going to be disabled." ),
                        i18n( "Watermark Error" ) );
    mMergeIdent = "0";
  }

  QString outputDir = ArchiveMan::self()->pdfBaseDir();
  QString filename = ArchiveMan::self()->archiveFileName( docID, archId, "pdf" );
  mOutFile = QString( "%1/%2" ).arg( outputDir ).arg( filename );

  kDebug() << "Writing output to " << mOutFile;

  prg << rmlbin;
  if ( mHaveMerge ) {
    prg << mMergeIdent;
  }
  prg << rmlFile;
  if ( mHaveMerge && mMergeIdent != "0" ) {
    prg << mWatermarkFile;
  }

  mFile.setFileName( mOutFile );
  if ( mFile.open( QIODevice::WriteOnly ) ) {
    mProcess->setProgram( prg );
    mTargetStream.setDevice( &mFile );
    QStringList args = mProcess->program();
    kDebug() << "* rml2pdf Call-Arguments: " << args;

    mProcess->start( );
  }
}

void ReportGenerator::slotReceivedStdout( )
{
  mTargetStream << (mProcess->readAllStandardOutput());
}

void ReportGenerator::slotReceivedStderr( )
{
  mErrors.append( mProcess->readAllStandardError() );
}

void ReportGenerator::slotError( QProcess::ProcessError err )
{
  mErrors.append( i18n("Program ended with status %1").arg(err));
}

void ReportGenerator::trml2pdfFinished( int exitStatus)
{
  mTargetStream.flush();
  mFile.close();
  kDebug() << "Trml2pdf Process finished with status " << exitStatus;

  if ( exitStatus == 0 ) {
    emit pdfAvailable( mOutFile );
    mOutFile = QString();
  } else {
    if ( mErrors.isEmpty() ) mErrors = i18n( "Unknown problem." );
    // KMessageBox::detailedError (QWidget *parent, const QString &text, const QString &details, const QString &caption=QString::null, int options=Notify)
    KMessageBox::detailedError ( 0,
                                 i18n( "Could not generate the pdf file. The trml2pdf script failed." ),
                                 mErrors,
                                 i18n( "rml2pdf Error" ) );
    mErrors = QString();
  }
}


