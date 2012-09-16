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

#include <kassistantdialog.h>
#include <kabc/addressee.h>

#include <akonadi/item.h>

#include "ui_statuspage.h"
#include "ui_dbselect.h"
#include "ui_mysqldetails.h"
#include "ui_createdb.h"
#include "ui_upgradedb.h"
#include "ui_sqlitedetails.h"

#include "kraftcat_export.h"

class KPageWidgetItem;
class KUrl;
class AkonadiAddressSelector;

using namespace KABC;

class WelcomePage:public QWidget
{
  Q_OBJECT

  public:
  WelcomePage( QWidget *parent = 0 );

  void setWelcomeText( const QString& );
  private:
  Ui::statusPage ui;
};

// ---------------------------------------------------------------------------

class DbSelectPage:public QWidget
{
  Q_OBJECT

  public:
  DbSelectPage( QWidget *parent = 0 );
  QString selectedDriver();

  private:
  Ui::dbSelectForm ui;
};

// ---------------------------------------------------------------------------

class SqLiteDetailsPage:public QWidget
{
  Q_OBJECT

  public:
  SqLiteDetailsPage( QWidget *parent = 0 );

  KUrl url();
  protected slots:
  void slotSelectCustom();
  private:
  Ui::sqLiteDetailsForm ui;
};

// ---------------------------------------------------------------------------

class MysqlDetailsPage:public QWidget
{
  Q_OBJECT

  public:
  MysqlDetailsPage( QWidget *parent = 0 );
  
  void reloadSettings();

  QString dbName();
  QString dbUser();
  QString dbServer();
  QString dbPasswd();

  private:
  Ui::mySqlDetailsForm ui;
};

// ---------------------------------------------------------------------------

class CreateDbPage:public QWidget
{
  Q_OBJECT

  public:
  CreateDbPage( QWidget *parent = 0 );

  void setStatusText( const QString& );
  void setCreateCmdsCount( int );
  void setFillCmdsCount( int );

  void setCreateCmdsCurrent( int );
  void setFillCmdsCurrent( int );

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

class UpgradeDbPage:public QWidget
{
  Q_OBJECT

  public:
  UpgradeDbPage( QWidget *parent = 0 );

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

class OwnAddressPage:public QWidget
{
  Q_OBJECT

  public:
  OwnAddressPage( QWidget *parent=0 );
  ~OwnAddressPage();

  void saveOwnName();

  private:
  AkonadiAddressSelector *mAddresses;
  KABC::Addressee mMe;

  private slots:
  void contactStored( const Akonadi::Item& );
  void gotMyAddress( Addressee );

};

// ---------------------------------------------------------------------------

class FinalStatusPage:public QWidget
{
  Q_OBJECT

  public:
  FinalStatusPage( QWidget *parent = 0 );

  public slots:
  void slotSetStatusText( const QString& );

  private:
  Ui::statusPage ui;
};

// ---------------------------------------------------------------------------

class SetupAssistant: public KAssistantDialog
{
  Q_OBJECT

public:
  enum Mode{ Reinit, Update };

  SetupAssistant( QWidget *parent = 0 );
  bool init( Mode );
  void createDatabase( bool );

  ~SetupAssistant();

public slots:
  void back();
  void next();
  void slotFinishedClicked();

private slots:
  void slotCurrentPageChanged( KPageWidgetItem*, KPageWidgetItem* );

private:
  void handleDatabaseBackendSelect();
  void handleSqLiteDetails();
  void handleMysqlDetails();
  void startDatabaseCreation();
  void startDatabaseUpdate();
  void finalizePage();
  bool tryMigrateFromKDE3();

  WelcomePage       *mWelcomePage;
  DbSelectPage      *mDbSelectPage;
  MysqlDetailsPage  *mMysqlDetailsPage;
  CreateDbPage      *mCreateDbPage;
  UpgradeDbPage     *mUpgradeDbPage;
  FinalStatusPage   *mFinalStatusPage;
  SqLiteDetailsPage *mSqLiteDetailsPage;
  OwnAddressPage    *mOwnAddressPage;

  KPageWidgetItem *mWelcomePageItem;
  KPageWidgetItem *mDbSelectPageItem;
  KPageWidgetItem *mMysqlDetailsPageItem;
  KPageWidgetItem *mSqLiteDetailsPageItem;
  KPageWidgetItem *mCreateDbPageItem;
  KPageWidgetItem *mUpgradeDbPageItem;
  KPageWidgetItem *mFinalStatusPageItem;
  KPageWidgetItem *mOwnAddressPageItem;

  Mode mMode;
  QStringList mErrors;
  QString mSqlBackendDriver;
};

#endif // SETUPASSISTANT_H
