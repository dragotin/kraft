/***************************************************************************
             zeitcalcdialog  -
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
#include <qcombobox.h>
#include <qcheckbox.h>

// include files for KDE
#include <QDebug>

#include "fixcalcdialog.h"
#include "fixcalcpart.h"
#include "stdsatzman.h"
#include "defaultprovider.h"


FixCalcDialog::FixCalcDialog(QWidget *parent)
    :CalcDialogBase(parent),
      _fixWidget(new Ui_calcdetailFix),
      m_part(0)
{
    setWindowTitle( i18n("Calculation Fix Item"));
    _fixWidget->setupUi(_centralWidget);
    _fixWidget->m_inpPreis->setSuffix( DefaultProvider::self()->currencySymbol() );
}

void FixCalcDialog::setCalcPart( FixCalcPart *cp )
{
    if( ! cp ) return;
    m_part = cp;
    _fixWidget->m_nameEdit->setText( cp->getName());
    _fixWidget->m_inpMenge->setValue( cp->getMenge());
    _fixWidget->m_inpPreis->setValue(cp->unitPreis().toDouble());
}

void FixCalcDialog::accept()
{
  if( m_part ) {
    m_part->setMenge( _fixWidget->m_inpMenge->value() );
    m_part->setName( _fixWidget->m_nameEdit->text());
    m_part->setUnitPreis(Geld(_fixWidget->m_inpPreis->value()));
  }

  if( m_part && m_part->isDirty() ) {
    emit fixCalcPartChanged(m_part);
  }

  CalcDialogBase::accept();
}

QString FixCalcDialog::getName()
{
    return _fixWidget->m_nameEdit->text();
}

double FixCalcDialog::getMenge()
{
    return _fixWidget->m_inpMenge->value();
}

double FixCalcDialog::getPreis()
{
    return _fixWidget->m_inpPreis->value();
}


/* END */

