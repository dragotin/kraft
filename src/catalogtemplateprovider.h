/***************************************************************************
      catalogtemplateprovider - template provider classes for catalog data
                             -------------------
    begin                : 2007-05-24
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
#ifndef CATALOGTEMPLATEPROVIDER_H
#define CATALOGTEMPLATEPROVIDER_H

#include "templateprovider.h"
#include "doctext.h"
#include "catalogtemplate.h"
#include "katalog.h"

class QWidget;
class CatalogSelection;

class CatalogTemplateProvider : public TemplateProvider
{
  Q_OBJECT
public:
  CatalogTemplateProvider( QWidget* );
  void setCatalogSelection( CatalogSelection * );

  Katalog *currentCatalog();

Q_SIGNALS:
  void templatesToDocument( Katalog*, CatalogTemplateList, const QString& );
  void catalogSelected(Katalog*);

public Q_SLOTS:
  void slotNewTemplate() override;
  void slotEditTemplate() override;
  void slotDeleteTemplate() override;

  void slotTemplateToDocument() override;
  void slotInsertTemplateToDocument() override;

private:
  CatalogSelection *mCatalogSelection;
};


#endif

