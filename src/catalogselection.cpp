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
      mCatalogSelector(nullptr),
      mWidgets(nullptr)
{
    QVBoxLayout *layout = new QVBoxLayout;
    // layout->setMargin(0); Qt6 port FIXME
    QHBoxLayout *hb = new QHBoxLayout;
    layout->addLayout(hb);
    QLabel *l = new QLabel( i18n( "Selected &catalog: " ) );
    hb->addWidget(l);
    mCatalogSelector = new QComboBox;
    hb->addWidget(mCatalogSelector);
    connect( mCatalogSelector, &QComboBox::textActivated, this, &CatalogSelection::slotSelectCatalog);
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
  Q_EMIT actionAppendPosition();
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
    qCritical() << "Could not find catalog " << currentCat;
  }
  return kat;
}

QString CatalogSelection::currentSelectedKatChapter()
{
    QString chap;
    const QString currentCat = mCatalogSelector->currentText();
    if( mWidgetMap.contains( currentCat ) ) {
      KatalogListView *lv = mWidgetMap[currentCat];

      chap = lv->selectedCatalogChapter();
    }

    return chap;
}

void CatalogSelection::slotSelectCatalog( const QString& katName )
{
    Katalog *kat = KatalogMan::self()->getKatalog( katName );

    if ( !kat ) {
        const QString type = KatalogMan::self()->catalogTypeString( katName );

        // qDebug () << "Catalog type for cat " << katName << " is " << type;
        if ( type == QStringLiteral("TemplCatalog") ) {
            kat = new TemplKatalog( katName );
        } else if ( type == QStringLiteral("MaterialCatalog") ) {
            kat = new MatKatalog( katName );
        }

        if ( kat ) {
            KatalogMan::self()->registerKatalog( kat );
        } else {
            qCritical() << "Could not find a valid catalog type for catalog named " << katName;
        }
    }

    if ( kat ) {
        KatalogListView *katListView = nullptr;
        if ( ! mWidgetMap.contains( katName ) ) {

            if ( kat->type() == TemplateCatalog ) {
                TemplKatalogListView *tmpllistview = new TemplKatalogListView( this );
                katListView = tmpllistview;
                tmpllistview->setShowCalcParts( false );

                // qDebug () << "Creating a selection list for catalog " << katName;
            } else if ( kat->type() == MaterialCatalog ) {
                MaterialKatalogListView *matListView = new MaterialKatalogListView( this );
                katListView = matListView;
            }

            if ( katListView ) {
                katListView->setSelectFromMode(); // mode to only select from
                mWidgets->addWidget(katListView);
                mWidgetMap.insert(  katName, katListView );
                katListView->contextMenu()->addAction( i18n("Append to Document"),
                                                       this, &CatalogSelection::actionAppendPosition);
                katListView->addCatalogDisplay( katName );
                connect(katListView, &KatalogListView::doubleClicked,
                        this, &CatalogSelection::slotCatalogDoubleClicked);
                connect(katListView, &KatalogListView::currentItemChanged,
                        this, &CatalogSelection::selectionChanged);
                KatalogMan::self()->registerKatalogListView( katName, katListView );

            }
        } else {
            katListView = mWidgetMap[katName];
        }

        // Select the widget
        if ( katListView ) {
            mWidgets->setCurrentWidget(katListView);
            mListSearchLine->setListView(katListView);
            Q_EMIT selectionChanged(katListView->currentItem(), nullptr);
        }
    }
}

