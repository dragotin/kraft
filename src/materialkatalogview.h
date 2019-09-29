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
class QTreeWidgetItem;
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

  static const QString MaterialCatalogName;

protected slots:
  void slNewTemplate();
  void slEditTemplate();
  void slDeleteTemplate();

  void openDialog( QTreeWidgetItem *, StockMaterial *, bool );
  void slotEditRejected();
  void slotEditOk( StockMaterial * );

protected:
  Katalog* getKatalog( const QString& );

  void saveWindowState( const QByteArray& arr );
  QByteArray windowState();

  void saveWindowGeo( const QByteArray& arr );
  QByteArray windowGeo();

  MaterialKatalogListView *m_materialListView;
  QLabel               *m_detailLabel;
  QTreeWidget           *m_details;
  MaterialTemplDialog *mDialog;
  QTreeWidgetItem *mNewItem;
};

#endif
