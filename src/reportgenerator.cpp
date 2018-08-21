/***************************************************************************
                     Report Generator based on Reportlab
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
#include <QStandardPaths>
#include <QMessageBox>
#include <QDebug>
#include <QUrl>

#include <KLocalizedString>

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

Q_GLOBAL_STATIC(ReportGenerator, mSelf)

ReportGenerator *ReportGenerator::self()
{
  return mSelf;
}

ReportGenerator::ReportGenerator()
  :mProcess(nullptr),
    mArchDoc(nullptr)
{
  mAddressProvider = new AddressProvider( this );
  connect( mAddressProvider, SIGNAL( lookupResult(QString,KContacts::Addressee)),
           this, SLOT( slotAddresseeFound(QString, KContacts::Addressee)));
}

ReportGenerator::~ReportGenerator()
{
  // qDebug () << "ReportGen is destroyed!";
}

/*
 * docID: document ID
 *  dbId: database ID of the archived doc.
 *
 * This is the starting point of a report creation.
 */
void ReportGenerator::createPdfFromArchive( const QString& docID, dbID archId )
{
  mDocId = docID;
  mArchId = archId;

  if( mProcess && mProcess->state() != QProcess::NotRunning ) {
      qDebug() << "===> WRN: Process still running, try again later.";
      return;
  }
  fillupTemplateFromArchive( archId );
  // Method fillupTemplateFromArchive raises a signal when the archive was
  // generated. This signal gets connected to slotConvertTemplate in the
  // constructor of this class.
}

void ReportGenerator::convertTemplate( const QString& templ )
{
  // qDebug() << "Report BASE:\n" << templ;

  if ( ! templ.isEmpty() ) {
    QTemporaryFile temp;
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
      // qDebug () << "ERROR: Could not open temporar file";
    }

    // qDebug () << "Wrote rml to " << temp.fileName();

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
  const QString country = mArchDoc->locale()->bcp47Name();
  const QString tmplFile = dType.templateFile(country);

  if ( tmplFile.isEmpty() ) {
      QMessageBox msgBox;
      msgBox.setText(i18n("A document template named %1 could not be loaded. ", dType.templateFile()));
      msgBox.setInformativeText(i18n("Please check your installation!"));
      msgBox.setStandardButtons(QMessageBox::Ok);
      msgBox.exec();
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
        AddressProvider::LookupState state = mAddressProvider->lookupAddressee( clientUid );
        KContacts::Addressee contact;
        switch( state ) {
        case AddressProvider::LookupFromCache:
            contact = mAddressProvider->getAddresseeFromCache(clientUid);
            slotAddresseeFound(clientUid, contact);
            break;
        case AddressProvider::LookupNotFound:
        case AddressProvider::ItemError:
        case AddressProvider::BackendError:
            // set an empty contact
            slotAddresseeFound(clientUid, contact);
            break;
        case AddressProvider::LookupOngoing:
        case AddressProvider::LookupStarted:
            // Not much to do, just wait and let the addressprovider
            // hit the slotAddresseFound
            break;
        }
    } else {
        // no address UID specified, skip the addressee search and generate the template directly
        slotAddresseeSearchFinished(0);
    }
}

void ReportGenerator::slotAddresseeFound( const QString&, const KContacts::Addressee& contact )
{
    mCustomerContact = contact;
    slotAddresseeSearchFinished(0);
}

void ReportGenerator::setMyContact( const KContacts::Addressee& contact )
{
  myContact = contact;
}

void ReportGenerator::slotAddresseeSearchFinished( int )
{
    qDebug() << "** Reached slotAddresseeSearchFinished!";

  // now the addressee search through the address provider is finished.
  // Rendering can be started.
  QString tmplFile = findTemplate( mArchDoc->docType() );

  if ( tmplFile.isEmpty() ) {
    // qDebug () << "tmplFile is empty!";
    return;
  } else {
    // qDebug () << "Reading this template: " << tmplFile;
  }

  // create a text template
  TextTemplate tmpl( tmplFile );
  if( !tmpl.open() ) {
      // qDebug () << "ERROR: Unable to open document template " << tmplFile;
      QMessageBox msgBox;
      msgBox.setText(i18n("The template file could not be opened: %1\n ", tmplFile));
      msgBox.setInformativeText(i18n("Please check the setup and the doc type configuration."));
      msgBox.setWindowTitle(i18n("Template Error"));
      msgBox.setStandardButtons(QMessageBox::Ok);
      msgBox.exec();

      return;
  }

  /* replace the placeholders */
  /* A placeholder has the format <!-- %VALUE --> */

  const ArchDocPositionList posList = mArchDoc->positions();
  QString h;

  ArchDocPositionList::const_iterator it;
  int specialPosCnt = 0;
  int taxFreeCnt    = 0;
  int reducedTaxCnt = 0;
  int fullTaxCnt    = 0;

  bool individualTax = false;
  /* Check for the tax settings: If the taxType is not the same for all items,
   * we have individual Tax setting and show the tax marker etc.
   */
  DocPositionBase::TaxType ttype = DocPositionBase::TaxInvalid;
  for ( it = posList.begin(); it != posList.end(); ++it ) {
    ArchDocPosition pos (*it);
    if( ttype == DocPositionBase::TaxInvalid  ) {
      ttype = pos.taxType();
    } else {
      if( ttype != pos.taxType() ) { // different from previous one?
        individualTax = true;
        break;
      }
    }
  }

  /* now loop over the items to fill the template structures */
  for ( it = posList.begin(); it != posList.end(); ++it ) {
    ArchDocPosition pos (*it);
    tmpl.createDictionary( "POSITIONS" );
    tmpl.setValue( "POSITIONS", TAG( "POS_NUMBER" )
                   , pos.posNumber() );
    tmpl.setValue( "POSITIONS", "POS_TEXT",
                   rmlString( pos.text(), QString( "%1text" ).arg( pos.kind().toLower() ) ) );

    // format the amount value of the item, do not show the precision if there is no fraction
    double amount = pos.amount();
    QString num;
    num.setNum( amount ); // no locale awareness.
    int prec = 0;
    if( num.contains( QChar('.') ) ) {
      // there is a decimal point
      // calculate the precision
      prec = num.length() - (1+num.lastIndexOf( QChar('.') ) );
    }
    // qDebug() << "**** " << num << " has precision " << prec;
    h = mArchDoc->locale()->toString( amount, 'f', prec );

    tmpl.setValue( "POSITIONS", "POS_AMOUNT", h );
    tmpl.setValue( "POSITIONS", "POS_UNIT", escapeTrml2pdfXML( pos.unit() ) );
    tmpl.setValue( "POSITIONS", "POS_UNITPRICE", pos.unitPrice().toString( mArchDoc->locale() ) );
    tmpl.setValue( "POSITIONS", "POS_TOTAL", pos.nettoPrice().toString( mArchDoc->locale() ) );
    tmpl.setValue( "POSITIONS", "POS_KIND", pos.kind().toLower() );

    QString taxType;

    if( individualTax ) {
      if( pos.taxType() == 1 ) {
        taxFreeCnt++;
        taxType = "TAX_FREE";
      } else if( pos.taxType() == 2 ) {
        reducedTaxCnt++;
        taxType = "REDUCED_TAX";
      } else {
        // ATTENTION: Default for all non known tax types is full tax.
        fullTaxCnt++;
        taxType = "FULL_TAX";
      }

      tmpl.createSubDictionary( "POSITIONS", taxType );
    }

    /* item kind: Normal, alternative or demand item. For normal items, the kind is empty.
     */
    if ( !pos.kind().isEmpty() ) {
      specialPosCnt++;
    }
  }
  if ( specialPosCnt ) {
    tmpl.createDictionary( "SPECIAL_POS" );
    tmpl.setValue( "SPECIAL_POS", "COUNT", QString::number( specialPosCnt ) );
  }

  /*
   * Just show the tax index if we have multiple tax settings
   */
  if( individualTax ) {
    tmpl.createDictionary( "TAX_FREE_ITEMS" );
    tmpl.setValue( "TAX_FREE_ITEMS", "COUNT", QString::number( taxFreeCnt ));

    tmpl.createDictionary( "REDUCED_TAX_ITEMS" );
    tmpl.setValue( "REDUCED_TAX_ITEMS", "COUNT", QString::number( reducedTaxCnt ));
    tmpl.setValue( "REDUCED_TAX_ITEMS", "TAX", mArchDoc->locale()->toString( mArchDoc->reducedTax()) );

    tmpl.createDictionary( "FULL_TAX_ITEMS" );
    tmpl.setValue( "FULL_TAX_ITEMS", "COUNT", QString::number( fullTaxCnt ));
    tmpl.setValue( "FULL_TAX_ITEMS", "TAX", mArchDoc->locale()->toString( mArchDoc->tax()) );
  }

  /* now replace stuff in the whole document */
  tmpl.setValue( TAG( "DATE" ), mArchDoc->locale()->toString(mArchDoc->date(), QLocale::NarrowFormat) );
  tmpl.setValue( TAG( "DOCTYPE" ), escapeTrml2pdfXML( mArchDoc->docType() ) );
  tmpl.setValue( TAG( "ADDRESS" ), escapeTrml2pdfXML( mArchDoc->address() ) );

  contactToTemplate( &tmpl, "CLIENT", mCustomerContact );
  contactToTemplate( &tmpl, "MY", myContact );

  tmpl.setValue( TAG( "DOCID" ),   escapeTrml2pdfXML( mArchDoc->ident() ) );
  tmpl.setValue( TAG( "PROJECTLABEL" ),   escapeTrml2pdfXML( mArchDoc->projectLabel() ) );
  tmpl.setValue( TAG( "SALUT" ),   escapeTrml2pdfXML( mArchDoc->salut() ) );
  tmpl.setValue( TAG( "GOODBYE" ), escapeTrml2pdfXML( mArchDoc->goodbye() ) );
  tmpl.setValue( TAG( "PRETEXT" ),   rmlString( mArchDoc->preText() ) );
  tmpl.setValue( TAG( "POSTTEXT" ),  rmlString( mArchDoc->postText() ) );
  tmpl.setValue( TAG( "BRUTTOSUM" ), mArchDoc->bruttoSum().toString( mArchDoc->locale() ) );
  tmpl.setValue( TAG( "NETTOSUM" ),  mArchDoc->nettoSum().toString( mArchDoc->locale() ) );

  h = mArchDoc->locale()->toString( mArchDoc->tax() );
  // qDebug () << "Tax in archive document: " << h;
  if ( mArchDoc->reducedTaxSum().toLong() > 0 ) {
    tmpl.createDictionary( DICT( "SECTION_REDUCED_TAX" ) );
    tmpl.setValue( "SECTION_REDUCED_TAX", TAG( "REDUCED_TAX_SUM" ),
      mArchDoc->reducedTaxSum().toString( mArchDoc->locale() ) );
    h = mArchDoc->locale()->toString( mArchDoc->reducedTax() );
    tmpl.setValue( "SECTION_REDUCED_TAX", TAG( "REDUCED_TAX" ), h );
    tmpl.setValue( "SECTION_REDUCED_TAX", TAG( "REDUCED_TAX_LABEL" ), i18n( "reduced VAT" ) );
  }
  if ( mArchDoc->fullTaxSum().toLong() > 0 ) {
    tmpl.createDictionary( DICT( "SECTION_FULL_TAX" ) );
    tmpl.setValue( "SECTION_FULL_TAX", TAG( "FULL_TAX_SUM" ),
      mArchDoc->fullTaxSum().toString( mArchDoc->locale() ) );
    h = mArchDoc->locale()->toString( mArchDoc->tax() );
    tmpl.setValue( "SECTION_FULL_TAX", TAG( "FULL_TAX" ), h );
    tmpl.setValue( "SECTION_FULL_TAX", TAG( "FULL_TAX_LABEL" ), i18n( "VAT" ) );
  }

  h = mArchDoc->locale()->toString( mArchDoc->tax() );
  tmpl.setValue( TAG( "VAT" ), h );

  tmpl.setValue( TAG( "VATSUM" ), mArchDoc->taxSum().toString( mArchDoc->locale() ) );

  // My own contact data

  const QString output = tmpl.expand();
  convertTemplate(output);
}

#define ADDRESS_TAG( PREFIX, TAG ) QString("%1_%2").arg( PREFIX ).arg( TAG )

void ReportGenerator::contactToTemplate( TextTemplate *tmpl, const QString& prefix, const KContacts::Addressee& contact )
{
  if( contact.isEmpty() ) return;

  tmpl->setValue( ADDRESS_TAG( prefix, "NAME" ),  escapeTrml2pdfXML( contact.realName() ) );
  QString co = contact.organization();
  if( co.isEmpty() ) {
    co = contact.realName();
  }
  tmpl->setValue( ADDRESS_TAG( prefix, "ORGANISATION" ), escapeTrml2pdfXML( co ) );
  tmpl->setValue( ADDRESS_TAG( prefix, "URL" ),   escapeTrml2pdfXML( contact.url().toString() ) );
  tmpl->setValue( ADDRESS_TAG( prefix, "EMAIL" ), escapeTrml2pdfXML( contact.preferredEmail() ) );
  tmpl->setValue( ADDRESS_TAG( prefix, "PHONE" ), escapeTrml2pdfXML( contact.phoneNumber( KContacts::PhoneNumber::Work ).number() ) );
  tmpl->setValue( ADDRESS_TAG( prefix, "FAX" ),   escapeTrml2pdfXML( contact.phoneNumber( KContacts::PhoneNumber::Fax ).number() ) );
  tmpl->setValue( ADDRESS_TAG( prefix, "CELL" ),  escapeTrml2pdfXML( contact.phoneNumber( KContacts::PhoneNumber::Cell ).number() ) );

  KContacts::Address address;
  address = contact.address( KContacts::Address::Pref );
  if( address.isEmpty() )
    address = contact.address(KContacts::Address::Work );
  if( address.isEmpty() )
    address = contact.address(KContacts::Address::Home );
  if( address.isEmpty() )
    address = contact.address(KContacts::Address::Postal );

  tmpl->setValue( ADDRESS_TAG( prefix, "POSTBOX" ),
                  escapeTrml2pdfXML( address.postOfficeBox() ) );

  tmpl->setValue( ADDRESS_TAG( prefix, "EXTENDED" ),
                  escapeTrml2pdfXML( address.extended() ) );
  tmpl->setValue( ADDRESS_TAG( prefix, "STREET" ),
                  escapeTrml2pdfXML( address.street() ) );
  tmpl->setValue( ADDRESS_TAG( prefix, "LOCALITY" ),
                  escapeTrml2pdfXML( address.locality() ) );
  tmpl->setValue( ADDRESS_TAG( prefix, "REGION" ),
                  escapeTrml2pdfXML( address.region() ) );
  tmpl->setValue( ADDRESS_TAG( prefix, "POSTCODE" ),
                  escapeTrml2pdfXML( address.postalCode() ) );
  tmpl->setValue( ADDRESS_TAG( prefix, "COUNTRY" ),
                  escapeTrml2pdfXML( address.country() ) );
  tmpl->setValue( ADDRESS_TAG( prefix, "REGION" ),
                  escapeTrml2pdfXML( address.region() ) );
  tmpl->setValue( ADDRESS_TAG( prefix,"LABEL" ),
                  escapeTrml2pdfXML( address.label() ) );


}

QString ReportGenerator::escapeTrml2pdfXML( const QString& str ) const
{
  return( str.toHtmlEscaped() );
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
  // qDebug () << "Returning " << rml;
  return rml;
}

QStringList ReportGenerator::findTrml2Pdf( )
{
  const QString rmlbinDefault = QString::fromLatin1( "trml2pdf" ); // FIXME: how to get the default value?
  QString rmlbin = KraftSettings::self()->trml2PdfBinary();
  // qDebug () << "### Start searching rml2pdf bin: " << rmlbin;

  QStringList retList;
  mHavePdfMerge = false;

  if ( rmlbinDefault == rmlbin  ) {
    QString ermlpy = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kraft/tools/erml2pdf.py" );
    // qDebug () << "Ermlpy: " << ermlpy;
    if( ! ermlpy.isEmpty() ) {
      // need the python interpreter
      // First check for python2 in python3 times.
      QString python = QStandardPaths::findExecutable(QLatin1String("python2"));
      if( python.isEmpty() ) {
        python = QStandardPaths::findExecutable(QLatin1String("python"));
      }
      if( python.isEmpty() ) {
        qCritical() << "ERR: Unable to find python, thats a problem";
      } else {
        // qDebug () << "Using python: " << python;
        retList << python;
        retList << ermlpy;
        mHavePdfMerge = true;
      }
    } else {
      // tool erml2pdf.py not found. Check in $KRAFT_HOME/tools
      QString p = QString::fromUtf8(qgetenv("KRAFT_HOME"));
      if( !p.isEmpty() ) {
          p += QLatin1String("/tools/erml2pdf.py");
          // qDebug () << "Found erml2pdf from KRAFT_HOME: " << p;
          if( QFile::exists( p ) ) {
              retList << "python";
              retList << p;
              mHavePdfMerge = true;
          }
      } else {
          // tool erml2pdf.py not found. Try trml2pdf_kraft.sh for legacy reasons
          QString trml2pdf = QStandardPaths::findExecutable(QLatin1String("trml2pdf_kraft.sh"));
          if( trml2pdf.isEmpty() ) {
              // qDebug () << "Could not find trml2pdf_kraft.sh";
          } else {
              // qDebug () << "Found trml2pdf: " << trml2pdf;
              retList << trml2pdf;
              mHavePdfMerge = true;
          }
      }
    }

    if ( ! mHavePdfMerge ) {
      QString trml2pdf = QStandardPaths::findExecutable(QLatin1String("trml2pdf"));
      if( trml2pdf.isEmpty() ) {
        // qDebug () << "trml2pdf is also empty, we can not convert rml. Debug!";
      } else {
        // qDebug () << "trml2pdf found here: " << trml2pdf;
        retList << trml2pdf;
      }
    }
  }
  if ( retList.isEmpty() ) {
    // qDebug () << "We have not found the script!";
  }

  return retList;
}


void ReportGenerator::runTrml2Pdf( const QString& rmlFile, const QString& docID, const QString& archId )
{
    mErrors.clear();
    // findTrml2Pdf returns a list of command line parts for the converter, such as
    // /usr/bin/pyhton /usr/local/share/erml2pdf.py
    QStringList rmlbin = findTrml2Pdf();

    if ( ! rmlbin.size() ) {
        mErrors = i18n("The utility to create PDF from the rml file could not be found, "
                       "but is required to create documents.\n");
        mErrors += i18n("Please make sure the package is installed accordingly.");

        trml2pdfFinished(1, QProcess::NormalExit);
    }

    QApplication::setOverrideCursor( QCursor( Qt::BusyCursor ) );

    if ( mHavePdfMerge && mMergeIdent != "0" &&
         ( mWatermarkFile.isEmpty() || !QFile::exists( mWatermarkFile ) ) ) {
        QMessageBox msgBox;
        msgBox.setText(i18n("The Watermark file to merge with the document could not be found. "
                            "Merge is going to be disabled."));
        msgBox.setWindowTitle(i18n("Watermark Error"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        mMergeIdent = "0";
    }

    const QString outputDir = ArchiveMan::self()->pdfBaseDir();
    const QString filename = ArchiveMan::self()->archiveFileName( docID, archId, "pdf" );
    mFile.setFileName( QString( "%1/%2").arg( outputDir).arg( filename ) );

    // qDebug () << "Writing output to " << mFile.fileName();

    // check if we have etrml2pdf
    bool haveErml = false;
    QStringList args;

    if( rmlbin.size() > 1 ) {
        QString ermlbin = rmlbin[1];
        if( ermlbin.endsWith( "erml2pdf.py") ) {
            haveErml = true;
        }
        args.append(rmlbin.at(1));
    }

    const QString prg = rmlbin.at(0);

    if( !haveErml ) {
        if ( mHavePdfMerge ) {
            args << mMergeIdent;
        }
        args << rmlFile;
        if ( mHavePdfMerge && mMergeIdent != "0" ) {
            args << mWatermarkFile;
        }
    } else {
        // qDebug () << "Erml2pdf available!";
        if ( mHavePdfMerge && mMergeIdent != "0" ) {
            args << "-m" << mMergeIdent;
            args << "-w" << mWatermarkFile;
        }
        args << rmlFile;
    }

    mOutputSize = 0;
    if ( mFile.open( QIODevice::WriteOnly ) ) {
        qDebug() << "Converting " << rmlFile << "using" << prg << args.join(QChar(' '));
        mProcess = new QProcess(this);
        connect(mProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(slotReceivedStdout()));
        connect(mProcess, SIGNAL(readyReadStandardError()), this, SLOT(slotReceivedStderr()));
        connect(mProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this,
                SLOT(trml2pdfFinished(int,QProcess::ExitStatus)));

        mProcess->setProgram( prg );
        mProcess->setArguments(args);
        mTargetStream.setDevice( &mFile );

        mProcess->start( );
    } else {
        mErrors = i18n("The file to save the PDF could not be written in folder %1\n", outputDir);
        mErrors += i18n("Please make sure that the output folder exists and is writeable.");
        trml2pdfFinished(1, QProcess::NormalExit);

    }
}

void ReportGenerator::slotReceivedStdout( )
{
    QByteArray arr  = mProcess->readAllStandardOutput();
    mOutputSize += arr.size();
    mTargetStream.writeRawData( arr.data(), arr.size());
}

void ReportGenerator::slotReceivedStderr( )
{
    QByteArray arr  = mProcess->readAllStandardError();
    mErrors.append( arr );
}

void ReportGenerator::slotError( QProcess::ProcessError err )
{
    mErrors.append( i18n("Program ended with status %1", err));
}

void ReportGenerator::trml2pdfFinished( int exitCode, QProcess::ExitStatus stat)
{
    if( mFile.isOpen() ) {
        mFile.close();
    }
    Q_UNUSED(stat);

    // qDebug () << "PDF Creation Process finished with status " << exitStatus;
    // qDebug () << "Wrote bytes to the output file: " << mOutputSize;
    if ( exitCode == 0 ) {
        emit pdfAvailable( mFile.fileName() );
        if( mProcess) {
            const QString rmlFile = mProcess->arguments().last(); // the file name of the temp rmlfile
            QFile::remove(rmlFile); // remove the rmlFile
        }
    } else {
        if( mErrors.contains(QLatin1String("No module named Reportlab"))) {
            mErrors = i18n("To generate PDF output, Kraft requires the python module ReportLab which can not be found.\n\n"
                           "Please make sure the package is installed on your computer.");
        }
        if( mErrors.contains(QLatin1String("No module named pyPdf"))) {
            mErrors = i18n("To generate PDF output, Kraft requires the python module pyPdf which can not be found.\n\n"
                           "Please make sure the package is installed on your computer.");
        }

        if ( mErrors.isEmpty() ) mErrors = i18n( "Unknown problem." );

        QMessageBox msgBox;
        msgBox.setText(i18n("The PDF output file could not be generated. The creation script failed.") );
        msgBox.setDetailedText(mErrors);
        msgBox.setWindowTitle(i18n("PDF Generation Error"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        mErrors.clear();
    }

    if(mProcess) {
        mProcess->deleteLater();
        mProcess = nullptr;
    }
    mFile.setFileName( QString() );

    QApplication::restoreOverrideCursor();
}


