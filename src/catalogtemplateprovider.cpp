/***************************************************************************
      catalogtemplateprovider - template provider class for catalog data
                             -------------------
    begin                : 2007-05-23
    copyright            : (C) 2007 by Klaas Freitag
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

#include <QDebug>

#include "catalogtemplateprovider.h"
#include "texteditdialog.h"
#include "doctext.h"
#include "defaultprovider.h"
#include "katalog.h"
#include "catalogselection.h"

CatalogTemplateProvider::CatalogTemplateProvider( QWidget *parent )
  :TemplateProvider( parent ),
   mCatalogSelection(nullptr)
{

}

Katalog *CatalogTemplateProvider::currentCatalog()
{
    Katalog *kat {nullptr};
    if (mCatalogSelection) {
        kat = mCatalogSelection->currentSelectedKat();
    }
    return kat;
}


void CatalogTemplateProvider::setCatalogSelection( CatalogSelection *cs )
{
  mCatalogSelection = cs;

  connect( mCatalogSelection, SIGNAL( actionAppendPosition() ),
           this, SLOT( slotTemplateToDocument() ) );
}

void CatalogTemplateProvider::slotNewTemplate()
{
    qDebug () << "SlotNewTemplate for Catalog called!";
    if ( mCatalogSelection ) {
      Katalog *catalog = mCatalogSelection->currentSelectedKat();

      CatalogTemplateList list;
      const QString currKat = mCatalogSelection->currentSelectedKatChapter();
      Q_EMIT templatesToDocument(catalog, list, currKat);
    }
}

void CatalogTemplateProvider::slotEditTemplate()
{
  // qDebug () << "SlotEditTemplate for Catalog called!";
    // mCatalogSelection->currentSelectedPositions
}

void CatalogTemplateProvider::slotDeleteTemplate()
{
}

void CatalogTemplateProvider::slotTemplateToDocument()
{
  // qDebug () << "Moving catalog entry to document";

  if ( mCatalogSelection ) {
    Katalog *catalog = mCatalogSelection->currentSelectedKat();

    Q_EMIT templatesToDocument(catalog, mCatalogSelection->currentSelectedPositions(),
                             QString());
  }
}

void CatalogTemplateProvider::slotInsertTemplateToDocument()
{

}
