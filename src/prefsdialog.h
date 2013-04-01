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

#include <kabc/addressee.h>

#include <kpagedialog.h>
#include <QItemDelegate>

#include "doctypeedit.h"
#include "doctype.h"
#include "taxeditdialog.h"

class QLineEdit;
class QLabel;
class QPushButton;
class QComboBox;
class QCheckBox;
class QSqlTableModel;
class QTreeView;
class QPainter;
class QStyleOptionViewItem;
class QStackedWidget;
class QModelIndex;
class KUrlRequester;
class ImpTreeView;
class PrefsWages;
class PrefsUnits;
class HtmlView;

// ################################################################################

class PrefsDialog : public KPageDialog
{
  Q_OBJECT

public:
  PrefsDialog(QWidget *parent);

  ~PrefsDialog();

  void setMyIdentity( const KABC::Addressee& );
  KABC::Addressee myIdentity();

protected:
  void readConfig();
  void writeConfig();

protected slots:
  void accept();
  
  void slotAddTax();
  void slotDeleteTax();
  void slotTaxSelected(QModelIndex);
  void slotDocTypeRemoved( const QString& );
  void slotChangeIdentity();

signals:
  void newOwnIdentity(const QString&, KABC::Addressee);

private:
  void docTab();
  void doctypeTab();
  void taxTab();
  void wagesTab();
  void unitsTab();
  void writeTaxes();
  void whoIsMeTab();

  QComboBox *m_databaseDriver;
  QLineEdit *m_leHost;
  QLineEdit *m_leUser;
  QLineEdit *m_leName;
  QLineEdit *m_lePasswd;
  KUrlRequester *m_leFile;
  QLabel    *m_statusLabel;

  QWidget         *m_mysqlpart;
  QWidget         *m_sqlitepart;
  QStackedWidget  *m_databaseconfigparts;

  QComboBox *mCbDocTypes;
  QComboBox *mCbDefaultTaxType;

  QPushButton *m_pbCheck;

  QCheckBox *mCbDocLocale;

  DocTypeEdit *mDocTypeEdit;
  
  PrefsWages *mPrefsWages;
  PrefsUnits *mPrefsUnits;

  QPushButton    *mDelTax;
  ImpTreeView    *mTaxTreeView;
  QSqlTableModel *mTaxModel;
  HtmlView       *mIdentityView;

};

class TaxItemDelegate : public QItemDelegate
{
  Q_OBJECT

public:
  TaxItemDelegate(QObject * parent = 0);

  virtual void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

#endif
