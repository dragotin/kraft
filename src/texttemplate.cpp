/***************************************************************************
           texttemplate.cpp - fill a template with text tags
                             -------------------
    begin                : Sep 2007
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

#include "texttemplate.h"
#include "ctemplate/template.h"
#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>

#include <QFile>
#include <QFileInfo>

#include <string.h>

TextTemplate::TextTemplate()
  :mStandardDict( 0 )
{

}

TextTemplate::TextTemplate( const QString& name )
  : mFileName( name ),
    mStandardDict( 0 )
{
}

TextTemplate::~TextTemplate()
{
  delete mStandardDict;
}

TextTemplate::Dictionary TextTemplate::createSubDictionary( const QString& parent, const QString& name )
{
  Dictionary ttd;

  if ( mDictionaries.contains( parent ) ) {
    ttd.mDict = mDictionaries[parent]->AddSectionDictionary( name.toAscii().data() );
    ttd.mParent = parent;
    ttd.mName = name;
    mDictionaries[name] = ttd.mDict;
  }
  return ttd;
}

TextTemplate::Dictionary TextTemplate::createSubDictionary( Dictionary parentTtd, const QString& name )
{
  Dictionary ttd;

  if ( parentTtd.mDict ) {
    ttd.mDict = ( parentTtd.mDict )->AddSectionDictionary( name.toAscii().data() );
    ttd.mParent = parentTtd.mName;
    ttd.mName = name;
    // mDictionaries[name] = ttd.mDict;
  }
  return ttd;
}

void TextTemplate::createDictionary( const QString& dictName )
{
  if ( mStandardDict ) {
    mDictionaries[dictName] = mStandardDict->AddSectionDictionary( dictName.toAscii().data() );
    mStandardDict->ShowSection( dictName.toAscii().data() );
  }
}

void TextTemplate::setValue( const QString& dictName, const QString& key, const QString& val )
{
  TemplateDictionary *dict = 0;

  if ( mDictionaries.contains( dictName ) ) {
    dict = mDictionaries[dictName];
  } else {
    if( mStandardDict ) {
      dict = mStandardDict->AddSectionDictionary( dictName.toAscii().data() );
      mDictionaries[dictName] = dict;
      mStandardDict->ShowSection( dictName.toAscii().data() );
    }
  }

  if ( dict )
    dict->SetValue( key.toAscii().data(), std::string( val.toUtf8() ) );
}

void TextTemplate::setValue( const QString& key, const QString& val )
{
  if ( mStandardDict ) {
    mStandardDict->SetValue( key.toAscii().data(), std::string( val.toUtf8() ) );
  }
}

void TextTemplate::setValue( Dictionary ttd, const QString& key, const QString& val )
{
  if ( ttd.mDict ) {
    ( ttd.mDict )->SetValue( key.toAscii().data(), std::string( val.toUtf8() ) );
  }
}

bool TextTemplate::setTemplateFileName( const QString& name )
{
  mErrorString.clear();

  mFileName = name;
  return open();
}

bool TextTemplate::open()
{
  QFileInfo info( mFileName );

  if ( info.isAbsolute() ) {
    // assume it is a absolute path
  } else {
    mFileName = findTemplateFile(mFileName);

    if ( mFileName.isEmpty() ) {
      mErrorString = i18n( "No file name given for template" );
      return false;
    }

    info.setFile( mFileName );
  }

  if ( ! ( info.isFile() && info.isReadable() ) ) {
    mErrorString = i18n( "Could not find template file %1" ).arg( info.absoluteFilePath() );
    return false;
  }

  kDebug() << "Loading this template source file: " << mFileName << endl;

  Template *tmpl = Template::GetTemplate( std::string( mFileName.toUtf8() ), ctemplate::DO_NOT_STRIP );

  if ( !tmpl || tmpl->state() != ctemplate::TS_READY ) {
    mErrorString = i18n( "Failed to open template source" );
    return false;
  }
  tmpl->ReloadAllIfChanged();

  mStandardDict = new TemplateDictionary( "TopLevel" );

  return true;
}


QString TextTemplate::errorString() const
{
  return mErrorString;
}

QString TextTemplate::expand() const
{
  std::string output;

  // if ( mStandardDict ) {
  //   mStandardDict->Dump();
  // }
  Template *textTemplate = Template::GetTemplate( std::string( mFileName.toUtf8() ),
                                                  ctemplate::DO_NOT_STRIP );
  if ( textTemplate && mStandardDict) {
    bool errorFree = textTemplate->Expand(&output, mStandardDict );

    if ( errorFree )
      return QString::fromUtf8( output.c_str() );
  }
  return QString();
}

// Static method to load

QString TextTemplate::findTemplateFile(const QString &filename) const
{
  if( filename.isEmpty() ) {
    return QString::null;
  }

  KStandardDirs stdDirs;
  QString templFileName = filename;
  QString findFile = "kraft/reports/" + templFileName;

  QString tmplFile = stdDirs.findResource( "data", findFile );

  if ( tmplFile.isEmpty() ) {
    QByteArray kraftHome = qgetenv("KRAFT_HOME");

    if( !kraftHome.isEmpty() ) {
      QString file = QString( "%1/reports/%2").arg(QString::fromLocal8Bit(kraftHome)).arg(templFileName);
      QFileInfo fi(file);
      if( fi.exists() && fi.isReadable() ) {
        tmplFile = file;
      }
    }
    if( tmplFile.isEmpty() ) {
      kDebug() << "Could not find template " << filename;
      return QString::null;
    }
  }
  return tmplFile;
}
