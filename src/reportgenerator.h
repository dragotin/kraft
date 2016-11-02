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

#include <kprocess.h>

#include "kraftdoc.h"
#include "archdoc.h"

class dbID;
class KJob;
class QFile;
class AddressProvider;
class TextTemplate;

class ReportGenerator : public QObject
{
    Q_OBJECT

public:
    ReportGenerator();
    ~ReportGenerator();

    static ReportGenerator *self();

    void runTrml2Pdf( const QString&, const QString&, const QString& );
    QStringList findTrml2Pdf();

signals:
    void pdfAvailable( const QString& );
    void templateGenerated( const QString& );

public slots:
    void createPdfFromArchive( const QString&, dbID );
    void setMyContact( const KContacts::Addressee& );

protected:

protected slots:
    void trml2pdfFinished( int );
    void slotReceivedStdout();
    void slotReceivedStderr();
    void slotError( QProcess::ProcessError );
    void slotConvertTemplate( const QString& );
    void slotAddresseeFound( const QString&, const KContacts::Addressee& );
    void slotAddresseeSearchFinished( int );

private:
    void fillupTemplateFromArchive( const dbID& );
    QString findTemplate( const QString& );
    void contactToTemplate( TextTemplate*, const QString&, const KContacts::Addressee& );
    QString registerDictionary( const QString&, const QString& ) const;
    QString registerTag( const QString&, const QString& ) const;
    QString registerDictTag( const QString&, const QString&, const QString& ) const;


    QString escapeTrml2pdfXML( const QString& str ) const;

    QString rmlString( const QString& str, const QString& paraStyle = QString() ) const;


    QString   mErrors;
    QString   mMergeIdent;
    bool      mHavePdfMerge;
    QString   mWatermarkFile;
    QString   mDocId;
    dbID      mArchId;
    long      mOutputSize;

    KContacts::Addressee mCustomerContact;
    KContacts::Addressee myContact;

    QFile mFile;
    QDataStream mTargetStream;
    ArchDoc *mArchDoc;
    QProcess mProcess;
    AddressProvider *mAddressProvider;
};

#endif
