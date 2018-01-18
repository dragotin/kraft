/***************************************************************************
        setupassistant  - assistant to setup kraft from scratch
                             -------------------
    begin                : 2009-12-26
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

#ifndef SETUPASSISTANT_H
#define SETUPASSISTANT_H

#include <QtGui>
#include <QWizard>

#include <kcontacts/addressee.h>

#include "ui_statuspage.h"
#include "ui_dbselect.h"
#include "ui_mysqldetails.h"
#include "ui_createdb.h"
#include "ui_upgradedb.h"
#include "ui_sqlitedetails.h"
#include "ui_identity.h"

#include "kraftcat_export.h"

class QUrl;
class AddressSelectorWidget;

using namespace KContacts;

class WelcomePage:public QWizardPage
{
  Q_OBJECT

  public:
  WelcomePage( QWidget *parent = 0 );

  void setWelcomeText( const QString& );
  private:
  Ui::statusPage ui;
};

// ---------------------------------------------------------------------------

class DbSelectPage:public QWizardPage
{
  Q_OBJECT

  public:
  DbSelectPage( QWidget *parent = 0 );
  QString selectedDriver() const;
  int nextId() const;

  private:
  Ui::dbSelectForm ui;
};

// ---------------------------------------------------------------------------

class SqLiteDetailsPage:public QWizardPage
{
    Q_OBJECT

  public:
    SqLiteDetailsPage( QWidget *parent = 0 );

    QUrl url();
    int nextId() const;
    bool validatePage();

  private:
    Ui::sqLiteDetailsForm ui;
};

// ---------------------------------------------------------------------------

class MysqlDetailsPage:public QWizardPage
{
    Q_OBJECT

  public:
    MysqlDetailsPage( QWidget *parent = 0 );

    void reloadSettings();
    int nextId() const;
    bool validatePage();

  private:
    Ui::mySqlDetailsForm ui;
};

// ---------------------------------------------------------------------------

class CreateDbPage:public QWizardPage
{
  Q_OBJECT

  public:
  CreateDbPage( QWidget *parent = 0 );

  void setStatusText( const QString& );
  void setCreateCmdsCount( int );
  void setFillCmdsCount( int );

  void setCreateCmdsCurrent( int );
  void setFillCmdsCurrent( int );

  int nextId() const;
  public slots:
  void slotStatusMessage( const QString& );
  void slotCountCreateProgress( bool );
  void slotCountFillProgress( bool );

  private:
  Ui::createDbForm ui;
  int mCreates;
  int mFills;
};

// ---------------------------------------------------------------------------

class UpgradeDbPage:public QWizardPage
{
  Q_OBJECT

  public:
  UpgradeDbPage( QWidget *parent = 0 );

  int nextId() const;

  public slots:
  void slotSetStatusText( const QString& );
  void slotSetOverallCount( int );
  void slotCountFillProgress( bool );

  private:
  void updateCounter();
  Ui::upgradeDbForm ui;
  int mUpgrades;
};

// ---------------------------------------------------------------------------

class OwnAddressPage:public QWizardPage
{
  Q_OBJECT

  public:
  OwnAddressPage( QWidget *parent=0 );
  ~OwnAddressPage();

  void saveOwnName();

  int nextId() const;
  private:
  AddressSelectorWidget *mAddresses;
  KContacts::Addressee mMe;
  Ui::manualOwnIdentity ui;

  private slots:
  void gotMyAddress( const KContacts::Addressee& addressee);

};

// ---------------------------------------------------------------------------

class FinalStatusPage:public QWizardPage
{
  Q_OBJECT

  public:
  FinalStatusPage( QWidget *parent = 0 );
  int nextId() const;

  public slots:
  void slotSetStatusText( const QString& );

  private:
  Ui::statusPage ui;
};

// ---------------------------------------------------------------------------

class SetupAssistant: public QWizard
{
    Q_OBJECT

public:
    enum { welcomePageNo,
           dbSelectPageNo,
           mySqlPageNo,
           sqlitePageNo,
           createDbPageNo,
           upgradeDbPageNo,
           finalStatusPageNo,
           ownAddressPageNo };

    enum Mode{ Reinit, Update };

    SetupAssistant( QWidget *parent = 0 );
    bool init( Mode );

    ~SetupAssistant();

    bool handleSqLiteDetails();
    bool handleMysqlDetails();

public slots:
    void done( int );

private slots:
    void slotCurrentPageChanged(int currId);

private:
    void startDatabaseCreation();
    void startDatabaseUpdate();
    void finalizePage();
    QString defaultSqliteFilename() const;


    Mode mMode;
    QStringList mErrors;
    QString mSqlBackendDriver;
};

#endif // SETUPASSISTANT_H
