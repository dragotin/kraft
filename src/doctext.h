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

#include <qvaluelist.h>

class DocText
{
public:
  enum TextType { Unknown, Header, Footer };
  
  DocText();

  QString text() const { return mText; }
  void setText( const QString& );

  QString description() const { return mDescription; }
  void setDescription( const QString& );

  TextType type() { return mTextType; }
  void setTextType( TextType );

  QString docType() const { return mDocType; }
  void setDocType( const QString& );

  static TextType stringToTextType( const QString& );
  static QString  textTypeToString( DocText::TextType );
  
private:
  QString  mText;
  QString  mDescription;
  QString  mDocType;
  TextType mTextType;
};

typedef QValueList<DocText> DocTextList;

#endif

