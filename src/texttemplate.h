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
#include <ctemplate/template_dictionary.h>
#include <ctemplate/template_modifiers.h>

using ctemplate::Template;
using ctemplate::TemplateDictionary;

class TextTemplate
{

public:
  struct Dictionary
  {
    QString mParent;
    QString mName;
    TemplateDictionary *mDict;
  };

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
   * returns true if successful. Otherwise check errorString() for 
   * error messages
   */
  bool setTemplateFileName( const QString& );

  bool open();

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
  void setValue( Dictionary, const QString& , const QString& );

  void createDictionary( const QString& );

  /**
   * creates a sub dictionary to a given dictionary. 
   * Parameter 1 is the parent dict, Param 2 the sub dictionary name.
   */
  Dictionary createSubDictionary( const QString& , const QString& );

  /**
   * creates a dictionary with the name given in parameter two nested 
   * in the parent dictionary given in the first parameter.
   *
   * The dictionary struck is given back to use with setValue.
   */
  Dictionary createSubDictionary( Dictionary, const QString& );
  /**
   * get the expanded output 
   */
  QString expand() const;

  QString findTemplateFile(const QString &filename) const;

private:
  QString mFileName;
  QString mErrorString;
  
  TemplateDictionary *mStandardDict;
  QMap<QString, TemplateDictionary*> mDictionaries;
};

#endif
