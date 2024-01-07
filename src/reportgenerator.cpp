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
#include "docposition.h"
#include "documentman.h"
#include "defaultprovider.h"
#include "doctype.h"
#include "addressprovider.h"
#include "documenttemplate.h"
#include "pdfconverter.h"
#include "xmldocindex.h"

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

// FIXME: This should work by UUID instead of docID
void ReportGenerator::createDocument( ReportFormat format, const QString& uuid)
{
    if (_uuid == uuid) {
        qDebug() << "PDF Creation for doc" << uuid << "is running already, returning";
        return;
    }
    _uuid = uuid;
    _requestedFormat = format;

    if( mProcess && mProcess->state() != QProcess::NotRunning ) {
        qDebug() << "===> WRN: Process still running, try again later.";
        emit failure(uuid, i18n("Document generation process is still running."), "");
        _uuid.clear();
        return;
    }

    KraftDoc *doc = DocumentMan::self()->openDocumentByUuid(uuid);

    // the next call also sets the watermark options
    const QString dt = doc->docType();
    _tmplFile = findTemplateFile( dt );

    if ( _tmplFile.isEmpty() ) {
        qDebug () << "tmplFile is empty, exit reportgenerator!";
        _uuid.clear();
        delete doc;
        return;
    } else {
        qDebug () << "Using this template: " << _tmplFile;
    }

    // ==== Look up the customer contact
    const QString clientUid = doc->addressUid();
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
    delete doc;
    slotAddresseeFound(clientUid, contact);
}

void ReportGenerator::slotAddresseeFound( const QString&, const KContacts::Addressee& contact )
{
    mCustomerContact = contact;
    // now the three pillars archDoc, myContact and mCustomerContact are defined.

    QFileInfo fi(_tmplFile);
    if (!fi.exists()) {
        emit failure(_uuid, i18n("Template file is not accessible."), "");
        _uuid.clear();
        return;
    }
    const QString ext = fi.completeSuffix();

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

    converter->setTemplatePath(fi.path());

    // expand the template...
    const QString expanded = templateEngine->expand(_uuid, myContact, mCustomerContact);
    _cleanupFiles = templateEngine->tempFilesCreated();

    if (expanded.isEmpty()) {
        emit failure(_uuid, i18n("The template conversion failed."), templateEngine->error());
        delete converter;
        _uuid.clear();
        return;
    }
    // ... and save to a tempoarary file
    const QString tempFile = saveToTempFile(expanded);

    if (tempFile.isEmpty()) {
        emit failure(_uuid, i18n("Saving to temporar file failed."), "");
        delete converter;
        _uuid.clear();
        return;
    }
    _cleanupFiles.append(tempFile);

    // Now there is the completed, expanded document source.
    connect( converter, &PDFConverter::docAvailable,
             this, &ReportGenerator::slotPdfDocAvailable);
    connect( converter, &PDFConverter::converterError,
             this, &ReportGenerator::slotConverterError);

    // Always write the finished document to a tmp file. Let it copy over by the
    // mergePdfWatermark func
    QTemporaryFile tmpFile;
    if (tmpFile.open()) {
        tmpFile.close();
        converter->convert(tempFile, QString("%1.pdf").arg(tmpFile.fileName()));
    } else {
        qWarning() << "Can not write to the temporary file" << tmpFile.fileName();
    }

}

void ReportGenerator::slotPdfDocAvailable(const QString& file)
{
    QObject *s = sender();
    qDebug() << "The document is finished:" << file;

    s->deleteLater();

    // Remove tmp files that might have been created during the template expansion,
    // ie. the EPC QR Code SVG file.
#ifndef QT_DEBUG
    for (const auto &subfile : _cleanupFiles) {
        QFile::remove(subfile);
    }
#endif
    _cleanupFiles.clear();

    // check for the watermark requirements
    mergePdfWatermark(file);
}

void ReportGenerator::mergePdfWatermark(const QString& file)
{
    mProcess = new QProcess();
    connect(mProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ReportGenerator::pdfMergeFinished);

    const QString target = targetFileName();
    const QStringList prg = DefaultProvider::self()->locatePythonTool(QStringLiteral("watermarkpdf.py"));

    QStringList args;
    if (mMergeIdent > 0) {
        // check if the watermark file is present. If not, just do not try to merge
        if (prg.isEmpty()) {
            qDebug() << "Can not find the tool watermarkpdf.py";
            mMergeIdent = 0;
        }
        if (mWatermarkFile.isEmpty()) {
            qDebug() << "A watermark file is not set.";
            mMergeIdent = 0;
        } else {
            QFileInfo fi{mWatermarkFile};
            if (!(fi.exists() && fi.isReadable())) {
                qDebug() << "The watermark file" << mWatermarkFile << "does not exist or can not be read.";
                mMergeIdent = 0;
            }
        }
    }

    if (mMergeIdent > 0) {
        // If merging, the result file is directly written to the target file name
        // no copying over is needed from the tmp file.
        mProcess->setProgram(prg.at(0));
        args << prg.at(1);
        args << QStringLiteral("-m") << QString::number(mMergeIdent);
        args << QStringLiteral("-o") << target;
        if (!mPdfAppendFile.isEmpty()) {
            args << QStringLiteral("-a") << mPdfAppendFile;
        }
        args << mWatermarkFile;
        args << file;

        qDebug() << "Merge PDF Watermark args:" << args;
        mProcess->setArguments(args);

        mProcess->start( );
    } else {
        // no watermark is wanted, copy the converted file over.
        if (QFile::copy(file, target)) {
            qDebug() << "Generated file" << file << "copied to" << target;
            pdfMergeFinished(0, QProcess::ExitStatus::NormalExit);
        } else {
            qDebug() << "ERR: Failed to copy temporary file" << file << "to" << target;
            pdfMergeFinished(1, QProcess::ExitStatus::NormalExit);
        }
    }
}

void ReportGenerator::pdfMergeFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    mWatermarkFile.clear();
    mPdfAppendFile.clear();
    mMergeIdent = 0;

    if (exitStatus == QProcess::ExitStatus::NormalExit && exitCode == 0) {
        // remove the temp file which comes as arg in any case, even if the watermark
        // tool was not called.
        if (mProcess->arguments().size() > 0) {
            const QString tmpFile = mProcess->arguments().last();
            QFile::remove(tmpFile);
        }
        mProcess->deleteLater();
        mProcess = nullptr;
        emit docAvailable(_requestedFormat, _uuid, mCustomerContact);
    } else {
        slotConverterError(PDFConverter::ConvError::PDFMergerError);
    }
    _uuid.clear();

}


void ReportGenerator::slotConverterError(PDFConverter::ConvError err)
{
    auto *converter = qobject_cast<PDFConverter*>(sender());

    const QString errors = converter->getErrors();

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
    case PDFConverter::ConvError::NoPyPDFMod:
        errMsg = i18n("The PyPDF2 python module is not installed.");
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
    emit failure(_uuid, errMsg, errors);
    _uuid.clear();
    converter->deleteLater();
}

QString ReportGenerator::targetFileName() const
{
    XmlDocIndex indx;
    const QString fileName = indx.pdfPathByUuid(_uuid).filePath();

    return fileName;
}

QString ReportGenerator::findTemplateFile( const QString& type )
{
    DocType dType( type );
    const QString tmplFile = dType.templateFile();

    if ( tmplFile.isEmpty() ) {
        emit failure(_uuid, i18n("There is not template defined for %1.").arg(dType.name()), "");
    } else {
        // check if file exists
        QFileInfo fi(tmplFile);
        if (!fi.isFile()) {
            emit failure(_uuid, i18n("The template file %1 for document type %2 does not exist.").arg(tmplFile).arg(dType.name()), "");
            return QString();
        }
        if (!fi.isReadable()) {
            emit failure(_uuid, i18n("The template file %1 for document type %2 can not be read.").arg(tmplFile).arg(dType.name()), "");
            return QString();
        }
    }

    mMergeIdent = dType.mergeIdent().toInt();
    mWatermarkFile = dType.watermarkFile();
    mPdfAppendFile = dType.appendPDF();

    return tmplFile;
}

void ReportGenerator::setMyContact( const KContacts::Addressee& contact )
{
    myContact = contact;
}


