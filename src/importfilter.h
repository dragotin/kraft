/***************************************************************************
         importfilter.cpp  - Import positions into Kraft documents
                             -------------------
    begin                : Oct 2008
    copyright            : (C) 2008 by Klaas Freitag
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

// include files for QT

#ifndef _IMPORTFILTER_H
#define _IMPORTFILTER_H

#include <qstring.h>

#include <stdlib.h>

#include "docposition.h"

class ImportFilter
{
public:
  ImportFilter();
  virtual ~ImportFilter() { }

  virtual bool parse();
  virtual QString kdeStdDirPath() const = 0;
  bool readDefinition( const QString& );
  QString error() { return mError; }
  void setStrict( bool _strict ) { mStrict = _strict; }
  bool strict() { return mStrict; }

  QString name() { return mName; }
  QString description() { return mDescription; }

protected:
  bool recode( const QString&, const QString& );

  QString mName;
  QString mDescription;
  QString mEncoding;
  QString mError;
  QString mFilename;
  QString mSeparator;
  QString mTags;
  QStringList mDefinition;
  bool mStrict;
};



class DocPositionImportFilter : public ImportFilter
{
public:
  DocPositionImportFilter( );
  virtual ~DocPositionImportFilter( ) {}

  virtual QString kdeStdDirPath() const;

  bool parseDefinition();

  DocPositionList import( const QUrl& );
  void debugDefinition();

private:
  DocPosition importDocPosition( const QString&, bool& );
  QString replaceCOL( const QStringList&, const QString& );

  QString mAmount;
  QString mText;
  QString mUnit;
  QString mUnitPrice;
};

#endif
