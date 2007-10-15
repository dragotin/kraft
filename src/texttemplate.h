/***************************************************************************
              texttemplate.h - fill a template with text tags
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
#ifndef TEXTTEMPLATE_H
#define TEXTTEMPLATE_H

#include <qmap.h>
#include <qstring.h>
#include <google/template_dictionary.h>
#include <google/template_modifiers.h>

using google::Template;
using google::TemplateDictionary;

class TextTemplate
{

public:
  TextTemplate();
  /**
   * constructs a text template and loads the template source file
   * via KStandardDirs immediately. No need to call setTemplateFilename
   * and openTemlate().
   */
  TextTemplate( const QString& );
  
  ~TextTemplate();

  /**
   * take the template absolute filename of the template source and 
   * load it immediately. 
   * returns true if successfull. Otherwise check errorString() for 
   * error messages
   */
  bool setTemplateFileName( const QString& );

  /** 
   * return a describing string if something went wrong when opening
   * the template.
   */
  QString errorString() const;

  /**
   * set a value in the default dictionary
   */
  void setValue( const QString&, const QString& );
  
  /**
   * set a value in the named dictionary
   * Parameters: 
   * the parameter group name 
   * the key name
   * the value
   */
  void setValue( const QString&, const QString&, const QString& );

  void createDictionary( const QString& );
  /**
   * get the expanded output 
   */
  QString expand() const;

private:
  QString mFileName;
  QString mErrorString;

  bool openTemplate();
  
  Template *mTemplate;
  TemplateDictionary *mStandardDict;
  QMap<QString, TemplateDictionary*> mDictionaries;
};

#endif
