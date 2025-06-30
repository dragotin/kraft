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
#include <kcontacts/addressee.h>

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
class ImpTreeView;
class PrefsWages;
class PrefsUnits;
class HtmlView;
class MyIdentity;

// ################################################################################

class PrefsDialog : public QDialog
{
    Q_OBJECT

public:
    PrefsDialog(QWidget *parent);

    ~PrefsDialog();

    void setMyIdentity(const KContacts::Addressee &addressee , bool backendUp);
    int addPage( QWidget *w, const QIcon& icon, const QString& title);

    void setMyIdentity(MyIdentity *identity);
protected:
    void readConfig();
    void writeConfig();

protected Q_SLOTS:
    void accept();

    void slotAddTax();
    void slotDeleteTax();
    void slotTaxSelected(QModelIndex);
    void slotDocTypeRemoved( const QString& );
    void slotChangeIdentity();
    void changePage(QListWidgetItem *current);

private:
    int addDialogPage( QWidget *w, const QIcon& icon, const QString& title);
    void displayOwnAddress(const KContacts::Addressee& addressee, bool backendUp);

    QWidget *docTab();
    QWidget* doctypeTab();
    QWidget *taxTab();
    void writeTaxes();
    void writeIdentity(int currIndx);
    QWidget *whoIsMeTab();
    void fillManualIdentityForm(const KContacts::Addressee& addressee);

    QComboBox *m_databaseDriver;
    QLineEdit *m_leHost;
    QLineEdit *m_leUser;
    QLineEdit *m_leName;
    QLineEdit *m_lePasswd;
    QLineEdit *_lineEditXRechnung;
    QLineEdit *_lineEditDemandText;
    QLineEdit *_lineEditAlternativeText;
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

    QPushButton    *mDelTax;
    ImpTreeView    *mTaxTreeView;
    QSqlTableModel *mTaxModel;
    HtmlView       *mIdentityView;
    QListWidget    *_navigationBar;
    QStackedWidget *_pagesWidget;

    QTabWidget     *_tabWidget;
    Ui::manualOwnIdentity _ownIdentUi;

    QLineEdit *_bacName;
    QLineEdit *_bacIBAN;
    QLineEdit *_bacBIC;

    MyIdentity *_myIdentity;
    KContacts::Addressee _newOwnAddress;

    int _maxNavBarTextWidth;
    int _whoIndx;
};

class TaxItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    TaxItemDelegate(QObject * parent = 0);

    virtual void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

#endif
