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
#include "materialkataloglistview.h"
#include "matkatalog.h"
#include "docposition.h"
#include "filterheader.h"
#include "brunskatalog.h"
#include "brunskataloglistview.h"

#include <klocale.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kmenu.h>

#include <qsizepolicy.h>
#include <qcombobox.h>
#include <q3widgetstack.h>
#include <qlabel.h>
#include <q3vbox.h>
#include <q3popupmenu.h>

CatalogSelection::CatalogSelection( QWidget *parent )
  :Q3VBox( parent ),
   mCatalogSelector( 0 ),
   mWidgets( 0 ),
   mActions( 0 ),
   mAcAddToDoc( 0 )
{
  setMargin( KDialog::marginHint() );
  setSpacing( KDialog::spacingHint() );


  Q3HBox *hb = new Q3HBox( this );
  QWidget *spaceEater = new QWidget( hb );
  spaceEater->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum ) );
  QLabel *l = new QLabel( i18n( "Selected &Catalog: " ), hb );
  mCatalogSelector = new QComboBox( hb );
  connect( mCatalogSelector, SIGNAL( activated( const QString& ) ),
           this,  SLOT( slotSelectCatalog( const QString& ) ) );
  l->setBuddy( mCatalogSelector );

  mListSearchLine = new FilterHeader( 0, this ) ;
  mListSearchLine->showCount( false );

  mWidgets  = new Q3WidgetStack( this );
  mWidgets->setSizePolicy( QSizePolicy( QSizePolicy::Expanding,  QSizePolicy::Expanding ) );

  initActions();
  setupCatalogList();
}

void CatalogSelection::setupCatalogList()
{
  QStringList katalogNames = KatalogMan::self()->allKatalogNames();
  mCatalogSelector->insertStringList( katalogNames );
  slotSelectCatalog( katalogNames[0] );
}

void CatalogSelection::initActions()
{
  mActions     = new KActionCollection( this );
  mActions->addAction( "appendToDoc", this, SIGNAL( actionAppendPosition()) );

//  mAcAddToDoc  = new KAction( i18n("&Append to document"), "back", 0, this,
//                              SIGNAL( actionAppendPosition() ), mActions, "appendToDoc");

}


void CatalogSelection::slotCatalogDoubleClicked( Q3ListViewItem*,  const QPoint&,  int )
{
  emit actionAppendPosition();
}

void *CatalogSelection::currentSelectedPosition()
{
  void *flosPtr = 0;

  const QString currentCat = mCatalogSelector->currentText();
  if( mWidgetMap.contains( currentCat ) ) {
    KatalogListView *lv = mWidgetMap[currentCat];

    if ( lv ) {
      flosPtr = lv->currentItemData();
    }
  }
  return flosPtr;
}

Katalog* CatalogSelection::currentSelectedKat()
{
  const QString currentCat = mCatalogSelector->currentText();

  Katalog *kat = KatalogMan::self()->getKatalog( currentCat );

  if ( ! kat ) {
    kError() << "Could not find catalog " << currentCat << endl;
  }
  return kat;
}

void CatalogSelection::slotSelectCatalog( const QString& katName )
{
  Katalog *kat = KatalogMan::self()->getKatalog( katName );

  if ( ! kat ) {
    const QString type = KatalogMan::self()->catalogTypeString( katName );

    kDebug() << "Catalog type for cat " << katName << " is " << type << endl;
    if ( type == "TemplCatalog" ) {
      kat = new TemplKatalog( katName );
    } else if ( type == "MaterialCatalog"  ) {
      kat = new MatKatalog( katName );
    } else if ( type == "PlantCatalog" ) {
      kat = new BrunsKatalog( katName );
    }

    if ( kat ) {
      KatalogMan::self()->registerKatalog( kat );
    } else {
      kError() << "Could not find a catalog type for catname " << katName << endl;
    }
  }

  if ( kat ) {
    if ( ! mWidgetMap.contains( katName ) ) {
      KatalogListView *katListView = 0;

      if ( kat->type() == TemplateCatalog ) {
        TemplKatalogListView *tmpllistview = new TemplKatalogListView( this );
        katListView = tmpllistview;
        connect( tmpllistview,
                 SIGNAL( doubleClicked ( Q3ListViewItem *, const QPoint &, int ) ),
                 this,
                 SLOT( slotCatalogDoubleClicked( Q3ListViewItem*,  const QPoint&,  int ) ) );
        tmpllistview->setShowCalcParts( false );
        tmpllistview->addCatalogDisplay( katName );
        // mAcAddToDoc->plug( tmpllistview->contextMenu() );
        tmpllistview->contextMenu()->addAction( mAcAddToDoc );

        mWidgets->addWidget( tmpllistview );
        mWidgetMap.insert(  katName, tmpllistview );
        kDebug() << "Creating a selection list for catalog " << katName << endl;
      } else if ( kat->type() == MaterialCatalog ) {
        MaterialKatalogListView *matListView = new MaterialKatalogListView( this );
        katListView = matListView;
        connect( matListView,
                 SIGNAL( doubleClicked( Q3ListViewItem*,  const QPoint&,  int ) ),
                 this,
                 SLOT( slCatalogDoubleClicked( Q3ListViewItem*, const QPoint&, int ) ) );
        matListView->addCatalogDisplay( katName );
        matListView->contextMenu()->addAction( mAcAddToDoc );
        mWidgets->addWidget( matListView );
        mWidgetMap.insert( katName, matListView );
      } else if ( kat->type() == PlantCatalog ) {
        BrunsKatalogListView *brunsListView = new BrunsKatalogListView( this );
        katListView = brunsListView;
        brunsListView->addCatalogDisplay( katName );
        brunsListView->contextMenu()->addAction( mAcAddToDoc );
        mWidgets->addWidget( brunsListView );
        mWidgetMap.insert(  katName, brunsListView );
        kDebug() << "Creating a selection list for catalog " << katName << endl;
      }

      if ( katListView ) {
        connect( katListView, SIGNAL( selectionChanged( Q3ListViewItem* ) ),
                 this, SIGNAL( selectionChanged( Q3ListViewItem* ) ) );
        KatalogMan::self()->registerKatalogListView( katName, katListView );
      }
    }
    if ( mWidgetMap.contains( katName ) ) {
      mWidgets->raiseWidget( mWidgetMap[katName] );
      mListSearchLine->setListView( mWidgetMap[katName] );
    }
  }
}

#include "catalogselection.moc"
