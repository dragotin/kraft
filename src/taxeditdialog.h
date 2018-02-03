/***************************************************************************
                   taxeditdialog.h  - edit tax rates
                             -------------------
    begin                : Apr 9 2009
    copyright            : (C) 2009 by Klaas Freitag
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

#ifndef TAXEDITDIALOG_H
#define TAXEDITDIALOG_H

#include <QObject>
#include <QSqlTableModel>
#include <QDataWidgetMapper>

#include <QDialog>

#include "ui_taxeditbase.h"

/**
 *  @author Klaas Freitag
 */

// ################################################################################

class TaxEditDialog: public QDialog, protected Ui::TaxEditBase
{
  Q_OBJECT

public:
  TaxEditDialog( QSqlTableModel *taxModel, QWidget *parent );

public slots:
  void accept();
  void reject();

private:
  Ui::TaxEditBase *mBaseWidget;
  QDataWidgetMapper *mapper;
  QSqlTableModel *model;
};

#endif
