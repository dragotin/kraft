/***************************************************************************
 inserttemplatedialog.cpp  - small dialog to insert templates into documents
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

#include "inserttempldialog.h"

// include files for Qt
#include <qvbox.h>
#include <qtextedit.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qtooltip.h>
#include <qmap.h>

// include files for KDE
#include <knuminput.h>

#include <klocale.h>
#include <kdebug.h>

#include "inserttmplbase.h"
#include "templtopositiondialogbase.h"
#include "katalog.h"
#include "einheit.h"
#include "unitmanager.h"
#include "defaultprovider.h"
#include "kraftsettings.h"
#include "tagman.h"

InsertTemplDialog::InsertTemplDialog( QWidget *parent )
  : TemplToPositionDialogBase( parent )
{
  QWidget *w = makeVBoxMainWidget();

  mBaseWidget = new insertTmplBase( w );
  mBaseWidget->dmUnitCombo->insertStringList( UnitManager::allUnits() );

  mBaseWidget->mPriceVal->setSuffix( DefaultProvider::self()->currencySymbol() );

  mBaseWidget->mPriceVal->setMinValue( 0 );
  mBaseWidget->mPriceVal->setMaxValue( 100000 );
  mBaseWidget->mPriceVal->setPrecision( 2 );
  mBaseWidget->dmAmount->setPrecision( 2 );
  mBaseWidget->dmAmount->setRange( 0, 100000, 1 );
  mBaseWidget->dmAmount->setLineStep( 1 );
  mBaseWidget->dmAmount->setSteps( 1, 10 );

  // hide the chapter combo by default
  mBaseWidget->mKeepGroup->hide();
  enableButtonSeparator( false );

  // Fill the tags list
  QButtonGroup *group = mBaseWidget->mTagGroup;

  group->setColumns( 1 );
  QStringList tags = TagTemplateMan::self()->allTagTemplates();
  int c = 0;

  for ( QStringList::Iterator it = tags.begin(); it != tags.end(); ++it ) {
    QCheckBox *cb = new QCheckBox( *it, group );
    QString desc = TagTemplateMan::self()->getTagTemplate( *it ).description();
    QToolTip::add( cb, desc );
    group->insert( cb, c );
    mTagMap[c] = *it;
    c++;
  }
}

void InsertTemplDialog::setDocPosition( DocPosition *dp, bool isNew )
{
  if ( dp ) {
    mParkPosition = *dp;

    mBaseWidget->dmTextEdit->setText( mParkPosition.text() );

    mBaseWidget->dmAmount->setValue( mParkPosition.amount() );
    mBaseWidget->dmUnitCombo->setCurrentText( mParkPosition.unit().einheit( 1.0 ) );
    mBaseWidget->mPriceVal->setValue( mParkPosition.unitPrice().toDouble() );

    if ( mParkPosition.text().isEmpty() ) {
      mBaseWidget->dmHeaderText->setText( i18n( "Create a new Position" ) );
    } else {
      mBaseWidget->dmHeaderText->setText( i18n( "Create a new Position from Template" ) );
    }
    if ( isNew ) {
      mBaseWidget->dmTextEdit->setFocus();
    } else {
      mBaseWidget->dmAmount->setFocus();
    }
  }
}

QComboBox *InsertTemplDialog::getPositionCombo()
{
  return mBaseWidget->dmPositionCombo;
}

DocPosition InsertTemplDialog::docPosition()
{
  mParkPosition.setText( mBaseWidget->dmTextEdit->text() );
  mParkPosition.setAmount( mBaseWidget->dmAmount->value() );
  mParkPosition.setUnitPrice( Geld( mBaseWidget->mPriceVal->value() ) );
  int uid = UnitManager::getUnitIDSingular( mBaseWidget->dmUnitCombo->currentText() );

  mParkPosition.setUnit( UnitManager::getUnit( uid ) );
  // mParkPosition.setPosition( itemPos );

  QButtonGroup *group = mBaseWidget->mTagGroup;
  for ( int i = 0; i < group->count(); i++ ) {
    QCheckBox *b = static_cast<QCheckBox*>( group->find( i ) );

    if ( b && b->isChecked() ) {
      if ( mTagMap.contains( i ) ) {
        QString tag = mTagMap[i];

        mParkPosition.setTag( tag );
      }
    }
  }

  kdDebug() << "in the dialog: " << mParkPosition.text() << endl;
  return mParkPosition;
}


InsertTemplDialog::~InsertTemplDialog()
{
  QString c = mBaseWidget->mComboChapter->currentText();
  if ( ! c.isEmpty() ) {
    KraftSettings::self()->setInsertTemplChapterName( c );
    KraftSettings::self()->writeConfig();
  }
}

void InsertTemplDialog::setCatalogChapters( const QStringList& chapters )
{
  if ( chapters.count() > 0 ) {
    mBaseWidget->mKeepGroup->show();
    mBaseWidget->mComboChapter->insertStringList( chapters );
    mBaseWidget->mComboChapter->setCurrentText(
      KraftSettings::self()->insertTemplChapterName() );
  }
}

// return only a chapter if the checkbox is checked.
QString InsertTemplDialog::chapter() const
{
  QString re;
  if ( mBaseWidget->mCbSave->isChecked() )
    re = mBaseWidget->mComboChapter->currentText();
  return re;
}

#include "inserttempldialog.moc"

/* END */

