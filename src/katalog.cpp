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

#include <qsqlcursor.h>
#include <qdom.h>
#include <kdebug.h>

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
    m_chapterIDs = new dbIdDict();
    m_chapterIDs->setAutoDelete(true);
}

Katalog::~Katalog()
{
    if( m_chapterIDs )
        delete m_chapterIDs;
}

/**
 * virtuelle Ladeklasse fr Kataloge. Muss berschrieben werden.
 */
int Katalog::load()
{
    QSqlCursor cur( "CatalogSet" );
    cur.select( QString("name='%1'").arg(m_name) );
    if( cur.next() ) {
        m_description = cur.value("description").toString();
        m_setID = cur.value("catalogSetID").toInt();
        kdDebug() << "Setting catalogSetID=" << m_setID << endl;
    }
    return 0;
}

QStringList Katalog::getKatalogChapters( bool freshup )
{ 
  if( m_chapters.empty() || freshup ) {
    if( ! KraftDB::getDB() ) return 0;
    if( freshup ) {
      m_chapters.clear();
      m_chapterIDs->clear();
    }
    QSqlCursor cur("CatalogChapters");
    QSqlIndex index = cur.index( "sortKey" );
    kdDebug() << "Selecting chapters for catalog no " << QString::number( m_setID ) << endl;
    cur.select( "catalogSetID=" + QString::number( m_setID ), index );

    while ( cur.next() )
    {
      QString katName = cur.value("chapter").toString();
      int katID = cur.value("chapterID").toInt();
      kdDebug() << "Adding catalog chapter " << katName << " with ID " << katID << endl;
      m_chapters.append(katName);
      dbID *id = new dbID(katID);
      m_chapterIDs->insert(katName, id);
    }
  }

  return m_chapters;
}

int Katalog::chapterID(const QString& chapter)
{
    if( m_chapterIDs->size() == 0 )
    {
        // fill up the dict of ids if still empty.
        getKatalogChapters();
    }

    dbID *id = m_chapterIDs->find(chapter);
    if( id )
        return id->intID();
    else
        return -1;
}

QString Katalog::chapterName(const dbID& id)
{
    if( m_chapterIDs->size() == 0 )
    {
        // fill up the dict of ids if still empty.
        getKatalogChapters();
    }

    QDictIterator<dbID> it( *m_chapterIDs ); // See QDictIterator
    for( ; it.current(); ++it )
        if( *(it.current()) == id )
            return it.currentKey();

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
    return UnspecKatalog;
}

void Katalog::addChapter( const QString& name, int sortKey )
{
  if( ! KraftDB::getDB() ) return;
  
  QSqlCursor cur("CatalogChapters");
  QSqlRecord *buffer = cur.primeInsert();
  buffer->setValue( "catalogSetID", m_setID );
  buffer->setValue( "chapter", name );
  buffer->setValue( "sortKey", sortKey );
  cur.insert();
}

bool Katalog::removeChapter( const QString& name, const QString& )
{
  if( ! KraftDB::getDB() ) return false;
  kdDebug() << "Deleting chapter " << name << endl;
  QSqlCursor cur( "CatalogChapters" );
  QString q = QString("catalogSetID=%1 AND chapter='%2'").arg( m_setID ).arg( name );
  
  cur.select( q );
  if ( cur.next() ) {
    cur.primeDelete();
    cur.del();
  }
  return false;
}

void Katalog::renameChapter( const QString& from, const QString& to )
{
  if( ! KraftDB::getDB() ) return;
  QSqlCursor cur( "CatalogChapters" );
  QString q = QString( "catalogSetID=%1 AND chapter='%2'").arg( m_setID ).arg( from );
  kdDebug()<< "Rename restriction: " << q << endl;
  cur.select( q );
  
  if ( cur.next() ) {
    QSqlRecord *buffer = cur.primeUpdate();
    buffer->setValue( "chapter", to );
    cur.update();
  }
}

void Katalog::setChapterSortKey( const QString& chap, int key )
{
  if( ! KraftDB::getDB() ) return;
  QSqlCursor cur( "CatalogChapters" );
  QString q = QString( "catalogSetID=%1 AND chapter='%2'").arg( m_setID ).arg( chap );
  cur.select( q );

  if ( cur.next() ) {
    QSqlRecord *buffer = cur.primeUpdate();
    buffer->setValue( "sortKey", key );
    cur.update();
  }
}

int Katalog::chapterSortKey( const QString& chap )
{
  int key = -1;
  if( ! KraftDB::getDB() ) return key;
  QSqlCursor cur( "CatalogChapters" );
  QString q = QString( "catalogSetID=%1 AND chapter='%2'").arg( m_setID ).arg( chap );
  cur.select( q );

  if ( cur.next() ) {
    key = cur.value("sortKey").toInt();
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
