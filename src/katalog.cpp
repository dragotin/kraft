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
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>

#include "floskeltemplate.h"
#include "dbids.h"
#include "katalog.h"
#include "kraftdb.h"
#include "unitmanager.h"
#include "zeitcalcpart.h"
#include "fixcalcpart.h"
#include "materialcalcpart.h"
#include "kraftdb.h"

/**
 *  constructor of a katalog, which is only a list of Floskel templates.
 *  A name must be given, which is displayed for the root element in the
 *
 */


Katalog::Katalog(const QString& name):
    m_name(name),
    m_setID(-1),
    m_readOnly( false )
{
    init();
}

Katalog::Katalog()
{
    init();
}

void Katalog::init()
{
    // FIXME: Catalogs could have their own locale in the future
  mLocale = KGlobal::locale();
}

Katalog::~Katalog()
{
}

/**
 * virtuell load method for catalogs. Must be overwritten.
 */
int Katalog::load()
{  
  QSqlQuery q;
  q.prepare("SELECT * FROM CatalogSet WHERE name = :name");
  q.bindValue(":name", m_name);
  q.exec();

  if( q.next() ) {
    m_description = q.value(2).toString();
    m_setID = q.value(0).toInt();
    kDebug() << "Setting catalogSetID=" << QString( m_setID ) << " from name " << m_name << endl;
  }
  return 0;
}

QStringList Katalog::getKatalogChapters( bool freshup )
{
  if( m_chapters.empty() || freshup ) {

    m_chapters.clear();
    m_chapterIDs.clear();

    QSqlQuery q;
    q.prepare("SELECT * FROM CatalogChapters WHERE catalogSetId = :catalogSetId ORDER BY sortKey");
    q.bindValue(":catalogSetId", m_setID);
    q.exec();
    kDebug() << "Selecting chapters for catalog no " << QString::number( m_setID ) << endl;

    while ( q.next() )
    {
      QString katName = q.value(2).toString();
      int katID = q.value(0).toInt();
      kDebug() << "Adding catalog chapter " << katName << " with ID " << katID << endl;
      m_chapters.append(katName);
      dbID id( katID );
      m_chapterIDs.insert(katName, id);
    }
  }

  return m_chapters;
}

int Katalog::chapterID(const QString& chapter)
{
  if( m_chapterIDs.size() == 0 ) {
    // fill up the dict of ids if still empty.
    getKatalogChapters();
  }

  dbIdDict::iterator it = m_chapterIDs.find(chapter);
  if( it != m_chapterIDs.end() )
    return it.value().intID();
  else
    return -1;
}

QString Katalog::chapterName(const dbID& id)
{
  if( m_chapterIDs.size() == 0 )
  {
    // fill up the dict of ids if still empty.
    getKatalogChapters();
  }

  dbIdDictIterator i( m_chapterIDs );
  while (i.hasNext()) {
    i.next();
    if ( i.value() == id ) {
      return i.key();
    }
  }

  return QString("not found!");
}

QString Katalog::getName() const
{
    return m_name;
}

void Katalog::setName( const QString& n )
{
    m_name = n;
}

KatalogType Katalog::type()
{
    return UnspecCatalog;
}

void Katalog::addChapter( const QString& name, int sortKey )
{
  kDebug() << "Inserting new chapter " << name << sortKey << endl;
  QSqlQuery q;
  q.prepare("INSERT INTO CatalogChapters (catalogSetID, chapter, sortKey)"
            "VALUES(:catalogSetID, :chapter, :sortKey)");
  q.bindValue( ":catalogSetID", m_setID );
  q.bindValue( ":chapter", name );
  q.bindValue( ":sortKey", sortKey );
  q.exec();
}

bool Katalog::removeChapter( const QString& name, const QString& )
{
  kDebug() << "Deleting chapter " << name << endl;
  QSqlQuery q;
  q.prepare("DELETE FROM CatalogChapters WHERE catalogSetId = :catalogSetId AND chapter = :chapter");
  q.bindValue(":catalogSetID", m_setID);
  q.bindValue(":chapter", name);

  return false;
}

void Katalog::renameChapter( const QString& from, const QString& to )
{
  kDebug() << "Rename chapter " << from << " to " << to << endl;
  QSqlQuery q;
  q.prepare("UPDATE CatalogChapters SET (chapter = :newchapter) WHERE catalogSetId = :catalogSetId AND chapter = :oldchapter");
  q.bindValue(":catalogSetID", m_setID);
  q.bindValue(":oldchapter", from);
  q.bindValue(":newchapter", to);
  q.exec();
}

void Katalog::setChapterSortKey( const QString& chap, int key )
{
  kDebug() << "Set chapter sortKey for " << chap << " to " << key << endl;
  QSqlQuery q;
  q.prepare("UPDATE CatalogChapters SET (sortKey = :sortKey) WHERE catalogSetId = :catalogSetId AND chapter = :chapter");
  q.bindValue(":catalogSetID", m_setID);
  q.bindValue(":chapter", chap);
  q.bindValue(":sortKey", key);
  q.exec();
}

int Katalog::chapterSortKey( const QString& chap )
{
  int key = -1;
  QSqlQuery q;
  q.prepare("SELECT sortKey FROM catalogChapters WHERE chapter = :chapter");
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

#if 0
int Katalog::getEntriesPerChapter( const QString& )
{

}
#endif
