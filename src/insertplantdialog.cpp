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

#include "insertplantdialog.h"
#include "docposition.h"

// include files for Qt
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QSqlQuery>

// include files for KDE
#include <knuminput.h>
#include <kvbox.h>
#include <kglobal.h>
#include <kpushbutton.h>

#include <klocale.h>
#include <kdebug.h>

#include "templtopositiondialogbase.h"
#include "brunskatalog.h"
#include "brunsrecord.h"
#include "unitmanager.h"
#include "geld.h"
#include "defaultprovider.h"

InsertPlantDialog::InsertPlantDialog( QWidget *parent )
  : TemplToPositionDialogBase( parent )
{
  QWidget *w = new QWidget( parent );
  setMainWidget( w );

  mBaseWidget = new Ui::insertPlantBase(  );
  mBaseWidget->setupUi( w );
  mBaseWidget->mSinglePrice->setSuffix( DefaultProvider::self()->locale()->currencySymbol() );

  mBaseWidget->mSizeList->setColumnCount( 7 );
  QStringList li;
  li << i18n( "Matchcode" );
  li << i18n( "Form" );
  li << i18n( "Form Add" );
  li << i18n( "Growth" );
  li << i18n( "Root" );
  li << i18n( "Quality" );
  li << i18n( "Group" );
  mBaseWidget->mSizeList->setHeaderLabels( li );

  mBaseWidget->mSizeList->setAllColumnsShowFocus ( true ) ;
  mBaseWidget->mSizeList->setSelectionMode( QAbstractItemView::SingleSelection );

  connect( mBaseWidget->mSizeList,  SIGNAL( selectionChanged() ),
           this,  SLOT( slotSizeListSelectionChanged() ) );

  button( Ok )->setEnabled( false );

  mPriceQuery.prepare( "SELECT price, lastUpdate FROM plantPrices WHERE matchCode=:match" );
  mBaseWidget->mUpdateLabel->setText( QString() );
}

void InsertPlantDialog::slotSizeListSelectionChanged()
{
  bool res = false;
  QTreeWidgetItem *item = static_cast<QTreeWidgetItem*>( mBaseWidget->mSizeList->currentItem() );
  if ( item ) {
    res = true;
  }
  button( Ok )->setEnabled( res );

  PlantPriceInfo ppi = mPriceMap[item];

  mBaseWidget->mSinglePrice->setValue( ppi.price().toDouble() );
  QString h;
  if ( ppi.price().toLong() > 0 ) {
    h = i18n( "(price entered in %1/%2)" )
        .arg( ppi.lastUpdateDate().month() ).arg( ppi.lastUpdateDate().year() );
  }
  mBaseWidget->mUpdateLabel->setText( h );
}

void InsertPlantDialog::setDocPosition( DocPosition*, bool, bool )
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

  QTreeWidgetItem *guiItem = static_cast<QTreeWidgetItem*>( mBaseWidget->mSizeList->currentItem() );

  if ( guiItem ) {
    h += mSizeMap[guiItem];
  }

  pos.setText( h );
  pos.setAmount( 1.0 );
  pos.setUnit( UnitManager::self()->getUnit( PIECE_UNIT_ID ) );

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
    QTreeWidgetItem *guiItem = new QTreeWidgetItem( mBaseWidget->mSizeList );
    guiItem->setText( 0, match );

    QStringList list = BrunsKatalog::formatQuality( (*it) );
    int i = 1;
    for ( QStringList::Iterator listIt = list.begin(); listIt != list.end(); ++listIt ) {
      guiItem->setText( i++, (*listIt) );
    }
    QString qualString = list[2] + ", " + list[3] + ", " + list[0] + " " + list[1];

    mSizeMap[guiItem] = ( qualString );
    mPriceMap[guiItem] = getPriceInfo( match );

    // kDebug() << "showing new plant detail item" << endl;
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
  kDebug() << "Writing price for " << match << ": " << g << endl;
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
  TemplToPositionDialogBase::slotButtonClicked( Ok );
}



InsertPlantDialog::~InsertPlantDialog()
{

}

void InsertPlantDialog::setCatalogChapters( const QList<CatalogChapter>& )
{

}

QString InsertPlantDialog::chapter() const
{
  return QString();
}

#include "insertplantdialog.moc"

/* END */

