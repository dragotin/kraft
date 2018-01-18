/***************************************************************************
                   prefsunits.h  - the units tab in the prefs dialog
                             -------------------
    begin                : Feb 28 2010
    copyright            : (C) 2010 by Thomas Richard
    email                : thomas.richard@proan.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PREFSUNITS_H
#define PREFSUNITS_H

#include <QWidget>
#include <QItemDelegate>

#include <QDialog>

#include "ui_unitseditbase.h"

class QModelIndex;
class QPushButton;
class ImpTreeView;
class QAbstractItemModel;
class QDataWidgetMapper;
class QSqlTableModel;
class QSortFilterProxyModel;

class PrefsUnits : public QWidget
{
    Q_OBJECT
public:
    PrefsUnits(QWidget* parent);

    ~PrefsUnits();

    void save();

public slots:
    void slotAddUnit();
    void slotEditUnit(QModelIndex index = QModelIndex());
    void slotDeleteUnit();
    void slotUnitSelected(QModelIndex);

private:
    QPushButton *mDelUnit;
    QPushButton *mEditUnit;
    ImpTreeView *mUnitsTreeView;
    QSqlTableModel *mUnitsModel;
    QSortFilterProxyModel *mProxyModel;
};

class UnitsEditDialog: public QDialog, protected Ui::UnitsEditBase
{
  Q_OBJECT

public:
  UnitsEditDialog( QAbstractItemModel *model, int row, QWidget *parent );

public slots:
  void accept();
  void reject();

private:
  Ui::UnitsEditBase *mBaseWidget;
  QDataWidgetMapper *mapper;
  QAbstractItemModel *mModel;
  int mRow;
};

#endif
