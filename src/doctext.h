/***************************************************************************
             doctext.h  - texts like header or footer for documents
                             -------------------
    begin                : March 2007
    copyright            : (C) 2007 by Klaas Freitag
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
#ifndef DOCTEXT_H
#define DOCTEXT_H

#include <QList>

#include "kraftcat_export.h"

#include "kraftdoc.h"
#include "dbids.h"

class QTreeWidgetItem;
class QPixmap;

class KRAFTCAT_EXPORT DocText
{
public:

  DocText();

  QString name() const { return mName; }
  void setName( const QString& );

  QString text() const { return mText; }
  void setText( const QString& );

  QString description() const { return mDescription; }
  void setDescription( const QString& );

  KraftDoc::Part type() { return mTextType; }
  QString textTypeString() const {
    return textTypeToString( mTextType );
  }

  bool isStandardText() const;

  void setTextType( KraftDoc::Part );
  KraftDoc::Part textType() const { return mTextType; }

  QString docType() const { return mDocType; }
  void setDocType( const QString& );

  QTreeWidgetItem *listViewItem() const {
    return mCurrentItem;
  }

  void setListViewItem( QTreeWidgetItem *item ) {
    mCurrentItem = item;
  }

  void setDbId( long );
  void setDbId( const dbID& );
  dbID dbId() const;

  static KraftDoc::Part stringToTextType( const QString& );
  static QString  textTypeToString( KraftDoc::Part );

  bool operator==( const DocText& ) const;
private:
  QString  mName;
  QString  mText;
  QString  mDescription;
  QString  mDocType;
  KraftDoc::Part mTextType;
  QTreeWidgetItem *mCurrentItem;
  dbID     mDbId;
};

typedef QList<DocText> DocTextList;

#endif

