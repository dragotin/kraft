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
#include <qvariant.h>

// include files for KDE

// application specific includes

/**
@author Klaas Freitag
*/
class QString;
class dbID;

class Attribute
{
  public:
    Attribute();
    Attribute( const QString& name );
    
    void     setValue( const QVariant& var );
    QVariant value() const;
    QString  name() const;
    bool     persistant();
    void     setPersistant( bool );

  private:
    QString  mName;
    QVariant mValue;
    bool     mPersist;
};

/*
 * Attribute Map
 */
class AttributeMap: public QMap<QString, Attribute>
{
public:
  AttributeMap();
  AttributeMap( const QString& );

  bool hasAttribute( const QString& );

  void setHost( const QString& );

  void load( dbID );
  void save( dbID );

private:
  void checkHost();

  QString mHost;

};


#endif
