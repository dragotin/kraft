/***************************************************************************
                   impviewwidgets.h  - Improved view widgets
                             -------------------
    begin                : Sun Nov 29 2009
    copyright            : (C) 2009 by Klaas Freitag and Thomas Richard
    email                : freitag@kde.org, thomas.richard@proan.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef IMPVIEWWIDGETS_H
#define IMPVIEWWIDGETS_H

#include <QTreeView>

class QWidget;
class QAbstractItemModel;

class ImpTreeView : public QTreeView
{
  Q_OBJECT

public:
  ImpTreeView(QWidget *parent = 0);

  void setModel ( QAbstractItemModel * model );
  void unhideRows();

private slots:
  void headerDataChanged(Qt::Orientation orientation, int first, int last);
};

#endif
