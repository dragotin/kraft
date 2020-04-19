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
#include "klocalizedstring.h"

#include <QDebug>

#include <string.h>


TextTemplate::TextTemplate()
    :TextTemplateInterface(),
      mStandardDict( nullptr )
{

}

TextTemplate::~TextTemplate()
{
    if (mStandardDict)
        delete mStandardDict;
}

bool TextTemplate::createSubDictionary( const QString& parent, const QString& name )
{
  Dictionary ttd;
  bool re = false;

  if ( mDictionaries.contains( parent ) ) {
    ttd.mDict = mDictionaries[parent]->AddSectionDictionary( name.toAscii().data() );
    ttd.mParent = parent;
    ttd.mName = name;
    mDictionaries[name] = ttd.mDict;
    re = true;
  }
  return re;
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
  TemplateDictionary *dict = nullptr;

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
    dict->SetValue( key.toAscii().data(), val.toStdString() ); // std::string( val.toUtf8() ) );
}

void TextTemplate::setValue( const QString& key, const QString& val )
{
  if ( mStandardDict ) {
    mStandardDict->SetValue( key.toAscii().data(), val.toStdString() );
  }
}

void TextTemplate::setValue( Dictionary ttd, const QString& key, const QString& val )
{
  if ( ttd.mDict ) {
    ( ttd.mDict )->SetValue( key.toAscii().data(), val.toStdString() );
  }
}



bool TextTemplate::initialize()
{

   Template *tmpl = Template::GetTemplate(fileName().toStdString(), ctemplate::DO_NOT_STRIP );

  if ( !tmpl || tmpl->state() != ctemplate::TS_READY ) {
    setError( i18n( "Failed to open template source" ) );
    return false;
  }
  tmpl->ReloadAllIfChanged();

  if (mStandardDict)
      delete mStandardDict;
  mStandardDict = new TemplateDictionary( "TopLevel" );

  return true;
}

QString TextTemplate::expand()
{
    std::string output;

    if ( mStandardDict) {
        bool errorFree = ExpandTemplate( fileName().toStdString(), ctemplate::DO_NOT_STRIP ,mStandardDict, &output );

        QString qout = QString::fromStdString(output);
        qout.remove(QChar(0));

        if ( errorFree ) {
            return qout;
        }
    }
    return QStringLiteral("Unable to expand template");
}



