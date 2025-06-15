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
	    const auto args = mProcess->arguments();
            const QString htmlFile = args.first(); // the file name of the temp rmlfile
            QFile::remove(htmlFile); // remove the rmlFile
        } else {
            Q_EMIT  converterError(ConvError::TargetFileMissing);
        }
    } else {
        qDebug() << "Weasyprint failed: " << mProcess->arguments();
        qDebug() << "Weasyprint error output:" << mErrors;
        Q_EMIT converterError(ConvError::WeasyPrintRunFail);
    }
    if (mProcess) {
        mProcess->deleteLater();
        mProcess = nullptr;
    }

    mFile.setFileName( QString() );
}

