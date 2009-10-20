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
#include <qlabel.h>

// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <knuminput.h>

#include "matcalcdialog.h"
#include "stockmaterial.h"

MatCalcDialog::MatCalcDialog(StockMaterial *mat, QWidget *parent, bool modal )
    : KDialog( parent ), Ui::calcdetailMat(),
    m_material(mat)
{
  setupUi( this );
  setModal( modal );
  init(1.0);
}

MatCalcDialog::MatCalcDialog(double amount, StockMaterial *mat, QWidget *parent, bool modal )
    : KDialog( parent ), Ui::calcdetailMat( ),
    m_material(mat)
{
  setupUi( this );
  setModal( modal );
  m_inpMenge->setValue(amount);
  init(amount);
}

void MatCalcDialog::init(double amount)
{
  if( ! m_material ) return;
  Einheit e = m_material->getUnit();

  matLabel->setText( m_material->name());
  einheitLabel->setText( e.einheit(amount) );
}

void MatCalcDialog::reject()
{
  reject();
}

void MatCalcDialog::accept()
{
  double val = m_inpMenge->value();
  emit( matCalcPartChanged(m_material, val ));
  accept();
}

double MatCalcDialog::getAmount()
{
  return m_inpMenge->value();
}

MatCalcDialog::~MatCalcDialog( )
{

}

/* END */


#include "matcalcdialog.moc"
