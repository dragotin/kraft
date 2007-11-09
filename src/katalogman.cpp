/***************************************************************************
             katalogman  -
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
#include <qstringlist.h>
#include <qsqlcursor.h>

// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <kstaticdeleter.h>

#include "kraftdb.h"
#include "katalogman.h"
#include "katalog.h"
#include "templkatalog.h"

static KStaticDeleter<KatalogMan> selfDeleter;

KatalogMan* KatalogMan::mSelf = 0;

KatalogMan *KatalogMan::self()
{
  if ( ! mSelf ) {
    selfDeleter.setObject( mSelf, new KatalogMan() );
  }
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

  QSqlCursor cur( "CatalogSet" );
  cur.select( );
  while( cur.next() ) {
    list << cur.value("name").toString();
  }

  return list;
}

QString KatalogMan::catalogTypeString( const QString& catName )
{
  QString res;
  QSqlCursor cur( "CatalogSet" );
  if ( !catName.isEmpty() ) {
    cur.select( "name='" + catName + "'" );
    if ( cur.next() ) {
      res = cur.value( "catalogType" ).toString();
    }
  }
  return res;
}

void KatalogMan::registerKatalog( Katalog *k )
{
    Katalog* kat = m_katalogDict[k->getName()];

    if( kat ) {
        kdWarning() << "Katalog with same name already here -> deleting!" << endl;
        delete kat;
    } else {
        // not found, try to open it
        kdDebug() << "Katalog " << k->getName() << " registered and loading..." << endl;
        m_katalogDict.insert( k->getName(), k );
        k->load ();
    }
}

Katalog *KatalogMan::getKatalog(const QString& name)
{
    Katalog* kat = m_katalogDict[name];

    if( !kat ) {
        kdDebug() << "No katalog " << name << " found" << endl;
    } else {
        kdDebug() << "Returning existing katalog " << name << endl;
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

    QPtrList<KatalogListView> views = mKatalogListViews[name];

    KatalogListView *view;
    for ( view = views.first(); view; view = views.next() ) {
      view->slotRedraw();
    }
  }

  // if ( id.isOk() ) {
    // update a existing item
  // } else {
    // it's a new item saved to the db now.
  // }
}

void KatalogMan::registerKatalogListView( const QString& name, KatalogListView *view )
{
  QPtrList<KatalogListView> views = mKatalogListViews[name];

  if ( ! views.contains( view ) ) {
    views.append( view );
    mKatalogListViews[name] = views;
  }
}

/*
 * currently, there is only one catalog of type Template by design, see
 * for example in templatesaverdb.cpp or the database design where only
 * one template catalog is in use.
 */

Katalog* KatalogMan::defaultTemplateCatalog()
{
  QDictIterator<Katalog> it( m_katalogDict ); // See QDictIterator
  for( ; it.current(); ++it ) {
    Katalog *k = it.current();
    if ( k->type() == TemplateCatalog ) {
      kdDebug() << "Found default template catalog: " << k->getName() << endl;
      return k;
    }
  }
  return 0;
}

/* END */

