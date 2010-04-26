/***************************************************************************
             Timecalcdialog  -
                             -------------------
    begin                : 2004-23-09
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

// include files for Qt
#include <QComboBox>
#include <QCheckBox>

// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <knuminput.h>
#include <klineedit.h>

#include "timecalcdialog.h"
#include "timecalcpart.h"
#include "stdsatzman.h"


TimeCalcDialog::TimeCalcDialog(QWidget *parent, bool modal )
    : KDialog( parent ), Ui::calcdetailTime(),
    m_part(0)
{
  QWidget *w = new QWidget( this );
  setMainWidget(w);

  setupUi( w );
  setModal( modal );
  m_hourSets->insertItems(-1, StdSatzMan::self()->allStdSaetze());
}

TimeCalcDialog::TimeCalcDialog(TimeCalcPart *cp, QWidget *parent, bool modal )
    : KDialog( parent ), Ui::calcdetailTime(),
    m_part(cp)
{
  QWidget *w = new QWidget( this );
  setMainWidget(w);

  setupUi( w );
  setModal( modal );
  m_hourSets->insertItems(-1, StdSatzMan::self()->allStdSaetze());

  if( ! cp ) return;

  m_nameEdit->setText( cp->getName());
  m_dauer->setValue( cp->getMinuten());
  m_stdGlobal->setChecked(cp->globalStdSetAllowed());
  StdSatz std = cp->getStundensatz();
  m_hourSets->setCurrentIndex(m_hourSets->findText( std.getName() ));
}

TimeCalcDialog::~TimeCalcDialog( )
{

}

void TimeCalcDialog::accept()
{

  if( m_part ) {
    m_part->setGlobalStdSetAllowed(m_stdGlobal->isChecked());
    m_part->setMinuten(m_dauer->value());
    m_part->setName(m_nameEdit->text());

    QString selHourSet = m_hourSets->currentText();
    StdSatz stdsatz = StdSatzMan::self()->getStdSatz(selHourSet);

    m_part->setStundensatz(stdsatz);

  }

  if( m_part && m_part->isDirty() ) {
    emit timeCalcPartChanged(m_part);
  }

  KDialog::accept();
}

QString TimeCalcDialog::getName()
{
  return m_nameEdit->text();
}

int TimeCalcDialog::getDauer()
{
  return m_dauer->value();
}

bool TimeCalcDialog::allowGlobal()
{
  return m_stdGlobal->isChecked();
}

QString TimeCalcDialog::getStundensatzName()
{
  return m_hourSets->currentText();
}

/* END */
