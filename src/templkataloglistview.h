/***************************************************************************
             templkataloglistview  - template katalog listview.
                             -------------------
    begin                : 2005-07-09
    copyright            : (C) 2005 by Klaas Freitag
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

#ifndef TEMPLKATALOGLISTVIEW_H
#define TEMPLKATALOGLISTVIEW_H

#include <kataloglistview.h>

#include "floskeltemplate.h"

/**
@author Klaas Freitag
*/
class DocPosition;


class TemplKatalogListView : public KatalogListView
{
  Q_OBJECT

public:
  TemplKatalogListView(QWidget*);

  ~TemplKatalogListView();

  FloskelTemplate *currentTemplate();

  /* create a listview entry for a floskel template */
  QTreeWidgetItem *addFlosTemplate( QTreeWidgetItem*, FloskelTemplate* );

  void addCatalogDisplay( const QString&);

  void setShowCalcParts( bool );
  bool showCalcParts();
  DocPosition itemToDocPosition( QTreeWidgetItem* it = 0 );
  CalcPartList itemsCalcParts( QTreeWidgetItem* it = 0 );
  public slots:
  void slFreshupItem( QTreeWidgetItem*, FloskelTemplate*, bool remChildren = false );
  void saveState();

protected slots:
  void slotUpdateSequence();

private:
  bool mShowCalcParts;
  void addCalcParts( FloskelTemplate* );

};

#endif
