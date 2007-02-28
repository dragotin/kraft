/***************************************************************************
  insertplantdialog.cpp  - small dialog to insert templates into documents
                             -------------------
    begin                : Sep 2006
    copyright            : (C) 2006 Klaas Freitag
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "insertplantdialog.h"
#include "docposition.h"

// include files for Qt
#include <qvbox.h>
#include <qtextedit.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qsqlquery.h>

// include files for KDE
#include <knuminput.h>
#include <klistview.h>
#include <kglobal.h>

#include <klocale.h>
#include <kdebug.h>

#include "insertplantbase.h"
#include "templtopositiondialogbase.h"
#include "brunskatalog.h"
#include "brunsrecord.h"
#include "unitmanager.h"
#include "geld.h"

InsertPlantDialog::InsertPlantDialog( QWidget *parent )
  : TemplToPositionDialogBase( parent )
{
  QWidget *w = makeVBoxMainWidget();

  mBaseWidget = new insertPlantBase( w );
  mBaseWidget->mSinglePrice->setSuffix( KGlobal().locale()->currencySymbol() );

  mBaseWidget->mSizeList->addColumn( i18n( "Matchcode" ) );
  mBaseWidget->mSizeList->addColumn( i18n( "Form" ) );
  mBaseWidget->mSizeList->addColumn( i18n( "Form Add" ) );
  mBaseWidget->mSizeList->addColumn( i18n( "Growth" ) );
  mBaseWidget->mSizeList->addColumn( i18n( "Root" ) );
  mBaseWidget->mSizeList->addColumn( i18n( "Quality"));
  mBaseWidget->mSizeList->addColumn( i18n( "Group" ));

  mBaseWidget->mSizeList->setAllColumnsShowFocus ( true ) ;
  mBaseWidget->mSizeList->setSelectionMode( QListView::Single );

  connect( mBaseWidget->mSizeList,  SIGNAL( selectionChanged() ),
           this,  SLOT( slotSizeListSelectionChanged() ) );

  actionButton( Ok )->setEnabled( false );

  mPriceQuery.prepare( "SELECT price, lastUpdate FROM plantPrices WHERE matchCode=:match" );
  mBaseWidget->mUpdateLabel->setText( QString() );
}

void InsertPlantDialog::slotSizeListSelectionChanged()
{
  bool res = false;
  KListViewItem *item = static_cast<KListViewItem*>( mBaseWidget->mSizeList->currentItem() );
  if ( item ) {
    res = true;
  }
  actionButton( Ok )->setEnabled( res );

  PlantPriceInfo ppi = mPriceMap[item];

  mBaseWidget->mSinglePrice->setValue( ppi.price().toDouble() );
  QString h;
  if ( ppi.price().toLong() > 0 ) {
    h = i18n( "(price entered in %1/%2)" )
        .arg( ppi.lastUpdateDate().month() ).arg( ppi.lastUpdateDate().year() );
  }
  mBaseWidget->mUpdateLabel->setText( h );
}

void InsertPlantDialog::setDocPosition( DocPosition* )
{
  mBaseWidget->mSizeList->setFocus();
}

QComboBox *InsertPlantDialog::getPositionCombo()
{
  return mBaseWidget->dmPositionCombo;
}

DocPosition InsertPlantDialog::docPosition()
{
  DocPosition pos;
  QString h;

  h = mLtName;
  if ( ! mDtName.isEmpty() ) {
    h += " - " + mDtName;
  }

  h += "\n";

  KListViewItem *guiItem = static_cast<KListViewItem*>( mBaseWidget->mSizeList->currentItem() );

  if ( guiItem ) {
    h += mSizeMap[guiItem];
  }

  pos.setText( h );
  pos.setAmount( 1.0 );
  pos.setUnit( UnitManager::getUnit( PIECE_UNIT_ID ) );

  double p = mBaseWidget->mSinglePrice->value();
  pos.setUnitPrice( Geld( p ) );
  pos.setAmount( mBaseWidget->mAmountSelector->value() );
  return pos;
}

void InsertPlantDialog::setSelectedPlant( BrunsRecord* bruns )
{
  if( !bruns ) return;

  mBaseWidget->mResultLabel->setText( bruns->getLtName() );
  mLtName = bruns->getLtName();
  mDtName = bruns->getDtName();

  BrunsSizeList sizes = bruns->getSizes();

  BrunsSizeList::iterator it;
  for( it = sizes.begin(); it != sizes.end(); ++it ) {
    QString match = ( *it ).getPrimMatchcode();
    KListViewItem *guiItem = new KListViewItem(mBaseWidget->mSizeList, match );

    QStringList list = BrunsKatalog::formatQuality( (*it) );
    int i = 1;
    for ( QStringList::Iterator listIt = list.begin(); listIt != list.end(); ++listIt ) {
      guiItem->setText( i++, (*listIt) );
    }
    QString qualString = list[2] + ", " + list[3] + ", " + list[0] + " " + list[1];

    mSizeMap[guiItem] = ( qualString );
    mPriceMap[guiItem] = getPriceInfo( match );

    // kdDebug() << "showing new plant detail item" << endl;
  }
}

PlantPriceInfo InsertPlantDialog::getPriceInfo( const QString& matchcode )
{
  Geld g;

  mPriceQuery.bindValue( ":match", matchcode );
  mPriceQuery.exec();

  if ( mPriceQuery.next() ) {
    double p = mPriceQuery.value( 0 ).toDouble();
    QDate d = mPriceQuery.value( 1 ).toDate();
    return PlantPriceInfo( p, d );
  }
  return PlantPriceInfo();
}

void InsertPlantDialog::setPrice( const QString& match, double g )
{
  QSqlQuery q;
  kdDebug() << "Writing price for " << match << ": " << g << endl;
  Geld price = getPriceInfo( match ).price();
  if ( price.toDouble() == g ) return;

  if ( price.toLong() > 0 ) {
    q.prepare( "UPDATE plantPrices SET price=:price WHERE matchCode=:match" );
  } else {
    q.prepare( "INSERT INTO plantPrices (matchCode, price) VALUES( :match, :price )" );
  }
  q.bindValue( ":match",  match );
  q.bindValue( ":price",  g );
  q.exec();
}

void InsertPlantDialog::slotOk()
{
  double d = mBaseWidget->mSinglePrice->value();
  if ( mBaseWidget->mSizeList->currentItem() ) {
    QString m = mBaseWidget->mSizeList->currentItem()->text( 0 );

    if ( d > 0 )
      setPrice( m, d );
  }
  TemplToPositionDialogBase::slotOk();
}



InsertPlantDialog::~InsertPlantDialog()
{

}

#include "insertplantdialog.moc"

/* END */

