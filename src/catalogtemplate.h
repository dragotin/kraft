/***************************************************************************
      catalogtemplate - template base class for catalog data
                             -------------------
    begin                : Oct 2007
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
#ifndef CATALOGTEMPLATE_H
#define CATALOGTEMPLATE_H

/**
 * base class that is the base for all templates in kraft catalogs.
 */
#include <QtCore>

#include "kraftcat_export.h"
#include "dbids.h"

class QWidget;
class CatalogSelection;
class Katalog;
class Geld;
class Einheit;

class KRAFTCAT_EXPORT CatalogTemplate
{
public:
  typedef enum { Unknown, ManualPrice, Calculation, AutoCalc } CalculationType;

  CatalogTemplate();
  virtual ~CatalogTemplate();

  virtual bool save() = 0;

  virtual Geld unitPrice() = 0;

  CalculationType calcKind();
  void setCalculationType( CalculationType t );
  QString calcKindString() const ;
  int sortKey() { return mSortKey; }
  void setSortKey( int k ) { mSortKey = k; }

  void setEnterDate( const QDateTime& );
  QDateTime enterDate();

  void setModifyDate( const QDateTime& );
  QDateTime modifyDate();

  void setLastUsedDate( const QDateTime& );
  QDateTime lastUsedDate();

  void setUseCounter( int );
  int useCounter();

  QString getText() const;
  void setText( const QString& );

  void setChapterId( const dbID&, bool );

  dbID chapterId();

  Einheit unit() const;
  void setUnitId(int id);

protected:
  virtual void saveChapterId();

  CalculationType m_calcType;
  int  mSortKey;
  int  mUseCounter;
  dbID mChapterId;  // the chapter (==parent) of the item

  QDateTime mEntered;
  QDateTime mLastModified;
  QDateTime mLastUsed;
  QString mText;

private:
  int mUnitId;
};

class KRAFTCAT_EXPORT CatalogTemplateList : public QList<CatalogTemplate*>
{
public:
  CatalogTemplateList();
  virtual ~CatalogTemplateList();

protected:
  // int compareItems( QPtrCollection::Item, QPtrCollection::Item );
  virtual int compareItems( CatalogTemplate*, CatalogTemplate* );
};

typedef QListIterator<CatalogTemplate*> CatalogTemplateListIterator;

#endif

