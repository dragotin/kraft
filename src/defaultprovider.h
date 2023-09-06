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

#include <QtCore>

#include "kraftcat_export.h"

#include "kraftdoc.h"
#include "doctext.h"
#include "docguardedptr.h"
#include "documentsaverxml.h"

class QSqlRecord;
class QStringList;
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
        NumberCycles
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

    // Two methods used for converting document data from db into a directory
    QString createV2BaseDir(const QString &base = QString());
    // after successful conversion this method switches to the new root dir
    bool switchToV2BaseDir(const QString& dirStr);

    // main function: Always use this method to get the path to a subdir
    // for data. The baseDir is only set in test cases
    QString kraftV2Dir(KraftV2Dir dir, const QString &baseDir = QString());

    // utility - returns the name of the subdir for a given enum type
    QString kraftV2Subdir(KraftV2Dir dir);

    DefaultProvider();

    // these are v1 methods
    bool writeXmlArchive();
    QString pdfOutputDir();
    QString xmlArchivePath();

    DocumentSaverBase &documentPersister();

private:
    QString kraftV2BaseDir(const QString &baseDir = QString());

    QLocale _locale;
    DocumentSaverXML _persister;

    const QString EuroTag;

};

#endif
