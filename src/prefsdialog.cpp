/***************************************************************************
                   prefsdialog.cpp  - the preferences Dialog
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

#include<qlayout.h>
#include<qlineedit.h>
#include <qlineedit.h>
#include<qlabel.h>
#include<qframe.h>
#include <qhbox.h>
#include <qpushbutton.h>

#include<kdialog.h>
#include<klocale.h>

#include "prefsdialog.h"
#include "katalogsettings.h"
#include "kraftdb.h"


PrefsDialog::PrefsDialog( QWidget *parent)
    : KDialogBase(parent, 0, true, i18n("Configure Kraft"), Ok|Cancel|User1, Ok, true )
{
  setButtonText( User1, i18n( "try to connect..." ) );
  actionButton( User1 )->setEnabled( false );

  QLabel *label;
  QFrame *topFrame = makeMainWidget();

  QGridLayout *topLayout = new QGridLayout( topFrame );
  topLayout->setSpacing( spacingHint() );
  topLayout->setColSpacing( 0, spacingHint() );

  label = new QLabel(i18n("Database Settings"), topFrame );
  QFont f = label->font();
  f.setPointSize( qRound( 1.4 * f.pointSize() ) );
  f.setBold( true );
  label->setFont( f );

  topLayout->addMultiCellWidget(label, 0,0,0,1);

  label = new QLabel(i18n("Database Host:"), topFrame );
  topLayout->addWidget(label, 1,0);

  label = new QLabel(i18n("Database Name:"), topFrame );
  topLayout->addWidget(label, 2,0);

  label = new QLabel(i18n("Database User:"), topFrame );
  topLayout->addWidget(label, 3,0);

  label = new QLabel(i18n("Database Password:"), topFrame );
  topLayout->addWidget(label, 4,0);

  label = new QLabel(i18n("Connection Status:"), topFrame );
  topLayout->addWidget(label, 5,0);

  m_leHost = new QLineEdit( topFrame );
  connect( m_leHost, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( slotTextChanged( const QString& ) ) );
  topLayout->addWidget(m_leHost, 1,1);

  m_leName = new QLineEdit( topFrame );
  connect( m_leName, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( slotTextChanged( const QString& ) ) );
  topLayout->addWidget(m_leName, 2,1);

  m_leUser = new QLineEdit( topFrame );
  connect( m_leUser, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( slotTextChanged( const QString& ) ) );
  topLayout->addWidget(m_leUser, 3,1);

  m_lePasswd = new QLineEdit( topFrame );
  m_lePasswd->setEchoMode(QLineEdit::Password);
  connect( m_lePasswd, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( slotTextChanged( const QString& ) ) );
  topLayout->addWidget(m_lePasswd, 4,1);

  m_statusLabel = new QLabel( topFrame );
  topLayout->addWidget( m_statusLabel,  5, 1 );

  readConfig();
  slotUser1();
}

void PrefsDialog::slotTextChanged( const QString& )
{
  bool en = false;
  if ( !m_leName->text().isEmpty() ) {
    en = true;
  }

  QPushButton *pb = actionButton ( User1 );
  if ( pb ) {
    pb->setEnabled( en );
  }
}

void PrefsDialog::readConfig()
{
    m_leHost->setText( KatalogSettings::dbServerName() );
    m_leName->setText( KatalogSettings::dbFile() );
    m_leUser->setText( KatalogSettings::dbUser() );
    m_lePasswd->setText( KatalogSettings::dbPassword() );
}

void PrefsDialog::writeConfig()
{
    KatalogSettings::setDbServerName(m_leHost->text());
    KatalogSettings::setDbFile(m_leName->text());
    KatalogSettings::setDbUser(m_leUser->text());
    KatalogSettings::setDbPassword( m_lePasswd->text());
    KatalogSettings::writeConfig();
}

PrefsDialog::~PrefsDialog()
{
}

void PrefsDialog::slotUser1()
{
  kdDebug() << "Trying database connect to db " << m_leName->text() << endl;

  bool x = KraftDB::self()->checkConnect( m_leHost->text(), m_leName->text(),
                                          m_leUser->text(), m_lePasswd->text() );
  kdDebug() << "Connection result: " << x << endl;
  if ( x ) {
    m_statusLabel->setText( i18n( "Good!" ) );
  } else {
    m_statusLabel->setText( i18n( "Failed" ) );
  }
}

void PrefsDialog::slotOk()
{
    writeConfig();
    accept();
}

#include "prefsdialog.moc"
