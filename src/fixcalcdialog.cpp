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
#include <klocale.h>
#include <kdebug.h>
#include <knuminput.h>
#include <klineedit.h>

#include "fixcalcdialog.h"
#include "fixcalcpart.h"
#include "stdsatzman.h"
#include "defaultprovider.h"


FixCalcDialog::FixCalcDialog(QWidget *parent, bool modal )
    :KDialog( parent ), Ui::calcdetailFix(),
    m_part(0)
{
  QWidget *w = new QWidget( this );
  setMainWidget(w);

  setupUi( w );
  setModal( modal );
  m_inpPreis->setSuffix( DefaultProvider::self()->currencySymbol() );
}

FixCalcDialog::FixCalcDialog(FixCalcPart *cp, QWidget *parent, bool modal )
    : KDialog( parent ), Ui::calcdetailFix( ),
    m_part(0)
{
  QWidget *w = new QWidget( this );
  setMainWidget(w);

  setupUi( w );
  setModal( modal );
  setCalcPart(cp);
  m_inpPreis->setSuffix( DefaultProvider::self()->currencySymbol() );
}

void FixCalcDialog::setCalcPart( FixCalcPart *cp )
{
    if( ! cp ) return;
    m_part = cp;
    m_nameEdit->setText( cp->getName());
    m_inpMenge->setValue( cp->getMenge());
    m_inpPreis->setValue(cp->unitPreis().toDouble());
}


FixCalcDialog::~FixCalcDialog( )
{

}

void FixCalcDialog::accept()
{
  if( m_part ) {
    m_part->setMenge( m_inpMenge->value() );
    m_part->setName( m_nameEdit->text());
    m_part->setUnitPreis(Geld(m_inpPreis->value()));
  }

  if( m_part && m_part->isDirty() ) {
    emit fixCalcPartChanged(m_part);
  }

  KDialog::accept();
}

QString FixCalcDialog::getName()
{
    return m_nameEdit->text();
}

double FixCalcDialog::getMenge()
{
    return m_inpMenge->value();
}

double FixCalcDialog::getPreis()
{
    return m_inpPreis->value();
}


/* END */


#include "fixcalcdialog.moc"
