/***************************************************************************
                 attribute.h - generic attribute object
                             -------------------
    begin                : Aug. 2007
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
#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

// include files for Qt
#include <QVariant>

// include files for KDE

// application specific includes
#include "kraftcat_export.h"

/**
@author Klaas Freitag
*/
class QString;
class dbID;

class KRAFTCAT_EXPORT Attribute
{
  friend class AttributeMap;

  public:
    Attribute();
    Attribute( const QString& name );

    void     setValue( const QVariant& var );
    QVariant value();
    QString  name() const;
    bool     persistant();
    bool     listValue();
    void     setListValue( bool );
    void     setPersistant( bool );
    bool     useRelationTable();
    void     setValueRelation( const QString&, const QString&, const QString& );
    QString  toString();
private:
    void     setRawValue( const QVariant& var ); // sets the value without checking for relations
    QString  mName;
    QVariant mValue;
    bool     mPersist;
    bool     mListValue;
    bool     mDelete;  // Delete the attribute on save. Written and read by the attributemap

    QString mTable;
    QString mIdCol;
    QString mStringCol;
};

/*
 * Attribute Map
 */
class KRAFTCAT_EXPORT AttributeMap: public QMap<QString, Attribute>
{
public:
  AttributeMap();
  AttributeMap( const QString& );

  bool hasAttribute( const QString& );

  void setHost( const QString& );

  void load( dbID );
  void save( dbID );

  void markDelete( const QString& );
  void dbDeleteAll( dbID );
protected:
  void dbDeleteAttribute( const QString& );
  void dbDeleteValue( const QString&, const QString& = QString() );

private:
  void checkHost();

  QString mHost;

};


#endif
