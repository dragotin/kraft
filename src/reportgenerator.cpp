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
#include <QApplication>
#include <QTextCodec>

#include <kdebug.h>
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
#include "addressprovider.h"

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

  mProcess.setOutputChannelMode( KProcess:: OnlyStdoutChannel );
  connect( &mProcess, SIGNAL( finished( int ) ),this, SLOT( trml2pdfFinished( int ) ) );
  connect( &mProcess, SIGNAL( readyReadStandardOutput()), this, SLOT( slotReceivedStdout() ) );
  connect( &mProcess, SIGNAL( readyReadStandardError()), this, SLOT( slotReceivedStderr() ) );
  connect( &mProcess, SIGNAL( error ( QProcess::ProcessError )), this, SLOT( slotError( QProcess::ProcessError)));

  mAddressProvider = new AddressProvider( this );
  connect( mAddressProvider, SIGNAL( addresseeFound( const QString&, const KABC::Addressee& )),
           this, SLOT( slotAddresseeFound( const QString&, const KABC::Addressee& ) ) );
  connect( mAddressProvider, SIGNAL( finished(int) ),
           this, SLOT( slotAddresseeSearchFinished(int)) );
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
  // kDebug() << "Report BASE:\n" << templ;

  if ( ! templ.isEmpty() ) {
    KTemporaryFile temp;
    temp.setSuffix( ".trml" );
    temp.setAutoRemove( false );

    if ( temp.open() ) {
      QTextStream s(&temp);

      // The following explicit coding settings were needed for Qt 4.7.3, former Qt versions
      // seemed to default on UTF-8. Try to comment the following two lines for older Qt versions
      // if needed and see if the trml file on the disk still is UTF-8 encoded.
      QTextCodec *codec = QTextCodec::codecForName("UTF-8");
      s.setCodec( codec );

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

  QString tmplFile = dType.templateFile( mArchDoc->locale()->country() );

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

  const QString clientUid = mArchDoc->clientUid();
  if( ! clientUid.isEmpty() ) {
    mAddressProvider->getAddressee( clientUid );
  } else {
    // no address UID specified, skip the addressee search and generate the template directly
    slotAddresseeSearchFinished(0);
  }
}

void ReportGenerator::slotAddresseeFound( const QString&, const KABC::Addressee& contact )
{
  mCustomerContact = contact;
}

void ReportGenerator::setMyContact( const KABC::Addressee& contact )
{
  myContact = contact;
}

void ReportGenerator::slotAddresseeSearchFinished( int )
{
  // now the addressee search through the address provider is finished.
  // Rendering can be started.
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

    // format the amount value of the item, do not show the precision if there is no fraction
    double amount = pos.amount();
    if( amount - qRound(amount) > 0 ) // if there is no fraction
      h = mArchDoc->locale()->formatNumber( amount, 2 );
    else
      h = mArchDoc->locale()->formatNumber( amount, 0 );

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

  contactToTemplate( &tmpl, "CLIENT", mCustomerContact );
  contactToTemplate( &tmpl, "MY", myContact );

  tmpl.setValue( TAG( "DOCID" ),   mArchDoc->ident() );
  tmpl.setValue( TAG( "PROJECTLABEL" ),   mArchDoc->projectLabel() );
  tmpl.setValue( TAG( "SALUT" ),   mArchDoc->salut() );
  tmpl.setValue( TAG( "GOODBYE" ), mArchDoc->goodbye() );
  tmpl.setValue( TAG( "PRETEXT" ),   rmlString( mArchDoc->preText() ) );
  tmpl.setValue( TAG( "POSTTEXT" ),  rmlString( mArchDoc->postText() ) );
  tmpl.setValue( TAG( "BRUTTOSUM" ), mArchDoc->bruttoSum().toString( mArchDoc->locale() ) );
  tmpl.setValue( TAG( "NETTOSUM" ),  mArchDoc->nettoSum().toString( mArchDoc->locale() ) );

  h = mArchDoc->locale()->formatNumber( mArchDoc->tax() );
  kDebug() << "Tax in archive document: " << h;
  if ( mArchDoc->reducedTaxSum().toLong() > 0 ) {
    tmpl.createDictionary( DICT( "SECTION_REDUCED_TAX" ) );
    tmpl.setValue( "SECTION_REDUCED_TAX", TAG( "REDUCED_TAX_SUM" ),
      mArchDoc->reducedTaxSum().toString( mArchDoc->locale() ) );
    h = mArchDoc->locale()->formatNumber( mArchDoc->reducedTax() );
    tmpl.setValue( "SECTION_REDUCED_TAX", TAG( "REDUCED_TAX" ), h );
    tmpl.setValue( "SECTION_REDUCED_TAX", TAG( "REDUCED_TAX_LABEL" ), i18n( "reduced VAT" ) );
  }
  if ( mArchDoc->fullTaxSum().toLong() > 0 ) {
    tmpl.createDictionary( DICT( "SECTION_FULL_TAX" ) );
    tmpl.setValue( "SECTION_FULL_TAX", TAG( "FULL_TAX_SUM" ),
      mArchDoc->fullTaxSum().toString( mArchDoc->locale() ) );
    h = mArchDoc->locale()->formatNumber( mArchDoc->tax() );
    tmpl.setValue( "SECTION_FULL_TAX", TAG( "FULL_TAX" ), h );
    tmpl.setValue( "SECTION_FULL_TAX", TAG( "FULL_TAX_LABEL" ), i18n( "VAT" ) );
  }

  h = mArchDoc->locale()->formatNumber( mArchDoc->tax() );
  tmpl.setValue( TAG( "VAT" ), h );

  tmpl.setValue( TAG( "VATSUM" ), mArchDoc->taxSum().toString( mArchDoc->locale() ) );

  // My own contact data

  QString output = tmpl.expand();

  emit templateGenerated( output );

}

#define ADDRESS_TAG( PREFIX, TAG ) QString("%1_%2").arg( PREFIX ).arg( TAG )

void ReportGenerator::contactToTemplate( TextTemplate *tmpl, const QString& prefix, const KABC::Addressee& contact )
{
  if( contact.isEmpty() ) return;

  tmpl->setValue( ADDRESS_TAG( prefix, "NAME" ),  contact.realName() );
  QString co = contact.organization();
  if( co.isEmpty() ) {
    co = contact.realName();
  }
  tmpl->setValue( ADDRESS_TAG( prefix, "ORGANISATION" ), co );
  tmpl->setValue( ADDRESS_TAG( prefix, "URL" ),   contact.url().prettyUrl() );
  tmpl->setValue( ADDRESS_TAG( prefix, "EMAIL" ), contact.preferredEmail() );
  tmpl->setValue( ADDRESS_TAG( prefix, "PHONE" ), contact.phoneNumber( KABC::PhoneNumber::Work ).number() );
  tmpl->setValue( ADDRESS_TAG( prefix, "FAX" ),   contact.phoneNumber( KABC::PhoneNumber::Fax ).number() );
  tmpl->setValue( ADDRESS_TAG( prefix, "CELL" ),  contact.phoneNumber( KABC::PhoneNumber::Cell ).number() );

  KABC::Address address;
  address = contact.address( KABC::Address::Pref );
  if( address.isEmpty() )
    address = contact.address(KABC::Address::Work );
  if( address.isEmpty() )
    address = contact.address(KABC::Address::Home );
  if( address.isEmpty() )
    address = contact.address(KABC::Address::Postal );

  tmpl->setValue( ADDRESS_TAG( prefix, "POSTBOX" ),
              address.postOfficeBox() );

  tmpl->setValue( ADDRESS_TAG( prefix, "EXTENDED" ),
                 address.extended() );
  tmpl->setValue( ADDRESS_TAG( prefix, "STREET" ),
                 address.street() );
  tmpl->setValue( ADDRESS_TAG( prefix, "LOCALITY" ),
                 address.locality() );
  tmpl->setValue( ADDRESS_TAG( prefix, "REGION" ),
                 address.region() );
  tmpl->setValue( ADDRESS_TAG( prefix, "POSTCODE" ),
                 address.postalCode() );
  tmpl->setValue( ADDRESS_TAG( prefix, "COUNTRY" ),
                 address.country() );
  tmpl->setValue( ADDRESS_TAG( prefix, "REGION" ),
                 address.region() );
  tmpl->setValue( ADDRESS_TAG( prefix,"LABEL" ),
                 address.label() );


}


QString ReportGenerator::escapeTrml2pdfXML( const QString& str ) const
{
  return( Qt::escape( str ) );
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

QStringList ReportGenerator::findTrml2Pdf( )
{
  const QString rmlbinDefault = QString::fromLatin1( "trml2pdf" ); // FIXME: how to get the default value?
  QString rmlbin = KraftSettings::self()->trml2PdfBinary();
  kDebug() << "### Start searching rml2pdf bin: " << rmlbin;

  QStringList retList;
  mHavePdfMerge = false;

  if ( rmlbinDefault == rmlbin  ) {
    QString ermlpy = KStandardDirs::locate( "data", "kraft/tools/erml2pdf.py" );
    kDebug() << "Ermlpy: " << ermlpy;
    if( ! ermlpy.isEmpty() ) {
      // need the python interpreter
      // First check for python2 in python3 times. 
      QString python = KStandardDirs::findExe("python2");
      if( python.isEmpty() ) {
	python = KStandardDirs::findExe("python");
      }
      if( python.isEmpty() ) {
        kError() << "ERR: Unable to find python, thats a problem";
      } else {
        kDebug() << "Using python: " << python;
        retList << python;
        retList << ermlpy;
        mHavePdfMerge = true;
      }
    } else {
      // tool erml2pdf.py not found. Try trml2pdf_kraft.sh for legacy reasons
      QString trml2pdf = KStandardDirs::findExe("trml2pdf_kraft.sh");
      if( trml2pdf.isEmpty() ) {
        kDebug() << "Could not find trml2pdf_kraft.sh";
      } else {
        kDebug() << "Found trml2pdf: " << trml2pdf;
        retList << trml2pdf;
        mHavePdfMerge = true;
      }
    }

    if ( ! mHavePdfMerge ) {
      QString trml2pdf = KStandardDirs::findExe( "trml2pdf");
      if( trml2pdf.isEmpty() ) {
        kDebug() << "trml2pdf is also empty, we can not convert rml. Debug!";
      } else {
        kDebug() << "trml2pdf found here: " << trml2pdf;
        retList << trml2pdf;
      }
    }
  }
  if ( retList.isEmpty() ) {
    kDebug() << "We have not found the script!";
  }

  return retList;
}


void ReportGenerator::runTrml2Pdf( const QString& rmlFile, const QString& docID, const QString& archId )
{
  mProcess.clearProgram();

  QStringList prg;

  mErrors = QString();
  // findTrml2Pdf returns a list of command line parts for the converter, such as
  // /usr/bin/pyhton /usr/local/share/erml2pdf.py
  QStringList rmlbin = findTrml2Pdf();

  if ( ! rmlbin.size() ) {

    KMessageBox::error( 0, i18n("The utility to create PDF from the rml file could not be found, "
                                "but is required to create documents."
                                "Please make sure the package is installed accordingly." ),
                        i18n( "Document Generation Error" ) );
    return;
  }

  QApplication::setOverrideCursor( QCursor( Qt::BusyCursor ) );

  if ( mHavePdfMerge && mMergeIdent != "0" &&
       ( mWatermarkFile.isEmpty() || !QFile::exists( mWatermarkFile ) ) ) {
    KMessageBox::error( 0, i18n("The Watermark file to merge with the document could not be found. "
                                "Merge is going to be disabled." ),
                        i18n( "Watermark Error" ) );
    mMergeIdent = "0";
  }

  QString outputDir = ArchiveMan::self()->pdfBaseDir();
  QString filename = ArchiveMan::self()->archiveFileName( docID, archId, "pdf" );
  mFile.setFileName( QString( "%1/%2").arg( outputDir).arg( filename ) );

  kDebug() << "Writing output to " << mFile.fileName();

  // check if we have etrml2pdf
  bool haveErml = false;
  if( rmlbin.size() > 1 ) {
    QString ermlbin = rmlbin[1];
    if( ermlbin.endsWith( "erml2pdf.py") ) {
      haveErml = true;
    }
  }

  prg << rmlbin;

  if( !haveErml ) {
    if ( mHavePdfMerge ) {
        prg << mMergeIdent;
    }
    prg << rmlFile;
    if ( mHavePdfMerge && mMergeIdent != "0" ) {
      prg << mWatermarkFile;
    }
  } else {
    kDebug() << "Erml2pdf available!";
    if ( mHavePdfMerge && mMergeIdent != "0" ) {
      prg << "-m" << mMergeIdent;
      prg << "-w" << mWatermarkFile;
    }
    prg << rmlFile;
  }

  mFile.setFileName( mFile.fileName() );
  mOutputSize = 0;
  if ( mFile.open( QIODevice::WriteOnly ) ) {
    mProcess.setProgram( prg );
    mTargetStream.setDevice( &mFile );
    QStringList args = mProcess.program();
    kDebug() << "* rml2pdf Call-Arguments: " << args;

    mProcess.start( );
  }
}

void ReportGenerator::slotReceivedStdout( )
{
  QByteArray arr  = mProcess.readAllStandardOutput();
  mOutputSize += arr.size();
  mTargetStream.writeRawData( arr, arr.size());
}

void ReportGenerator::slotReceivedStderr( )
{
  QByteArray arr  = mProcess.readAllStandardError();
  mErrors.append( arr );
}

void ReportGenerator::slotError( QProcess::ProcessError err )
{
  mErrors.append( i18n("Program ended with status %1").arg(err));
}

void ReportGenerator::trml2pdfFinished( int exitStatus)
{
  mFile.close();

  kDebug() << "PDF Creation Process finished with status " << exitStatus;
  kDebug() << "Wrote bytes to the output file: " << mOutputSize;
  if ( exitStatus == 0 ) {
    emit pdfAvailable( mFile.fileName() );
    mFile.setFileName( QString() );
  } else {
    if ( mErrors.isEmpty() ) mErrors = i18n( "Unknown problem." );
    // KMessageBox::detailedError (QWidget *parent, const QString &text, const QString &details, const QString &caption=QString::null, int options=Notify)
    KMessageBox::detailedError ( 0,
                                 i18n( "Could not generate the pdf file. The pdf creation script failed." ),
                                 mErrors,
                                 i18n( "PDF Generation Error" ) );
    mErrors = QString();
  }
  QApplication::restoreOverrideCursor();

}


