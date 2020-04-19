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
#include <QDataStream>

#include <kcontacts/addressee.h>

#include "kraftdoc.h"
#include "archdoc.h"
#include "pdfconverter.h"

class dbID;
class KJob;
class QFile;
class AddressProvider;
class TextTemplate;

enum class ReportFormat { PDF, PDFMail, HTML };

class ReportGenerator : public QObject
{
    Q_OBJECT

public:
    ReportGenerator();
    ~ReportGenerator();

signals:
    void docAvailable( ReportFormat, const QString& file,
                       const KContacts::Addressee& customerContact);
    void failure(const QString&);

public slots:
    void createDocument(ReportFormat, const QString&, dbID );

    void setMyContact( const KContacts::Addressee& );

private slots:
    void slotPdfDocAvailable(const QString& file);
    void slotConverterError(PDFConverter::ConvError err);
    void mergePdfWatermark(const QString &file);
    void pdfMergeFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QString findTemplateFile( const QString& );

    void lookupCustomerAddress();

    QString _tmplFile;
    ArchDoc _archDoc;
protected:

protected slots:
    void slotAddresseeFound( const QString&, const KContacts::Addressee& );

private:
    void convertTemplate( const QString& );
    void fillupTemplateFromArchive( const dbID& );
    void contactToTemplate( TextTemplate*, const QString&, const KContacts::Addressee& );
    QString registerDictionary( const QString&, const QString& ) const;
    QString registerTag( const QString&, const QString& ) const;
    QString registerDictTag( const QString&, const QString&, const QString& ) const;
    QString targetFileName() const;

    QString escapeTrml2pdfXML( const QString& str ) const;

    QString rmlString( const QString& str, const QString& paraStyle = QString() ) const;

    bool _useGrantlee;

    QString   mErrors;
    QString   mMergeIdent;
    bool      mHavePdfMerge;
    QString   mWatermarkFile;
    QString   mDocId;
    dbID      mArchId;
    long      mOutputSize;

    KContacts::Addressee mCustomerContact;
    KContacts::Addressee myContact;

    QPointer<QProcess> mProcess;

    QFile mFile;
    QDataStream mTargetStream;
    AddressProvider *mAddressProvider;
    ReportFormat _requestedFormat;
};

#endif
