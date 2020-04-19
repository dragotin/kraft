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
#include "grantleetemplate.h"
#include "documenttemplate.h"
#include "pdfconverter.h"

namespace {
QString saveToTempFile( const QString& doc )
{
    if ( ! doc.isEmpty() ) {
        QTemporaryFile temp;
        temp.setAutoRemove( false );

        if ( temp.open() ) {
            QTextStream s(&temp);

            // The following explicit coding settings were needed for Qt 4.7.3, former Qt versions
            // seemed to default on UTF-8. Try to comment the following two lines for older Qt versions
            // if needed and see if the trml file on the disk still is UTF-8 encoded.
            QTextCodec *codec = QTextCodec::codecForName("UTF-8");
            s.setCodec( codec );

            s << doc;
            temp.close();
        } else {
            // qDebug () << "ERROR: Could not open temporar file";
        }

        qDebug () << "Wrote rml to " << temp.fileName();

        return temp.fileName();
    }
    return QString();
}

}


ReportGenerator::ReportGenerator()
    : _useGrantlee(true),
      mProcess(nullptr)
{
  mAddressProvider = new AddressProvider(this);
  connect(mAddressProvider, &AddressProvider::lookupResult,
          this, &ReportGenerator::slotAddresseeFound);
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
void ReportGenerator::createDocument( ReportFormat format, const QString& docID, dbID archId )
{
    mDocId = docID;
    mArchId = archId;
    _requestedFormat = format;

    if( mProcess && mProcess->state() != QProcess::NotRunning ) {
        qDebug() << "===> WRN: Process still running, try again later.";
        emit failure(i18n("Document generation process is still running."));
        return;
    }

    // now the addressee search through the address provider is finished.
    // Rendering can be started.
    _archDoc.loadFromDb(archId);

    // the next call also sets the watermark options
    _tmplFile = findTemplateFile( _archDoc.docType() );

    if ( _tmplFile.isEmpty() ) {
        qDebug () << "tmplFile is empty, exit reportgenerator!";
        return;
    } else {
        qDebug () << "Using this template: " << _tmplFile;
    }

    lookupCustomerAddress();
}

void ReportGenerator::lookupCustomerAddress()
{
    const QString clientUid = _archDoc.clientUid();
    KContacts::Addressee contact;

    if( ! clientUid.isEmpty() ) {
        AddressProvider::LookupState state = mAddressProvider->lookupAddressee( clientUid );
        switch( state ) {
        case AddressProvider::LookupFromCache:
            contact = mAddressProvider->getAddresseeFromCache(clientUid);
            break;
        case AddressProvider::LookupNotFound:
        case AddressProvider::ItemError:
        case AddressProvider::BackendError:
            // set an empty contact
            break;
        case AddressProvider::LookupOngoing:
        case AddressProvider::LookupStarted:
            // Not much to do, just wait and let the addressprovider
            // hit the slotAddresseFound
            return;
        }
    }
    slotAddresseeFound(clientUid, contact);
}

void ReportGenerator::slotAddresseeFound( const QString&, const KContacts::Addressee& contact )
{
    mCustomerContact = contact;
    // now the three pillars archDoc, myContact and mCustomerContact are defined.

    QFileInfo fi(_tmplFile);
    if (!fi.exists()) {
        emit failure(i18n("Template file is not accessible."));
    }
    const QString ext = fi.completeSuffix();

    QString output;
    QScopedPointer<DocumentTemplate> templateEngine;
    QPointer<PDFConverter> converter;

    if (QString::compare(ext, QStringLiteral("trml"), Qt::CaseInsensitive) == 0) {
        // use the old ctemplate engine with reportlab.
        templateEngine.reset(new CTemplateDocumentTemplate(_tmplFile));
        converter = new ReportLabPDFConverter;
    } else {
        // use Grantlee.
        templateEngine.reset(new GrantleeDocumentTemplate(_tmplFile));
        converter = new WeasyPrintPDFConverter;
    }

    // expand the template...
    const QString expanded = templateEngine->expand(&_archDoc, myContact, mCustomerContact);

    if (expanded.isEmpty()) {
        emit failure(i18n("The template conversion failed."));
        delete converter;
        return;
    }
    // ... and save to a tempoarary file
    const QString tempFile = saveToTempFile(expanded);

    if (tempFile.isEmpty()) {
        emit failure(i18n("Saving to temporar file failed."));
        delete converter;
        return;
    }

    QString fullOutputPath = targetFileName();

    if (mMergeIdent == "1" || mMergeIdent == "2") {
        // check if the watermark file exists
        QFileInfo fi(mWatermarkFile);
        if (!mWatermarkFile.isEmpty() && fi.isReadable()) {
            QTemporaryFile tmpFile;
            tmpFile.open();
            tmpFile.close();

            // PDF merge is required. Write to temp file
            fullOutputPath = tmpFile.fileName() + QStringLiteral(".pdf");
        } else {
            mMergeIdent = "0";
            qDebug() << "Can not read watermark file, generating without" << mWatermarkFile;
        }
    }

    // Now there is the completed, expanded document source.
    connect( converter, &PDFConverter::docAvailable,
             this, &ReportGenerator::slotPdfDocAvailable);
    connect( converter, &PDFConverter::converterError,
             this, &ReportGenerator::slotConverterError);
    converter->convert(tempFile, fullOutputPath);

}

void ReportGenerator::slotPdfDocAvailable(const QString& file)
{
    QObject *s = sender();
    qDebug() << "The document is finished!:" << file;

    s->deleteLater();
    // check for the watermark requirements
    if (mMergeIdent == "1" || mMergeIdent == "2") {
        // check if the watermark file exists
        mergePdfWatermark(file);
    } else {
        emit docAvailable(_requestedFormat, file, mCustomerContact);
    }
}

void ReportGenerator::mergePdfWatermark(const QString& file)
{
    mProcess = new QProcess();
    connect(mProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ReportGenerator::pdfMergeFinished);

    const QString prg = DefaultProvider::self()->locateKraftTool(QStringLiteral("watermarkpdf.py"));
    if (!prg.isEmpty()) {
        mProcess->setProgram( QStringLiteral("python3") );
        QStringList args;
        args << prg;
        args << QStringLiteral("-m") << mMergeIdent;
        args << QStringLiteral("-o") << targetFileName();
        args << mWatermarkFile;
        args << file;

        mProcess->setArguments(args);

        mProcess->start( );
    }
}

void ReportGenerator::pdfMergeFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::ExitStatus::NormalExit && exitCode == 0) {
        const QString fileName = targetFileName();

        const QString tmpFile = mProcess->arguments().last();
        QFile::remove(tmpFile);
        mProcess->deleteLater();
        mProcess = nullptr;
        emit docAvailable(_requestedFormat, fileName, mCustomerContact);
    } else {
        slotConverterError(PDFConverter::ConvError::PDFMergerError);
    }
}


void ReportGenerator::slotConverterError(PDFConverter::ConvError err)
{
    QObject *s = sender();

    QString errMsg;
    switch(err) {
    case PDFConverter::ConvError::NoError:
         errMsg = i18n("No converter error.");
        break;
    case PDFConverter::ConvError::TrmlToolFail:
        errMsg = i18n("The ReportLab based converter script can not be executed.");
        break;
    case PDFConverter::ConvError::UnknownError:
        errMsg = i18n("An unknown error happened.");
        break;
    case PDFConverter::ConvError::NoReportLabMod:
        errMsg = i18n("The ReportLab python module is not installed.");
        break;
    case PDFConverter::ConvError::SourceFileFail:
        errMsg = i18n("The source file can not be read.");
        break;
    case PDFConverter::ConvError::TargetFileError:
        errMsg = i18n("The target can not be opened to write.");
        break;
    case PDFConverter::ConvError::TargetFileMissing:
        errMsg = i18n("The target file does not exist.");
        break;
    case PDFConverter::ConvError::WeasyPrintNotFound:
        errMsg = i18n("The WeasyPrint tool is not installed.");
        break;
    case PDFConverter::ConvError::PDFMergerError:
        errMsg = i18n("The PDF merger utility failed.");
        break;
    }
    emit failure(errMsg);
    s->deleteLater();
}

QString ReportGenerator::targetFileName() const
{
    const QString filename = ArchiveMan::self()->archiveFileName( mDocId, mArchId.toString(), "pdf" );
    const QString fullOutputPath = QString("%1/%2").arg(ArchiveMan::self()->pdfBaseDir()).arg(filename);

    return fullOutputPath;
}

QString ReportGenerator::findTemplateFile( const QString& type )
{
    DocType dType( type );
    const QString tmplFile = dType.templateFile();

    if ( tmplFile.isEmpty() ) {
        emit failure(i18n("There is not template defined for %1.").arg(dType.name()));
    } else {
        // check if file exists
        QFileInfo fi(tmplFile);
        if (!fi.isFile()) {
            emit failure(i18n("The template file %1 for document type %2 does not exist.").arg(tmplFile).arg(dType.name()));
            return QString();
        }
        if (!fi.isReadable()) {
            emit failure(i18n("The template file %1 for document type %2 can not be read.").arg(tmplFile).arg(dType.name()));
            return QString();
        }
    }

    mMergeIdent = dType.mergeIdent();
    mWatermarkFile = dType.watermarkFile();

    return tmplFile;
}

void ReportGenerator::setMyContact( const KContacts::Addressee& contact )
{
    myContact = contact;
}


