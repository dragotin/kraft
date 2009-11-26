/***************************************************************************
                   prefsdialog.h  - the preferences Dialog
                             -------------------
    begin                : Sun Jul 3 2004
    copyright            : (C) 2004 by Klaas Freitag
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

#ifndef PREFSDIALOG_H
#define PREFSDIALOG_H

#include <kpagedialog.h>

#include "doctypeedit.h"
#include "doctype.h"
#include "taxeditdialog.h"

class QLineEdit;
class QLabel;
class QPushButton;
class QComboBox;
class QCheckBox;
class QTreeView;
class QSqlTableModel;

// ################################################################################

class PrefsDialog : public KPageDialog
{
  Q_OBJECT

public:
  PrefsDialog(QWidget *parent);

  ~PrefsDialog();


protected:
  void readConfig();
  void writeConfig();

protected slots:
  void accept();
  void slotDbCredentialsChanged( const QString& );
  void slotCheckConnect();
  
  void slotAddTax();
  void slotDeleteTax();
  void slotTaxSelected(QModelIndex);
  void slotDocTypeRemoved( const QString& );

private:
  void databaseTab();
  void docTab();
  void doctypeTab();
  void taxTab();
  void writeTaxes();

  QLineEdit *m_leHost;
  QLineEdit *m_leUser;
  QLineEdit *m_leName;
  QLineEdit *m_lePasswd;
  QLabel    *m_statusLabel;

  QComboBox *mCbDocTypes;
  QComboBox *mCbDefaultTaxType;

  QPushButton *m_pbCheck;

  QCheckBox *mCbDocLocale;

  DocTypeEdit *mDocTypeEdit;
  
  QPushButton *mDelTax;
  QTreeView *mTaxTreeView;
  QSqlTableModel *mTaxModel;
};

#endif
