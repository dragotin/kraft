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

  DefaultProvider();

  bool writeXmlArchive();
  QString pdfOutputDir();
  QString xmlArchivePath();

  DocumentSaverBase &documentPersister();

private:

  QLocale _locale;
  DocumentSaverXML _persister;

  const QString EuroTag;

};

#endif
