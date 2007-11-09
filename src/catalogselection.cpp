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


#include <qsizepolicy.h>
#include <qcombobox.h>
#include <qwidgetstack.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qpopupmenu.h>

CatalogSelection::CatalogSelection( QWidget *parent )
  :QVBox( parent ),
   mCatalogSelector( 0 ),
   mWidgets( 0 ),
   mActions( 0 ),
   mAcAddToDoc( 0 )
{
  setMargin( KDialog::marginHint() );
  setSpacing( KDialog::spacingHint() );

  QHBox *hb = new QHBox( this );
  QWidget *spaceEater = new QWidget( hb );
  spaceEater->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum ) );
  QLabel *l = new QLabel( i18n( "Selected &Catalog: " ), hb );
  mCatalogSelector = new QComboBox( hb );
  connect( mCatalogSelector, SIGNAL( activated( const QString& ) ),
           this,  SLOT( slotSelectCatalog( const QString& ) ) );
  l->setBuddy( mCatalogSelector );

  mListSearchLine = new FilterHeader( 0, this ) ;
  mListSearchLine->showCount( false );

  mWidgets  = new QWidgetStack( this );
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
  mAcAddToDoc  = new KAction( i18n("&Append to document"), "back", 0, this,
                              SIGNAL( actionAppendPosition() ), mActions, "appendToDoc");

}


void CatalogSelection::slotCatalogDoubleClicked( QListViewItem*,  const QPoint&,  int )
{
  emit actionAppendPosition();
}

void *CatalogSelection::currentSelectedPosition()
{
  const QString currentCat = mCatalogSelector->currentText();
  KatalogListView *lv = mWidgetDict[ currentCat ];

  void *flosPtr = 0;
  if ( lv ) {
    flosPtr = lv->currentItemData();
  }
  return flosPtr;
}

Katalog* CatalogSelection::currentSelectedKat()
{
  const QString currentCat = mCatalogSelector->currentText();

  Katalog *kat = KatalogMan::self()->getKatalog( currentCat );

  if ( ! kat ) {
    kdError() << "Could not find catalog " << currentCat << endl;
  }
  return kat;
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
    } else if ( type == "PlantCatalog" ) {
      kat = new BrunsKatalog( katName );
    }

    if ( kat ) {
      KatalogMan::self()->registerKatalog( kat );
    } else {
      kdError() << "Could not find a catalog type for catname " << katName << endl;
    }
  }

  if ( kat ) {

    if ( ! mWidgetDict[katName] ) {
      KatalogListView *katListView = 0;

      if ( kat->type() == TemplateCatalog ) {
        TemplKatalogListView *tmpllistview = new TemplKatalogListView( this );
        katListView = tmpllistview;
        connect( tmpllistview,
                 SIGNAL( doubleClicked ( QListViewItem *, const QPoint &, int ) ),
                 this,
                 SLOT( slotCatalogDoubleClicked( QListViewItem*,  const QPoint&,  int ) ) );
        tmpllistview->setShowCalcParts( false );
        tmpllistview->addCatalogDisplay( katName );
        mAcAddToDoc->plug( tmpllistview->contextMenu() );

        mWidgets->addWidget( tmpllistview );
        mWidgetDict.insert(  katName, tmpllistview );
        kdDebug() << "Creating a selection list for catalog " << katName << endl;
      } else if ( kat->type() == MaterialCatalog ) {
        MaterialKatalogListView *matListView = new MaterialKatalogListView( this );
        katListView = matListView;
        connect( matListView,
                 SIGNAL( doubleClicked( QListViewItem*,  const QPoint&,  int ) ),
                 this,
                 SLOT( slCatalogDoubleClicked( QListViewItem*, const QPoint&, int ) ) );
        matListView->addCatalogDisplay( katName );
        mAcAddToDoc->plug( matListView->contextMenu() );
        mWidgets->addWidget( matListView );
        mWidgetDict.insert( katName, matListView );
      } else if ( kat->type() == PlantCatalog ) {
        BrunsKatalogListView *brunsListView = new BrunsKatalogListView( this );
        katListView = brunsListView;
        brunsListView->addCatalogDisplay( katName );

        mAcAddToDoc->plug( brunsListView->contextMenu() );
        mWidgets->addWidget( brunsListView );
        mWidgetDict.insert(  katName, brunsListView );
        kdDebug() << "Creating a selection list for catalog " << katName << endl;
      }

      if ( katListView ) {
        connect( katListView, SIGNAL( selectionChanged( QListViewItem* ) ),
                 this, SIGNAL( selectionChanged( QListViewItem* ) ) );
        KatalogMan::self()->registerKatalogListView( katName, katListView );
      }
    }
    if ( mWidgetDict[katName] ) {
      mWidgets->raiseWidget( mWidgetDict[katName] );
      mListSearchLine->setListView( mWidgetDict[katName] );
    }
  }
}

#include "catalogselection.moc"
