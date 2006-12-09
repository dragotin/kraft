/***************************************************************************
                          materialkatalogview.h
                             -------------------
    begin                : 2006-11-30
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
#ifndef MATERIALKATALOGVIEW_H
#define MATERIALKATALOGVIEW_H

#include <katalogview.h>

#include "materialkataloglistview.h"

class QBoxLayout;
class QListViewItem;
class BrunsKatalogListView;
class QLabel;
class MaterialTemplDialog;

/**
@author Klaas Freitag
*/
class MaterialKatalogView : public KatalogView
{
  Q_OBJECT
public:
  MaterialKatalogView();

  ~MaterialKatalogView();

  void createCentralWidget(QBoxLayout*, QWidget *w);
  KatalogListView* getListView() { return m_materialListView; }
protected slots:
  void slNeueVorlage();
  void openDialog( KListViewItem *, StockMaterial *, bool );
  void slotEditRejected();
  void slotEditOk( StockMaterial * );
protected:
  Katalog* getKatalog( const QString& );

  MaterialKatalogListView *m_materialListView;
  QLabel               *m_detailLabel;
  KListView            *m_details;
  MaterialTemplDialog *mDialog;
  KListViewItem *mNewItem;
  const QString MaterialName;
};

#endif
