/***************************************************************************
       materialselectdialog - select material for inserting into template
                             -------------------
    begin                : 2006-12-17
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

#ifndef _MATERIALSELECTDIALOG_H
#define _MATERIALSELECTDIALOG_H

#include "calcdialogbase.h"

class MaterialKatalogListView;

/* ********************************************************************************
 * Editor that shows the MaterialKatalogListView
 * ********************************************************************************/

class MaterialSelectDialog : public CalcDialogBase
{
  Q_OBJECT

public:
  MaterialSelectDialog(QWidget * parent = 0);
  ~MaterialSelectDialog();

public slots:

protected slots:
  void accept();
signals:
  void materialSelected( int, double );
private:
  MaterialKatalogListView *mKatalogListView;
};

#endif

/* END */

