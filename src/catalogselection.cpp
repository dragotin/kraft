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
#include "catalogtemplate.h"
#include "katalogman.h"
#include "templkatalog.h"
#include "templkataloglistview.h"
#include "materialkataloglistview.h"
#include "matkatalog.h"
#include "docposition.h"
#include "filterheader.h"

#include <QLocale>
#include <QDebug>
#include <QDialog>
#include <QAction>
#include <QMenu>

#include <QSizePolicy>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QStackedWidget>
#include <QLabel>
#include <KLocalizedString>

CatalogSelection::CatalogSelection( QWidget *parent )
    :QWidget( parent ),
      mCatalogSelector( 0 ),
      mWidgets( 0 )
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    QHBoxLayout *hb = new QHBoxLayout;
    layout->addLayout(hb);
    QLabel *l = new QLabel( i18n( "Selected &Catalog: " ) );
    hb->addWidget(l);
    mCatalogSelector = new QComboBox;
    hb->addWidget(mCatalogSelector);
    connect( mCatalogSelector, SIGNAL( activated( const QString& ) ),
             this,  SLOT( slotSelectCatalog( const QString& ) ) );
    l->setBuddy( mCatalogSelector );

    hb->addStretch();
    mListSearchLine = new FilterHeader;
    hb->addWidget(mListSearchLine);

    mWidgets  = new QStackedWidget;
    mWidgets->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    layout->addWidget(mWidgets);

    this->setLayout(layout);
    setupCatalogList();
}

void CatalogSelection::setupCatalogList()
{
  QStringList katalogNames = KatalogMan::self()->allKatalogNames();
  mCatalogSelector->insertItems(-1, katalogNames );
  slotSelectCatalog( katalogNames[0] );
}

void CatalogSelection::slotCatalogDoubleClicked( QModelIndex )
{
  emit actionAppendPosition();
}

CatalogTemplateList CatalogSelection::currentSelectedPositions()
{
  CatalogTemplateList re;
  const QString currentCat = mCatalogSelector->currentText();
  if( mWidgetMap.contains( currentCat ) ) {
    KatalogListView *lv = mWidgetMap[currentCat];

    re = lv->selectedTemplates();
  }
  return re;
}

Katalog* CatalogSelection::currentSelectedKat()
{
  const QString currentCat = mCatalogSelector->currentText();

  Katalog *kat = KatalogMan::self()->getKatalog( currentCat );

  if ( ! kat ) {
    qCritical() << "Could not find catalog " << currentCat << endl;
  }
  return kat;
}

void CatalogSelection::slotSelectCatalog( const QString& katName )
{
  Katalog *kat = KatalogMan::self()->getKatalog( katName );

  if ( ! kat ) {
    const QString type = KatalogMan::self()->catalogTypeString( katName );

    // qDebug () << "Catalog type for cat " << katName << " is " << type << endl;
    if ( type == "TemplCatalog" ) {
      kat = new TemplKatalog( katName );
    } else if ( type == "MaterialCatalog"  ) {
      kat = new MatKatalog( katName );
    } else {
      // nothing.
    }
    if ( kat ) {
      KatalogMan::self()->registerKatalog( kat );
    } else {
      qCritical() << "Could not find a catalog type for catname " << katName << endl;
    }
  }

  if ( kat ) {
    if ( ! mWidgetMap.contains( katName ) ) {
      KatalogListView *katListView = 0;

      if ( kat->type() == TemplateCatalog ) {
        TemplKatalogListView *tmpllistview = new TemplKatalogListView( this );
        katListView = tmpllistview;
        katListView->setSelectFromMode(); // mode to only select from
        connect( tmpllistview,
                 SIGNAL( doubleClicked( QModelIndex ) ),
                 this,
                 SLOT( slotCatalogDoubleClicked( QModelIndex ) ) );
        tmpllistview->setShowCalcParts( false );
        tmpllistview->addCatalogDisplay( katName );
        tmpllistview->contextMenu()->addAction( i18n("Append to Document"),
                                               this, SIGNAL( actionAppendPosition() ) );

        mWidgets->addWidget( tmpllistview );
        mWidgetMap.insert(  katName, tmpllistview );
        // qDebug () << "Creating a selection list for catalog " << katName << endl;
      } else if ( kat->type() == MaterialCatalog ) {
        MaterialKatalogListView *matListView = new MaterialKatalogListView( this );
        katListView = matListView;
        katListView->setSelectFromMode(); // mode to only select from
        connect( matListView,
                 SIGNAL( doubleClicked( QModelIndex ) ),
                 this,
                 SLOT( slCatalogDoubleClicked( QModelIndex ) ) );
        matListView->addCatalogDisplay( katName );
        matListView->contextMenu()->addAction( i18n("Append to Document"),
                                              this, SIGNAL( actionAppendPosition() ) );
        mWidgets->addWidget( matListView );
        mWidgetMap.insert( katName, matListView );
      }
      if ( katListView ) {
        connect( katListView, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
                 this, SIGNAL( selectionChanged(QTreeWidgetItem*,QTreeWidgetItem*) ) );
        KatalogMan::self()->registerKatalogListView( katName, katListView );

      }
    }
    if ( mWidgetMap.contains( katName ) ) {
      mWidgets->setCurrentWidget( mWidgetMap[katName] );
      mListSearchLine->setListView( mWidgetMap[katName] );
    }
  }
}

