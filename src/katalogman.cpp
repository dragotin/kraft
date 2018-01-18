/***************************************************************************
                     katalogman  - Catalog manager
                             -------------------
    begin                : 2004-12-09
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

// include files for Qt
#include <QtCore>
#include <QSqlQuery>
#include <QGlobalStatic>

#include "kraftdb.h"
#include "katalogman.h"
#include "katalog.h"
#include "templkatalog.h"
#include "materialkatalogview.h"

Q_GLOBAL_STATIC(KatalogMan, mSelf)

KatalogMan *KatalogMan::self()
{
  return mSelf;
}

KatalogMan::KatalogMan( )
{

}

KatalogMan::~KatalogMan( )
{
}

QStringList KatalogMan::allKatalogNames()
{

  QStringList list;

  QSqlQuery q( "SELECT name FROM CatalogSet ORDER BY sortKey, name" );

  while( q.next() ) {
    list << q.value( 0 ).toString();
  }

  return list;
}

QString KatalogMan::catalogTypeString( const QString& catName )
{
  QString res;
  if ( !catName.isEmpty() ) {
    QSqlQuery q;
    q.prepare( "SELECT catalogType FROM CatalogSet where name=:name" );
    q.bindValue( ":name",  catName );

    if ( q.exec() && q.next() ) {
      res = q.value( 0 ).toString();
    }
  }
  return res;
}

void KatalogMan::registerKatalog( Katalog *k )
{
    Katalog* kat = m_katalogDict[k->getName()];

    if( kat ) {
        qWarning() << "Katalog with same name already here -> deleting!" << endl;
        delete kat;
    } else {
        // not found, try to open it
        // qDebug () << "Katalog " << k->getName() << " registered and loading..." << endl;
        m_katalogDict.insert( k->getName(), k );
        k->load ();
    }
}

Katalog *KatalogMan::getKatalog(const QString& name)
{
    Katalog* kat = m_katalogDict[name];

    if( !kat ) {
        // qDebug () << "No katalog " << name << " found" << endl;
    } else {
        // qDebug() << "Returning existing katalog " << name << endl;
    }
    return kat;
}

// this is called after an template has been changed in the database.
void KatalogMan::notifyKatalogChange( Katalog* k, dbID )
{
  // FIXME: More efficient catalog reloading.
  if ( k ) {
    const QString name = k->getName();
    k->reload( dbID() );

    QList< QPointer<KatalogListView> > views = mKatalogListViews.values(name);

    KatalogListView *view;
    QListIterator< QPointer<KatalogListView> > i( views );
    while ( i.hasNext() ) {
      view = i.next();
      if( view ) {
          view->slotRedraw();
      }
    }
  }
}

void KatalogMan::registerKatalogListView( const QString& name, KatalogListView *view )
{
  QList< QPointer<KatalogListView> > views = mKatalogListViews.values(name);

  if ( ! views.contains( view ) ) {
      mKatalogListViews.insert(name, QPointer<KatalogListView>(view));
  }
}

/*
 * currently, there is only one catalog of type Template by design, see
 * for example in templatesaverdb.cpp or the database design where only
 * one template catalog is in use.
 */

Katalog* KatalogMan::defaultTemplateCatalog()
{
  QHashIterator<QString, Katalog*> it( m_katalogDict ); // See QDictIterator
  while ( it.hasNext() ) {
    it.next();
    Katalog *k = it.value();
    if ( k->type() == TemplateCatalog ) {
      // qDebug () << "Found default template catalog: " << k->getName() << endl;
      return k;
    }
  }
  return 0;
}

KatalogMan::CatalogDetails KatalogMan::catalogDetails( const QString& catName )
{
    KatalogMan::CatalogDetails details;

    QString sql;
    QString catTypeString = KatalogMan::catalogTypeString( catName );

    if( catTypeString == QLatin1String("MaterialCatalog") ) {
        sql = "SELECT count(matID), COUNT(distinct chapterID), MAX(modifyDate) FROM stockMaterial";
    } else if( catTypeString == QLatin1String("TemplCatalog") ) {
        sql = "SELECT count(TemplID), COUNT(distinct chapterID), MAX(modifyDatum) FROM Catalog";
    }
    QSqlQuery q;
    q.prepare( sql );

    if ( !sql.isEmpty() && q.exec() && q.next() ) {
        details.countEntries  = q.value( 0 ).toInt();
        details.countChapters = q.value( 1 ).toInt();
        details.maxModDate    = q.value( 2 ).toDateTime();
    }

    return details;
    
}

/* END */

