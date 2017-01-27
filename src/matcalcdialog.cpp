/***************************************************************************
             matcalcdialog  -
                             -------------------
    begin                : 2005-03-00
    copyright            : (C) 2005 by Klaas Freitag
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
#include <QLabel>
#include <QDebug>
#include <QDoubleSpinBox>

#include "matcalcdialog.h"
#include "materialcalcpart.h"
#include "stockmaterial.h"

MatCalcDialog::MatCalcDialog( MaterialCalcPart *mc, QWidget *parent, bool modal )
    : QDialog( parent ), Ui::calcdetailMat( ),
    m_mc(mc)
{
  QWidget *w = new QWidget( this );
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  mainLayout->addWidget(w);

  setupUi( w );
  setModal( modal );
  m_inpMenge->setValue(mc->getCalcAmount());
  init(mc->getCalcAmount());
}

void MatCalcDialog::init(double amount)
{
  Einheit e = m_mc->getMaterial()->getUnit();

  matLabel->setText( m_mc->getMaterial()->name());
  einheitLabel->setText( e.einheit(amount) );
}

void MatCalcDialog::reject()
{
  QDialog::reject();
}

void MatCalcDialog::accept()
{
  double val = m_inpMenge->value();
  m_mc->setCalcAmount(val);
  emit( matCalcPartChanged(m_mc));

  QDialog::accept();
}

MatCalcDialog::~MatCalcDialog( )
{

}

/* END */

