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
#include <QComboBox>
#include <QString>

// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <knuminput.h>

#include "materialtempldialog.h"
#include "katalogman.h"
#include "unitmanager.h"
#include "geld.h"
#include "kraftsettings.h"
#include "defaultprovider.h"

MaterialTemplDialog::MaterialTemplDialog( QWidget *parent, bool modal )
    : KDialog( parent ),
    Ui::MaterialDialogBase(),
    Eta( 0.00000000001 )
{
  /* connect a value Changed signal of the manual price field */
  QWidget *w = new QWidget(this);
  setMainWidget(w);

  setupUi( w );
  setModal( modal );
  setButtons(KDialog::Ok | KDialog::Cancel);
  setDefaultButton(KDialog::Ok);
  showButtonSeparator( true);

  const QString currSymbol = DefaultProvider::self()->locale()->currencySymbol();
  mInPurchasePrice->setPrefix( currSymbol + " " );
  mInPurchasePrice->setMinimum( -999999.99 );
  mInPurchasePrice->setMaximum( 999999.99 );
  mInPurchasePrice->setDecimals( 2 );

  mInSalePrice->setPrefix( currSymbol + " " );
  mInSalePrice->setMinimum( -999999.99 );
  mInSalePrice->setMaximum( 999999.99 );
  mInSalePrice->setDecimals( 2 );

  double m = KraftSettings::self()->materialAddOnPercent();
  mInSaleAdd->setValue(m);
  mInSaleAdd->setMinimum(-99.0);
  mInSaleAdd->setMaximum(999.0);

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

  if ( m > Eta ) {
    sale = ( 1+m/100 )*purch;
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
  mInPurchasePrice->blockSignals(true);
  mInSalePrice->blockSignals(true);
  mInSaleAdd->blockSignals(true);
  mInPurchasePrice->setValue( purch );
  mInSalePrice->setValue( sale );
  mInSaleAdd->setValue( addPercent );
  mInPurchasePrice->blockSignals(false);
  mInSalePrice->blockSignals(false);
  mInSaleAdd->blockSignals(false);
}

void MaterialTemplDialog::setMaterial( StockMaterial *t, const QString& katalogname, bool newTempl )
{
  if( ! t ) return;
  mSaveMaterial = t;

  m_templateIsNew = newTempl;

  m_katalog = KatalogMan::self()->getKatalog(katalogname);

  if( m_katalog == 0 ) {
    kDebug() << "ERR: Floskel Dialog called without valid Katalog!" << endl;
    return;
  }

  // chapter settings
  QStringList chapterNames;
  foreach( CatalogChapter chap, m_katalog->getKatalogChapters() ) {
    chapterNames.append( chap.name() );
  }
  mCbChapter->insertItems(-1, chapterNames );
  mChapId = t->chapter();
  QString chap = m_katalog->chapterName(dbID( mChapId ));
  mCbChapter->setCurrentIndex(mCbChapter->findText( chap ));
  mCbChapter->setEnabled( false );

  // unit settings
  mCbUnit->insertItems(-1, UnitManager::self()->allUnits() );
  Einheit e = t->getUnit();
  mCbUnit->setCurrentIndex(mCbUnit->findText( e.einheitSingular() ));

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
  double percent = 10.0;
  if( priceIn > Eta ) {
    double diff = priceOut - priceIn;
    percent = diff / priceIn * 100.0;
  }
  mInSaleAdd->setValue( percent );
}


MaterialTemplDialog::~MaterialTemplDialog( )
{
}

void MaterialTemplDialog::accept()
{
  kDebug() << "*** Saving finished " << endl;
  const QString newMat = mEditMaterial->toPlainText();

  if ( newMat.isEmpty() ) {
    kDebug() << "We do not want to store empty materials" << endl;
  } else {
    mSaveMaterial->setName( mEditMaterial->toPlainText() );
    mSaveMaterial->setAmountPerPack( mDiPerPack->value() );

    const QString str = mCbUnit->currentText();

    int u = UnitManager::self()->getUnitIDSingular( str );
    kDebug() << "Setting unit id "  << u << endl;
    mSaveMaterial->setUnit( UnitManager::self()->getUnit( u ) );

    // chapId = 0; // FIXME: get a chapter catalog Id of hirarchical
    mSaveMaterial->setChapter( mChapId );

    double db = mInPurchasePrice->value();
    mSaveMaterial->setPurchPrice( Geld( db ) );

    db = mInSalePrice->value();
    mSaveMaterial->setSalesPrice( Geld( db ) );

    mSaveMaterial->save();

    emit editAccepted( mSaveMaterial );
    KatalogMan::self()->notifyKatalogChange( m_katalog, mSaveMaterial->getID() );
  }

  KDialog::accept();
}

void MaterialTemplDialog::reject()
{
  if ( m_templateIsNew ) {
    // remove the listview item if it was created newly
    emit editRejected();
  }
  KDialog::reject();
}
