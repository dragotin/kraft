/***************************************************************************
                    reportgenerator.h - report generation
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
#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H

#include <QFile>
#include <QObject>
#include <QProcess>
#include <QPointer>
#include <QDataStream>

#include <kcontacts/addressee.h>

#include "pdfconverter.h"

class dbID;
class KJob;
class QFile;
class AddressProvider;

enum class ReportFormat { PDF, PDFMail, HTML };

class ReportGenerator : public QObject
{
    Q_OBJECT

public:
    ReportGenerator();
    ~ReportGenerator();

Q_SIGNALS:
    void docAvailable( ReportFormat, const QString& file,
                       const KContacts::Addressee& customerContact);
    void failure(const QString&, const QString&, const QString&);

public Q_SLOTS:
    void createDocument(ReportFormat, const QString&uuid);

private Q_SLOTS:
    void slotPdfDocAvailable(const QString& file);
    void slotConverterError(PDFConverter::ConvError err);
    void mergePdfWatermark(const QString &file);
    void pdfMergeFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QString findTemplateFile( const QString& );

    void lookupCustomerAddress();

    QString _tmplFile;
protected:
    QStringList _cleanupFiles;

protected Q_SLOTS:
    void slotAddresseeFound( const QString&, const KContacts::Addressee& );

private:
    void convertTemplate( const QString& );
    QString registerDictionary( const QString&, const QString& ) const;
    QString registerTag( const QString&, const QString& ) const;
    QString registerDictTag( const QString&, const QString&, const QString& ) const;
    QString targetFileName() const;

    QString escapeTrml2pdfXML( const QString& str ) const;

    QString rmlString( const QString& str, const QString& paraStyle = QString() ) const;

    bool _useGrantlee;

    QString   mErrors;
    int       mMergeIdent;
    bool      mHavePdfMerge;
    QString   mWatermarkFile;
    QString   mPdfAppendFile;
    QString   _uuid;
    long      mOutputSize;

    KContacts::Addressee mCustomerContact;

    QPointer<QProcess> mProcess;

    QFile mFile;
    QDataStream mTargetStream;
    AddressProvider *mAddressProvider;
    ReportFormat _requestedFormat;
};

#endif
