/***************************************************************************
                 doctype.h - doc type class
                             -------------------
    begin                : Oct. 2007
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
#ifndef DOCTYPE_H
#define DOCTYPE_H

// include files for Qt
#include <qstring.h>
#include <qmap.h>

#include "kraftcat_export.h"

#include "attribute.h"
#include "dbids.h"

// application specific includes

/**
@author Klaas Freitag
*/

typedef QMap<QString, dbID> idMap;
class KraftDoc;

class KRAFTCAT_EXPORT DocType
{
  public:
  DocType();
  /** 
   * create a doctype from its localised or tech name 
   */
  DocType( const QString&, bool dirty = false );
  DocType& operator=( const DocType& );

  static QStringList all();
  static QStringList allLocalised();
  static dbID docTypeId( const QString& );
  
  QString name() const;
  void setName( const QString& );

  bool allowDemand();
  bool allowAlternative();
  bool pricesVisible();

  QStringList follower();

  QString     generateDocumentIdent( KraftDoc* doc, int id = -1 );
  QString     identTemplate();
  void        setIdentTemplate( const QString& );

  QString     numberCycleName();
  void        setNumberCycleName( const QString& );

  QString     defaultTemplateFile() const;
  QString     templateFile( const QString& language = QString() );
  void        setTemplateFile( const QString& );

  QString     watermarkFile();
  void        setWatermarkFile( const QString& );

  QString     mergeIdent();
  void        setMergeIdent( const QString& );

  static void  clearMap();

  int         nextIdentId( bool hot = true );
  void        save();

  void        readIdentTemplate();

  protected:
  void        readFollowerList();

  private:
  static void init();

private:
  AttributeMap mAttributes;
  QStringList  mFollowerList;
  QString      mName;
  QString      mIdentTemplate;
  bool         mDirty;
  QString      mMergeIdent;

  static idMap mNameMap;
};

#endif
