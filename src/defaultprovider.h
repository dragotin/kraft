/***************************************************************************
                 defaultprovider.h  - Defaults for this and that
                             -------------------
    begin                : November 2006
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
#ifndef DEFAULTPROVIDER_H
#define DEFAULTPROVIDER_H

#include "kraftcat_export.h"

#include "kraftdoc.h"
#include "doctext.h"
#include "docguardedptr.h"
#include "documentsaverxml.h"

class dbID;

/**
 * encapsulates all relevant for default values for documents such as
 * texts etc.
 */
class KRAFTCAT_EXPORT DefaultProvider
{
public:
    enum class KraftV2Dir {
        Root,
        XmlDocs,
        PdfDocs,
        NumberCycles,
        OwnIdentity
    };

    ~DefaultProvider();

    static DefaultProvider *self();

    QIcon icon(const QString& name);

    QString defaultText( const QString&, KraftDoc::Part, DocGuardedPtr = 0 );
    dbID saveDocumentText( const DocText& );
    void deleteDocumentText( const DocText& );

    QString docType(); // the default document type for new docs
    DocTextList documentTexts( const QString&, KraftDoc::Part );

    QString currencySymbol() const;

    QLocale* locale();

    QString iconvTool() const;
    QStringList locatePythonTool(const QString& toolName) const;
    QString locateBinary(const QString& name) const;
    QString locateFile(const QString& findFile) const;

    QString getStyleSheet( const QString& ) const;

    // -- Functions for the new file based Kraft version 2 --

    /**
     *  Important:
     * The KraftV2BaseDir is always the dir, which contains the versions and the
     * current link to the current data dir.
     */

    // Two methods used for converting document data from db into a directory
    QString createV2BaseDir(const QString &base = QString());
    // after successful conversion this method switches to the new root dir
    // it writes to the config file and permanently changes the v2 dir
    bool switchToV2BaseDir(const QString& dirStr);

    // main function: Always use this method to get the path to a subdir
    // for data. The baseDir is only set in test cases
    QString kraftV2Dir(KraftV2Dir dir = KraftV2Dir::Root);

    // utility - returns the name of the subdir for a given enum type
    QString kraftV2Subdir(KraftV2Dir dir);

    // returns the path to the directory where vcd files are stored as file based addressbook
    // if Akonadi is not enabled
    QString kraftV2AddressDir();

    DefaultProvider();

    // these are v1 methods
    bool writeXmlArchive();
    QString pdfOutputDir();
    QString xmlArchivePath();

    DocumentSaverBase &documentPersister();

private:

    QLocale _locale;
    DocumentSaverXML _persister;
    static QString _v2BaseDir;

    const QString EuroTag;

};

#endif
