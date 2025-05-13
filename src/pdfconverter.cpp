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

#include <QObject>
#include <QTemporaryFile>
#include <QTextStream>
#include <QDebug>
#include <QApplication>
#include <QProcess>



void PDFConverter::slotReceivedStderr( )
{
    QByteArray arr  = mProcess->readAllStandardError();
    mErrors.append( arr );
}

// ====================================================================

ReportLabPDFConverter::ReportLabPDFConverter()
    :PDFConverter()
{

}



void ReportLabPDFConverter::convert(const QString& sourceFile, const QString &outputFile)
{
    // qDebug() << "Report BASE:\n" << templ;

    if ( sourceFile.isEmpty() ) {
        return;
    }

    // findTrml2Pdf returns a list of command line parts for the converter, such as
    // /usr/bin/pyhton3 /usr/local/share/erml2pdf.py
    QStringList rmlbin = DefaultProvider::self()->locatePythonTool("erml2pdf.py");

    if ( ! rmlbin.size() ) {
      Q_EMIT converterError(ConvError::TrmlToolFail);
    }

    QApplication::setOverrideCursor( QCursor( Qt::BusyCursor ) );

    QStringList args;

    QString prg = rmlbin.at(0);

    if( rmlbin.size() > 1 ) {
        // something like "python3 erml2pdf.py
        args.append(rmlbin.at(1));
    }

    args.append(sourceFile);

    mFile.setFileName(outputFile);
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

        if (!mProcess->waitForStarted(1000)) {
            Q_EMIT converterError(ConvError::TrmlToolFail);
        }
    }
}

void ReportLabPDFConverter::slotReceivedStdout( )
{
    QByteArray arr  = mProcess->readAllStandardOutput();
    mOutputSize += arr.size();
    mTargetStream.writeRawData( arr.data(), arr.size());
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
            Q_EMIT docAvailable( mFile.fileName() );
            if( mProcess) {
                const QString rmlFile = mProcess->arguments().last(); // the file name of the temp rmlfile
                QFile::remove(rmlFile); // remove the rmlFile
            }
        } else {
            Q_EMIT  converterError(ConvError::TargetFileMissing);
        }
    } else {
        if( mErrors.contains(QLatin1String("No module named 'reportlab"))) {
            Q_EMIT converterError(ConvError::NoReportLabMod);
        } else if (mErrors.contains("No module named 'PyPDF2")){
            Q_EMIT converterError(ConvError::NoPyPDFMod);
        } else {
            qDebug() << "Trml2Pdf Error:" << mErrors;
            Q_EMIT converterError(ConvError::UnknownError);
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
  // Version string of version 55: WeasyPrint version 55.0
  //                               WeasyPrint version 56.1


}

void WeasyPrintPDFConverter::convert(const QString& sourceFile, const QString& outputFile)
{
    mErrors.clear();

    const QString prg = DefaultProvider::self()->locateBinary("weasyprint");
    const QString styleSheet = DefaultProvider::self()->locateFile("reports/kraft.css");

    QFileInfo prgInfo(prg);
    if ( ! prgInfo.exists() || ! prgInfo.isExecutable() ) {
        Q_EMIT converterError(ConvError::WeasyPrintNotFound);
        return;
    }

    mFile.setFileName(outputFile);

    QApplication::setOverrideCursor( QCursor( Qt::BusyCursor ) );

    mProcess = new QProcess;
    connect(mProcess, &QProcess::readyReadStandardOutput, this, &WeasyPrintPDFConverter::slotReceivedStdout);
    connect(mProcess, &QProcess::readyReadStandardError,  this, &WeasyPrintPDFConverter::slotReceivedStderr);
    connect(mProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &WeasyPrintPDFConverter::weasyPrintFinished);

    QStringList args;
    QFileInfo styleFI(styleSheet);
    const QString styleSheetDir = styleFI.canonicalPath();

    args << "-p";
    args << "-u";
    args << styleSheetDir;
    if (!_templatePath.isEmpty() && _templatePath != styleSheetDir) {
        args << "-u";
        args << _templatePath;
    }
    args << sourceFile;
    args << mFile.fileName();

    qDebug() << "Calling converter:" << prg << args;
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

void WeasyPrintPDFConverter::weasyPrintFinished( int exitCode, QProcess::ExitStatus stat)
{
    if( mFile.isOpen() ) {
        mFile.close();
    }
    Q_UNUSED(stat)
    if (mProcess == nullptr) {
        qDebug() << "Unexpected: mProcess uninitialized!";
        Q_EMIT converterError(ConvError::UnknownError);
        return;
    }
    QApplication::restoreOverrideCursor();

    // qDebug () << "PDF Creation Process finished with status " << exitStatus;
    // qDebug () << "Wrote bytes to the output file: " << mOutputSize;
    if ( exitCode == 0 ) {
        QFileInfo fi(mFile.fileName());
        if( fi.exists() ) {
            Q_EMIT docAvailable( mFile.fileName() );
            const QString htmlFile = mProcess->arguments().first(); // the file name of the temp rmlfile
            QFile::remove(htmlFile); // remove the rmlFile
        } else {
            Q_EMIT  converterError(ConvError::TargetFileMissing);
        }
    } else {
        qDebug() << "Weasyprint failed: " << mProcess->arguments();
        qDebug() << "Weasyprint error output:" << mErrors;
        Q_EMIT converterError(ConvError::WeasyPrintRunFail);
    }
    mProcess->deleteLater();
    mProcess = nullptr;

    mFile.setFileName( QString() );
}

