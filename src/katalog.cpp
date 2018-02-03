/***************************************************************************
                     katalog.cpp  - Abstrakte Katalogklasse
                             -------------------
    begin                : Son Feb 8 2004
    copyright            : (C) 2004 by Klaas Freitag
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

#include <qdom.h>
#include <QSqlQuery>
#include <QDebug>

#include "floskeltemplate.h"
#include "dbids.h"
#include "katalog.h"
#include "kraftdb.h"
#include "unitmanager.h"
#include "timecalcpart.h"
#include "fixcalcpart.h"
#include "materialcalcpart.h"
#include "defaultprovider.h"

#include <klocalizedstring.h>

/**
 *  constructor of a katalog, which is only a list of Floskel templates.
 *  A name must be given, which is displayed for the root element in the
 *
 */


Katalog::Katalog(const QString& name):
    m_name(name),
    m_setID(-1),
    m_readOnly( false ),
    mChapterListNeedsRefresh( true )
{
    init();
}

Katalog::Katalog():
    m_setID(-1),
    m_readOnly( false ),
    mChapterListNeedsRefresh( true )
{
    init();
}

void Katalog::init()
{
    // FIXME: Catalogs could have their own locale in the future
  mLocale = DefaultProvider::self()->locale();
}

Katalog::~Katalog()
{
}

/**
 * virtuell load method for catalogs.
 */
int Katalog::load()
{
//  CREATE TABLE CatalogSet(
//    catalogSetID INTEGER PRIMARY KEY ASC autoincrement,
//    name         VARCHAR(255),
//    description  VARCHAR(255),
//    catalogType  VARCHAR(64),
//    sortKey      INT NOT NULL
//  );
//
  QSqlQuery q;
  q.prepare("SELECT catalogSetID, description FROM CatalogSet WHERE name = :name");
  q.bindValue(":name", m_name);
  q.exec();

  if( q.next() ) {
    m_setID = q.value(0).toInt();
    m_description = q.value(1).toString();
    // qDebug () << "Setting catalogSetID=" <<  m_setID << " from name " << m_name << endl;
  }
  return 0;
}

QList<CatalogChapter> Katalog::getKatalogChapters( bool freshup )
{
  if( mChapters.empty() || freshup || mChapterListNeedsRefresh ) {

    mChapters.clear();

    //    CREATE TABLE CatalogChapters(
    //            chapterID INTEGER PRIMARY KEY ASC autoincrement,
    //            catalogSetID INT NOT NULL,
    //            chapter      VARCHAR(255),
    //            sortKey      INT NOT NULL
    //    );
    QSqlQuery q;
    q.prepare("SELECT chapterID, chapter, parentChapter, description FROM CatalogChapters WHERE "
              "catalogSetId = :catalogSetId ORDER BY parentChapter, sortKey");
    q.bindValue(":catalogSetId", m_setID);
    q.exec();
    // qDebug () << "Selecting chapters for catalog no " << QString::number( m_setID ) << endl;

    while ( q.next() )
    {
      int chapID          = q.value(0).toInt();
      QString chapterName = q.value(1).toString();
      int parentChapter   = q.value(2).toInt();
      QString desc        = q.value(3).toString();

      // qDebug () << "Adding catalog chapter " << chapterName << " with ID " << chapID << endl;
      CatalogChapter c( chapID, m_setID, chapterName, parentChapter, desc );
      mChapters.append( c );
    }
    mChapterListNeedsRefresh = false;
  }

  return mChapters;
}

QString Katalog::chapterName(const dbID& id)
{
  foreach( CatalogChapter chapter, mChapters ) {
    if( chapter.id() == id ) {
      return chapter.name();
    }
  }
  return i18n("not found");
}

dbID Katalog::chapterID( const QString& name )
{
    foreach( CatalogChapter chapter, mChapters ) {
      if( chapter.name() == name ) {
        return chapter.id();
      }
    }
    return dbID();
}

QString Katalog::getName() const
{
    return m_name;
}

void Katalog::setName( const QString& n )
{
    m_name = n;
}

// Needs reimplementation in the inherited catalogs
KatalogType Katalog::type()
{
    return UnspecCatalog;
}

void Katalog::refreshChapterList()
{
  mChapterListNeedsRefresh = true;
}

void Katalog::setChapterSortKey( const QString& chap, int key )
{
  // qDebug () << "Set chapter sortKey for " << chap << " to " << key << endl;
  QSqlQuery q;
  q.prepare("UPDATE CatalogChapters SET sortKey = :sortKey WHERE catalogSetID = :catalogSetID AND chapter = :chapter");
  q.bindValue(":catalogSetID", m_setID);
  q.bindValue(":chapter", chap);
  q.bindValue(":sortKey", key);
  q.exec();
}

int Katalog::chapterSortKey( const QString& chap )
{
  int key = -1;
  QSqlQuery q;
  q.prepare("SELECT sortKey FROM CatalogChapters WHERE chapter = :chapter");
  q.bindValue(":chapter", chap);
  q.exec();

  if(q.next())
  {
    key = q.value(0).toInt();
  }
  return key;
}

QDomDocument Katalog::toXML()
{
    return QDomDocument();
}

void Katalog::writeXMLFile()
{

}

dbID Katalog::id()
{
  return dbID( m_setID );
}
