/***************************************************************************
        katalogselection  - widget to select catalog entries from
                             -------------------
    begin                : 2006-08-30
    copyright            : (C) 2006 by Klaas Freitag
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
#include "catalogselection.h"
#include "katalogman.h"
#include "templkatalog.h"
#include "templkataloglistview.h"
#include "matkatalog.h"

#include <klocale.h>
#include <kdebug.h>
#include <kdialog.h>

#include <qsizepolicy.h>
#include <qcombobox.h>
#include <qwidgetstack.h>
#include <qlabel.h>
#include <qvbox.h>

CatalogSelection::CatalogSelection( QWidget *parent )
  :QVBox( parent )
{
  setMargin( KDialog::marginHint() );
  setSpacing( KDialog::spacingHint() );

  QHBox *hb = new QHBox( this );
  QWidget *spaceEater = new QWidget( hb );
  spaceEater->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum ) );
  QLabel *l = new QLabel( i18n( "Selected &Catalog:" ), hb );
  mCatalogSelector = new QComboBox( hb );
  connect( mCatalogSelector, SIGNAL( activated( const QString& ) ),
           this,  SLOT( slotSelectCatalog( const QString& ) ) );
  l->setBuddy( mCatalogSelector );
  mWidgets  = new QWidgetStack( this );
  mWidgets->setSizePolicy( QSizePolicy( QSizePolicy::Expanding,  QSizePolicy::Expanding ) );

  setupCatalogList();
}

void CatalogSelection::setupCatalogList()
{
  QStringList katalogNames = KatalogMan::self()->allKatalogs();
  mCatalogSelector->insertStringList( katalogNames );
  slotSelectCatalog( katalogNames[0] );
}

void CatalogSelection::slotSelectCatalog( const QString& katName )
{
  Katalog *kat = KatalogMan::self()->getKatalog( katName );

  if ( ! kat ) {
    const QString type = KatalogMan::self()->catalogTypeString( katName );

    kdDebug() << "Catalog type for cat " << katName << " is " << type << endl;
    if ( type == "TemplCatalog" ) {
      kat = new TemplKatalog( katName );
    } else if ( type == "MaterialCatalog"  ) {
      kat = new MatKatalog( katName );
    }

    if ( kat ) {
      KatalogMan::self()->registerKatalog( kat );
    } else {
      kdError() << "Could not find a catalog type for catname " << katName << endl;
    }
  }

  if ( kat ) {
    if ( kat->type() == TemplateKatalog ) {

      TemplKatalogListView *tmpllistview = new TemplKatalogListView( this );
      tmpllistview->setShowCalcParts( false );
      tmpllistview->addCatalogDisplay( katName );
      mWidgets->addWidget( tmpllistview );
    }
  }

}

#include "catalogselection.moc"
