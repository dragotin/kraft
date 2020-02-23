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

#include <kcontacts/addressee.h>

#include <QItemDelegate>

#include "ui_identity.h"

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

class PrefsDialog : public QDialog
{
  Q_OBJECT

public:
  PrefsDialog(QWidget *parent);

  ~PrefsDialog();

  void setMyIdentity(const KContacts::Addressee& , bool backendUp);
  int addPage( QWidget *w, const QIcon& icon, const QString& title);

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
  void changePage(QListWidgetItem *current);

signals:
  void newOwnIdentity(const QString&, KContacts::Addressee);

private:
  int addDialogPage( QWidget *w, const QIcon& icon, const QString& title);

  QWidget *docTab();
  QWidget* doctypeTab();
  QWidget *taxTab();
  void writeTaxes();
  void writeIdentity();
  QWidget *whoIsMeTab();
  void fillManualIdentityForm(const KContacts::Addressee& addressee);

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
  QComboBox *mCbDateFormats;
  QPushButton *_pbChangeIdentity;

  DocTypeEdit *mDocTypeEdit;
  
  PrefsWages *mPrefsWages;
  PrefsUnits *mPrefsUnits;

  KContacts::Addressee _newIdentity;

  QPushButton    *mDelTax;
  ImpTreeView    *mTaxTreeView;
  QSqlTableModel *mTaxModel;
  HtmlView       *mIdentityView;
  QListWidget    *_navigationBar;
  QStackedWidget *_pagesWidget;

  QTabWidget     *_tabWidget;
  Ui::manualOwnIdentity ui;

    int _maxNavBarTextWidth;
};

class TaxItemDelegate : public QItemDelegate
{
  Q_OBJECT

public:
  TaxItemDelegate(QObject * parent = 0);

  virtual void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

#endif
