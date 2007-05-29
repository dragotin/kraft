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

#include <kdebug.h>
#include <klocale.h>

#include "catalogtemplateprovider.h"
#include "texteditdialog.h"
#include "doctext.h"
#include "defaultprovider.h"

CatalogTemplateProvider::CatalogTemplateProvider( QWidget *parent )
  :TemplateProvider( parent )
{

}

void CatalogTemplateProvider::slotNewTemplate()
{
  kdDebug() << "SlotNewTemplate for Catalog called!" << endl;


}

void CatalogTemplateProvider::slotEditTemplate()
{
  kdDebug() << "SlotEditTemplate for Catalog called!" << endl;


}

void CatalogTemplateProvider::slotSetCurrentCatalogName( const QString& dt )
{
  mCurrentCatalogName = dt;
}

void CatalogTemplateProvider::slotDeleteTemplate()
{
}

void CatalogTemplateProvider::slotTemplateToDocument()
{
  kdDebug() << "Moving catalog entry to document" << endl;

}

#include "catalogtemplateprovider.moc"

