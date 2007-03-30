/***************************************************************************
             doctext.cpp  - texts like header or footer for documents
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

#include <kdebug.h>
#include <klocale.h>

#include "doctext.h"


DocText::DocText()
  : mTextType( Unknown )
{
  
}

void DocText::setText( const QString& t )
{
  mText = t;
}

void DocText::setDescription( const QString& t )
{
  mDescription = t;
}

void DocText::setDocType( const QString& t )
{
  mDocType = t;
}

void DocText::setTextType( DocText::TextType t )
{
  mTextType = t;
}

DocText::TextType DocText::stringToTextType( const QString& str )
{
  TextType tt = Unknown;
  
  if ( str == i18n( "Header" ) ) tt = DocText::Header;
  if ( str == i18n( "Footer" ) ) tt = DocText::Footer;

  return tt;
}

QString DocText::textTypeToString( DocText::TextType tt )
{
  if ( tt == DocText::Header ) return i18n( "Header" );
  if ( tt == DocText::Footer ) return i18n( "Footer" );
  return i18n( "Unknown" );
}
