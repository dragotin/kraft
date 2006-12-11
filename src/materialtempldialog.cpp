/***************************************************************************
             materialtempldialog  - dialog to edit material templates
                             -------------------
    begin                : 2006-12-03
    copyright            : (C) 2006 by Klaas Freitag
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
#include <qtextedit.h>
#include <qstring.h>

// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <knuminput.h>

#include "materialtempldialog.h"
#include "katalogman.h"
#include "unitmanager.h"
#include "geld.h"

MaterialTemplDialog::MaterialTemplDialog( QWidget *parent, const char* name, bool modal, WFlags fl)
    : MaterialDialogBase(parent, name, modal, fl),
      Eta( 0.00000000001 )
{
    /* connect a value Changed signal of the manual price field */
  const QString currSymbol = KGlobal().locale()->currencySymbol();
  mInPurchasePrice->setPrefix( currSymbol + " " );
  mInSalePrice->setPrefix( currSymbol + " " );

  connect( mInSalePrice, SIGNAL( valueChanged( double ) ),
           SLOT( slSalePriceChanged( double ) ) );
  connect( mInPurchasePrice, SIGNAL( valueChanged( double ) ),
           SLOT( slPurchPriceChanged( double ) ) );
  connect( mInSaleAdd, SIGNAL( valueChanged( double ) ),
           SLOT( slSaleAddChanged( double ) ) );


}

void MaterialTemplDialog::slSalePriceChanged( double sale )
{
  // change the percent val
  double purch = mInPurchasePrice->value();
  double m = mInSaleAdd->value();

  if ( m > Eta && purch < Eta ) {
    // recalc the purchase price
    purch = sale/( 1+ m / 100.0 );
  } else if ( purch > Eta ) {
    // recalc the add-percentage
    m = 100 * ( ( sale-purch ) / purch );
  }
  setPriceCalc( purch, m, sale );
}

void MaterialTemplDialog::slPurchPriceChanged( double purch )
{
  // change the percent val
  double sale = mInSalePrice->value();
  double m = mInSaleAdd->value();

  if ( m > Eta && sale < Eta ) {
    sale = ( 1+m/100 )*purch;
  } else if ( sale > Eta ) {
    m = 100*( ( sale-purch )/purch );
  }
  setPriceCalc( purch, m, sale );
}

void MaterialTemplDialog::slSaleAddChanged( double m )
{
  // change the Sales Price
  double sale  = mInSalePrice->value();
  double purch = mInPurchasePrice->value();

  if (purch < Eta && sale > Eta ) {
    // calc the purchase price
    purch = sale/( 1+ m/100 );
  } else {
    sale = ( 1+ m/100.0 ) * purch;
  }
  setPriceCalc( purch, m, sale );
}

void MaterialTemplDialog::setPriceCalc( double purch, double addPercent, double sale )
{
  mInPurchasePrice->setValue( purch );
  mInSalePrice->setValue( sale );
  mInSaleAdd->setValue( addPercent );
}

void MaterialTemplDialog::setMaterial( StockMaterial *t, const QString& katalogname, bool newTempl )
{
  if( ! t ) return;
  mSaveMaterial = t;

  m_templateIsNew = newTempl;

  m_katalog = KatalogMan::self()->getKatalog(katalogname);

  if( m_katalog == 0 ) {
    kdDebug() << "ERR: Floskel Dialog called without valid Katalog!" << endl;
    return;
  }

  // chapter settings
  mCbChapter->insertStringList( m_katalog->getKatalogChapters() );
  int chapID = t->chapter();
  QString chap = m_katalog->chapterName(dbID(chapID));
  mCbChapter->setCurrentText(chap);

  // unit settings
  mCbUnit->insertStringList( UnitManager::allUnits() );
  Einheit e = t->getUnit();
  mCbUnit->setCurrentText( e.einheitSingular() );

  // text
  mEditMaterial->setText( t->name() );

  double priceIn = t->purchPrice().toDouble();
  double priceOut = t->salesPrice().toDouble();

  mInPurchasePrice->setValue( priceIn );
  mInSalePrice->setValue( priceOut );

  mDiPerPack->setValue( t->getAmountPerPack() );

  // user experience
  mEditMaterial->setFocus();
  mEditMaterial->selectAll();

  // percent add on sale price
  double diff = priceOut - priceIn;
  double percent = diff / priceIn * 100.0;
  mInSaleAdd->setValue( percent );
}


MaterialTemplDialog::~MaterialTemplDialog( )
{
}

void MaterialTemplDialog::accept()
{
  kdDebug() << "*** Saving finished " << endl;
  const QString newMat = mEditMaterial->text();

  if ( newMat.isEmpty() ) {
    kdDebug() << "We do not want to store empty materials" << endl;
  } else {
    mSaveMaterial->setName( mEditMaterial->text() );
    mSaveMaterial->setAmountPerPack( mDiPerPack->value() );

    const QString str = mCbUnit->currentText();

    int u = UnitManager::getUnitIDSingular( str );
    kdDebug() << "Setting unit id "  << u << endl;
    mSaveMaterial->setUnit( UnitManager::getUnit( u ) );

    const QString str2 = mCbChapter->currentText();
    int chapId = m_katalog->chapterID( str2 );
    if ( !templateIsNew() && chapId != mSaveMaterial->chapter() ) {
      if( askChapterChange( mSaveMaterial, chapId )) {
        mSaveMaterial->setChapter( chapId );
        emit( chapterChanged( chapId ));
      }
    }

    mSaveMaterial->setChapter( m_katalog->chapterID( str2 ) );

    double db = mInPurchasePrice->value();
    mSaveMaterial->setPurchPrice( Geld( db ) );

    db = mInSalePrice->value();
    mSaveMaterial->setSalesPrice( Geld( db ) );

    mSaveMaterial->save();

    emit editAccepted( mSaveMaterial );
  }

  MaterialDialogBase::accept();
}

bool MaterialTemplDialog::askChapterChange( StockMaterial*, int )
{
    if( KMessageBox::questionYesNo( this,
        i18n( "The catalog chapter was changed for this template.\nDo you really want to move the template to the new chapter?"),
        i18n("Changed Chapter"), KStdGuiItem::yes(), KStdGuiItem::no(),
        "chapterchange" ) == KMessageBox::Yes )
    {
        return true;

    } else {
        return false;
    }
}

void MaterialTemplDialog::reject()
{
    if ( m_templateIsNew ) {
      // remove the listview item if it was created newly
      emit editRejected();
    }
    MaterialDialogBase::reject();
}


#include "materialtempldialog.moc"
