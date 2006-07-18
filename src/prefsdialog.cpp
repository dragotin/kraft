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

#include<kdialog.h>
#include<klocale.h>

#include "prefsdialog.h"
#include "katalogsettings.h"

PrefsDialog::PrefsDialog( QWidget *parent)
    : KDialogBase(parent, 0, true, i18n("Configure KAnge"), Ok|Cancel, Ok, true )
{
    QLabel *lable;
    QFrame *topFrame = makeMainWidget();

    QGridLayout *topLayout = new QGridLayout( topFrame );
    topLayout->setSpacing( spacingHint() );
    topLayout->setColSpacing( 0, spacingHint() );

    lable = new QLabel(i18n("<b>Database Settings</b>"), topFrame);
    topLayout->addMultiCellWidget(lable, 0,0,0,1);

    lable = new QLabel(i18n("Database Host:"), topFrame);
    topLayout->addWidget(lable, 1,0);

    lable = new QLabel(i18n("Database Name:"), topFrame);
    topLayout->addWidget(lable, 2,0);
    
    lable = new QLabel(i18n("Database User:"), topFrame);
    topLayout->addWidget(lable, 3,0);

    lable = new QLabel(i18n("Database Password:"), topFrame);
    topLayout->addWidget(lable, 4,0);

    m_leHost = new QLineEdit(topFrame);
    topLayout->addWidget(m_leHost, 1,1);

    m_leName = new QLineEdit(topFrame);
    topLayout->addWidget(m_leName, 2,1);

    m_leUser = new QLineEdit(topFrame);
    topLayout->addWidget(m_leUser, 3,1);

    m_lePasswd = new QLineEdit(topFrame);
    m_lePasswd->setEchoMode(QLineEdit::Password);
    topLayout->addWidget(m_lePasswd, 4,1);

    readConfig();
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

void PrefsDialog::slotOk()
{
    writeConfig();
    accept();
}

