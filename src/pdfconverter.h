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

#ifndef PDFCONVERTER_H
#define PDFCONVERTER_H

#include <QObject>
#include <QString>
#include <QProcess>
#include <QDataStream>
#include <QFile>

#include "dbids.h"

class PDFConverter : public QObject
{
    Q_OBJECT
public:
    PDFConverter();

    enum class ConvError { NoError,
                           SourceFileFail,
                           TrmlToolFail,
                           TargetFileError,
                           NoReportLabMod,
                           TargetFileMissing,
                           UnknownError,
                           WeasyPrintNotFound,
                           PDFMergerError
                         };

    virtual void convert(const QString& sourceFile, const QString& outputPath) = 0;

    QString getErrors() { return mErrors; }

signals:
    void docAvailable(const QString& fileName);
    void converterError( ConvError );

protected:
    QString mErrors;
    QProcess *mProcess;
    QFile mFile;

};

// ====================================================================
class ReportLabPDFConverter: public PDFConverter
{
    Q_OBJECT
public:
    ReportLabPDFConverter();

    void convert(const QString& sourceFile, const QString& outputPath) override;

private slots:
    void trml2pdfFinished( int exitCode, QProcess::ExitStatus stat);
    void slotReceivedStdout();
    void slotReceivedStderr();

private:
    QFile mFile;

    QDataStream mTargetStream;
    int mOutputSize;
};

// ====================================================================
class WeasyPrintPDFConverter : public PDFConverter
{
    Q_OBJECT
public:
    WeasyPrintPDFConverter();

    void convert(const QString& sourceFile, const QString& outputPath) override;

private slots:
    void slotReceivedStdout();
    void slotReceivedStderr();
    void weasyPrintFinished(int exitCode, QProcess::ExitStatus stat);

private:
    QByteArray mOutput;
};

#endif // PDFCONVERTER_H
