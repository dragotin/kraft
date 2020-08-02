/***************************************************************************
              pdfconverter.cpp - convert documents to pdf
                             -------------------
    begin                : March 2020
    copyright            : (C) 2020 by Klaas Freitag
    email                : kraft@freisturz.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "pdfconverter.h"
#include "defaultprovider.h"
#include "archiveman.h"

#include <QObject>
#include <QTemporaryFile>
#include <QTextCodec>
#include <QTextStream>
#include <QDebug>
#include <QApplication>
#include <QProcess>

PDFConverter::PDFConverter()
    : QObject()
{

}

// ====================================================================


ReportLabPDFConverter::ReportLabPDFConverter()
    :PDFConverter()
{

}

void ReportLabPDFConverter::convert(const QString& sourceFile, const QString &outputPath)
{
    // qDebug() << "Report BASE:\n" << templ;

    if ( sourceFile.isEmpty() ) {
        return;
    }

    // findTrml2Pdf returns a list of command line parts for the converter, such as
    // /usr/bin/pyhton3 /usr/local/share/erml2pdf.py
    QStringList rmlbin = DefaultProvider::self()->findTrml2Pdf();

    if ( ! rmlbin.size() ) {
      emit converterError(ConvError::TrmlToolFail);
    }

    QApplication::setOverrideCursor( QCursor( Qt::BusyCursor ) );


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

    if( haveErml ) {
        args << sourceFile;

        mFile.setFileName(outputPath);
        mOutputSize = 0;
        if ( mFile.open( QIODevice::WriteOnly ) ) {
            qDebug() << "Converting " << mFile.fileName() << "using" << prg << args.join(QChar(' '));
            mProcess = new QProcess();
            connect(mProcess, &QProcess::readyReadStandardOutput, this, &ReportLabPDFConverter::slotReceivedStdout);
            connect(mProcess, &QProcess::readyReadStandardError,  this, &ReportLabPDFConverter::slotReceivedStderr);
            connect(mProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                    this, &ReportLabPDFConverter::trml2pdfFinished);

            mProcess->setProgram( prg );
            mProcess->setArguments(args);
            mTargetStream.setDevice( &mFile );

            mProcess->start( );
        } else {
            emit converterError(ConvError::TargetFileError);
        }
    }
}

void ReportLabPDFConverter::slotReceivedStdout( )
{
    QByteArray arr  = mProcess->readAllStandardOutput();
    mOutputSize += arr.size();
    mTargetStream.writeRawData( arr.data(), arr.size());
}

void ReportLabPDFConverter::slotReceivedStderr( )
{
    QByteArray arr  = mProcess->readAllStandardError();
    mErrors.append( arr );
}

void ReportLabPDFConverter::trml2pdfFinished( int exitCode, QProcess::ExitStatus stat)
{
    if( mFile.isOpen() ) {
        mFile.close();
    }
    Q_UNUSED(stat)
    QApplication::restoreOverrideCursor();

    // qDebug () << "PDF Creation Process finished with status " << exitStatus;
    // qDebug () << "Wrote bytes to the output file: " << mOutputSize;
    if ( exitCode == 0 ) {
        QFileInfo fi(mFile.fileName());
        if( fi.exists() ) {
            emit docAvailable( mFile.fileName() );
            if( mProcess) {
                const QString rmlFile = mProcess->arguments().last(); // the file name of the temp rmlfile
                QFile::remove(rmlFile); // remove the rmlFile
            }
        } else {
            emit  converterError(ConvError::TargetFileMissing);
        }
    } else {
        if( mErrors.contains(QLatin1String("No module named Reportlab"))) {
            emit converterError(ConvError::NoReportLabMod);
        } else {
            qDebug() << "Trml2Pdf Error:" << mErrors;
            emit converterError(ConvError::UnknownError);
        }
    }
    mProcess->deleteLater();
    mProcess = nullptr;

    mFile.setFileName( QString() );
}


// ====================================================================

WeasyPrintPDFConverter::WeasyPrintPDFConverter()
    :PDFConverter()
{

}

void WeasyPrintPDFConverter::convert(const QString& sourceFile, const QString& outputPath)
{
    mErrors.clear();

    const QString prg = DefaultProvider::self()->locateBinary("weasyprint");
    const QString styleSheet = DefaultProvider::self()->locateFile("reports/kraft.css");

    QFileInfo prgInfo(prg);
    if ( ! prgInfo.exists() || ! prgInfo.isExecutable() ) {
        emit converterError(ConvError::WeasyPrintNotFound);
    }

    mFile.setFileName(outputPath);

    QApplication::setOverrideCursor( QCursor( Qt::BusyCursor ) );

    mProcess = new QProcess;
    connect(mProcess, &QProcess::readyReadStandardOutput, this, &WeasyPrintPDFConverter::slotReceivedStdout);
    connect(mProcess, &QProcess::readyReadStandardError,  this, &WeasyPrintPDFConverter::slotReceivedStderr);
    connect(mProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &WeasyPrintPDFConverter::weasyPrintFinished);

    QStringList args;
    QFileInfo styleFI(styleSheet);
    const QString styleSheetDir = styleFI.canonicalPath();

    args << sourceFile;
    args << mFile.fileName();
    args << "-u";
    args << styleSheetDir;

    mProcess->setProgram( prg );
    mProcess->setArguments(args);
    mOutput.clear();

    mProcess->start( );


}

void WeasyPrintPDFConverter::slotReceivedStdout( )
{
    QByteArray arr  = mProcess->readAllStandardOutput();
    mOutput.append(arr);
}

void WeasyPrintPDFConverter::slotReceivedStderr( )
{
    QByteArray arr  = mProcess->readAllStandardError();
    mErrors.append( arr );
}

void WeasyPrintPDFConverter::weasyPrintFinished( int exitCode, QProcess::ExitStatus stat)
{
    if( mFile.isOpen() ) {
        mFile.close();
    }
    Q_UNUSED(stat)
    QApplication::restoreOverrideCursor();

    // qDebug () << "PDF Creation Process finished with status " << exitStatus;
    // qDebug () << "Wrote bytes to the output file: " << mOutputSize;
    if ( exitCode == 0 ) {
        QFileInfo fi(mFile.fileName());
        if( fi.exists() ) {
            emit docAvailable( mFile.fileName() );
            if( mProcess) {
                const QString htmlFile = mProcess->arguments().first(); // the file name of the temp rmlfile
                QFile::remove(htmlFile); // remove the rmlFile
            }
        } else {
            emit  converterError(ConvError::TargetFileMissing);
        }
    } else {
        if( mErrors.contains(QLatin1String("No module named Reportlab"))) {
            emit converterError( ConvError::NoReportLabMod);
        } else {
            qDebug() << "Failed: " << mProcess->arguments();
            emit converterError( ConvError::UnknownError);
        }
    }
    mProcess->deleteLater();
    mProcess = nullptr;

    mFile.setFileName( QString() );
}

