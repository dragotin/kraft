/***************************************************************************
                   impviewwidgets.cpp  - Improved view widgets
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

#include <QTreeView>
#include <QWidget>
#include <QAbstractItemModel>
#include <QVariant>
#include <QModelIndex>
#include <QString>

#include "impviewwidgets.h"

ImpTreeView::ImpTreeView(QWidget *parent ) : QTreeView(parent) {}

void ImpTreeView::setModel ( QAbstractItemModel * model )
{
  connect(model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)), this, SLOT(headerDataChanged(Qt::Orientation,int,int)));

  QTreeView::setModel(model);
}

void ImpTreeView::headerDataChanged(Qt::Orientation orientation, int first, int last)
{
  if(orientation == Qt::Vertical)
  {
    for(int i = first; i <= last; ++i)
    {
      QVariant sign = model()->headerData(i, orientation);
      if (sign == QString ("!"))
        setRowHidden(i, QModelIndex(), true);
      else
        setRowHidden(i, QModelIndex(), false);
    }
  }
}

void ImpTreeView::unhideRows()
{
  for(int i = 0; i < model()->rowCount(); ++i)
    setRowHidden(i, QModelIndex(), false);
}
