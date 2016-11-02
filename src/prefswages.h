/***************************************************************************
                   prefswages.h  - the wages tab in the prefs dialog
                             -------------------
    begin                : Feb 26 2010
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

#ifndef PREFSWAGES_H
#define PREFSWAGES_H

#include <QWidget>
#include <QItemDelegate>

#include <QDialog>

#include "ui_wageseditbase.h"

class QModelIndex;
class QPushButton;
class ImpTreeView;
class QAbstractItemModel;
class QDataWidgetMapper;
class QSqlTableModel;
class QSortFilterProxyModel;

class PrefsWages : public QWidget
{
    Q_OBJECT
public:
    PrefsWages(QWidget* parent);

    ~PrefsWages();

    void save();

public slots:
    void slotAddWage();
    void slotEditWage(QModelIndex index = QModelIndex());
    void slotDeleteWage();
    void slotWageSelected(QModelIndex);
    void slotUp();
    void slotDown();

private:
    QPushButton *mDelWage;
    QPushButton *mEditWage;
    QPushButton *mUp;
    QPushButton *mDown;
    ImpTreeView *mWagesTreeView;
    QSqlTableModel *mWagesModel;
    QSortFilterProxyModel *mProxyModel;
};

class WagesEditDialog: public QDialog, protected Ui::WagesEditBase
{
  Q_OBJECT

public:
  WagesEditDialog( QAbstractItemModel *model, int row, QWidget *parent );

public slots:
  void accept();
  void reject();

private:
  Ui::WagesEditBase *mBaseWidget;
  QDataWidgetMapper *mapper;
  QAbstractItemModel *mModel;
  int mRow;
};

class WagesItemDelegate : public QItemDelegate
{
  Q_OBJECT

public:
  WagesItemDelegate(QObject * parent = 0);

  virtual void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

#endif
