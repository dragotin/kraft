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

  QString defaultText( const QString&, KraftDoc::Part, DocGuardedPtr = 0 );
  dbID saveDocumentText( const DocText& );
  void deleteDocumentText( const DocText& );

  QString docType(); // the default document type for new docs
  DocTextList documentTexts( const QString&, KraftDoc::Part );

  QString currencySymbol() const;
  QString formatDate( const QDate& date ) const;

  QLocale* locale();

  QString iconvTool() const;

  QString getStyleSheet( const QString& ) const;

  QString locateFile(const QString& findFile) const;

  DefaultProvider();

  static const QString DateFormatIso;
  static const QString DateFormatShort;
  static const QString DateFormatLong;
  static const QString DateFormatRFC;
  static const QString DateFormatGerman;

private:

 //  static DefaultProvider *mSelf;
  QLocale _locale;
  const QString EuroTag;

};

#endif
