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
#include <QDebug>
#include <QDialog>

#include "timecalcdialog.h"
#include "timecalcpart.h"
#include "stdsatzman.h"

TimeCalcDialog::TimeCalcDialog(QWidget *parent)
    : CalcDialogBase( parent ),
      _timeWidget(new Ui::calcdetailTime),
      _part(0)
{
    _timeWidget->setupUi(_centralWidget);

    _timeWidget->m_hourSets->insertItems(-1, StdSatzMan::self()->allStdSaetze());
    _timeWidget->_cbTimeUnit->addItems(TimeCalcPart::timeUnitStrings());
}

void TimeCalcDialog::setTimeCalcPart(TimeCalcPart *cp)
{
    _part = cp;
    if( ! cp ) return;

    _timeWidget->m_nameEdit->setText( cp->getName());
    _timeWidget->m_dauer->setValue( cp->duration());
    _timeWidget->_cbTimeUnit->setCurrentText( TimeCalcPart::timeUnitString(cp->timeUnit()));
    _timeWidget->m_stdGlobal->setChecked(cp->globalStdSetAllowed());
    StdSatz std = cp->getStundensatz();
    _timeWidget->m_hourSets->setCurrentIndex(_timeWidget->m_hourSets->findText( std.getName() ));
}

void TimeCalcDialog::accept()
{
  if( _part ) {
    _part->setGlobalStdSetAllowed(_timeWidget->m_stdGlobal->isChecked());
    _part->setDuration(_timeWidget->m_dauer->value(), _timeWidget->_cbTimeUnit->currentText());
    _part->setName(_timeWidget->m_nameEdit->text());

    QString selHourSet = _timeWidget->m_hourSets->currentText();
    StdSatz stdsatz = StdSatzMan::self()->getStdSatz(selHourSet);

    _part->setStundensatz(stdsatz);
  }

  if( _part && _part->isDirty() ) {
    emit timeCalcPartChanged(_part);
  }

  CalcDialogBase::accept();
}

QString TimeCalcDialog::getName()
{
  return _timeWidget->m_nameEdit->text();
}

int TimeCalcDialog::getDauer()
{
  return _timeWidget->m_dauer->value();
}

bool TimeCalcDialog::allowGlobal()
{
  return _timeWidget->m_stdGlobal->isChecked();
}

QString TimeCalcDialog::getStundensatzName()
{
  return _timeWidget->m_hourSets->currentText();
}

QString TimeCalcDialog::unitStr() const
{
    return _timeWidget->_cbTimeUnit->currentText();
}

/* END */
