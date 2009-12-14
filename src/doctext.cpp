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
#include <kiconloader.h>

#include <qpixmap.h>

#include "doctext.h"


DocText::DocText()
  : mTextType( KraftDoc::Unknown ),
    mCurrentItem( 0 )
{

}

void DocText::setName( const QString& t )
{
  mName = t;
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

void DocText::setTextType( KraftDoc::Part t )
{
  mTextType = t;
}

bool DocText::isStandardText() const
{
  return ( mName == i18n( "Standard" ) ); // can surely be improved...
}

QPixmap DocText::pixmap() const
{
  if ( isStandardText() ) {
    return SmallIcon( "get-hot-new-stuff" );
  } else {
    return QPixmap();
  }
}

KraftDoc::Part DocText::stringToTextType( const QString& str )
{
  KraftDoc::Part tt = KraftDoc::Unknown;

  if ( str == textTypeToString( KraftDoc::Header ) ) tt = KraftDoc::Header;
  if ( str == textTypeToString( KraftDoc::Footer ) ) tt = KraftDoc::Footer;
  if ( str == textTypeToString( KraftDoc::Positions ) ) tt = KraftDoc::Positions;
  return tt;
}

QString DocText::textTypeToString( KraftDoc::Part tt )
{
  if ( tt == KraftDoc::Header ) return i18n( "Header Text" );
  if ( tt == KraftDoc::Footer ) return i18n( "Footer Text" );
  if ( tt == KraftDoc::Positions ) return i18n( "Positions" );

  return i18n( "Unknown" );
}

bool DocText::operator==( const DocText& _dt ) const
{
  return ( ( mName == _dt.mName ) &&
           ( mDocType == _dt.mDocType ) &&
           ( mTextType == _dt.mTextType ) );
}

void DocText::setDbId( long id )
{
  mDbId = id ;
}

void DocText::setDbId( const dbID& id )
{
  mDbId = id ;
}

dbID DocText::dbId() const
{
  return mDbId;
}

