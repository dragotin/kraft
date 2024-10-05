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

#include "inserttempldialog.h"

// include files for Qt
#include <QTextEdit>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QToolTip>
#include <QMap>
#include <QDate>
#include <QDebug>
#include <QLocale>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "ui_inserttmplbase.h"
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
  QWidget *w = new QWidget( this );

  mBaseWidget = new Ui::insertTmplBase;
  mBaseWidget->setupUi( w );
  mBaseWidget->dmUnitCombo->insertItems( -1, UnitManager::self()->allUnits() );

  mBaseWidget->mPriceVal->setSuffix( DefaultProvider::self()->currencySymbol() );

  mBaseWidget->dmAmount->setDecimals( 2 );
  mBaseWidget->dmAmount->setRange( 0, 100000 );
  mBaseWidget->dmAmount->setSingleStep( 1 );
  // mBaseWidget->dmAmount->setSteps( 1, 10 );

  // hide the chapter combo by default
  mBaseWidget->mKeepGroup->hide();

  // Fill the tags list
  QGroupBox *group = mBaseWidget->mTagGroup;
  QVBoxLayout *groupLay = new QVBoxLayout;
  group->setLayout( groupLay );

  QStringList tags = TagTemplateMan::self()->allTagTemplates();

  int cnt = 0;
  for ( QStringList::Iterator it = tags.begin(); it != tags.end(); ++it ) {
    QCheckBox *cb = new QCheckBox( *it );
    groupLay->insertWidget( cnt++, cb );
    mTagMap[cb] = *it;
  }
  groupLay->addStretch();

  QVBoxLayout *lay = new QVBoxLayout(this);
  lay->setMargin(0);
  lay->addWidget(w);
  setLayout(lay);

  connect(mBaseWidget->mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(mBaseWidget->mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void InsertTemplDialog::setDocPosition(DocPosition *dp, bool isNew , bool showPrices)
{
  if ( dp ) {
    mParkPosition = *dp;

    mBaseWidget->dmTextEdit->setPlainText( prepareText(mParkPosition.text()) );

    mBaseWidget->dmAmount->setValue( mParkPosition.amount() );
    mBaseWidget->dmUnitCombo->setCurrentIndex(mBaseWidget->dmUnitCombo->findText( mParkPosition.unit().einheit( 1.0 )));
    mBaseWidget->mPriceVal->setValue( mParkPosition.unitPrice().toDouble() );

    if ( mParkPosition.text().isEmpty() ) {
      mBaseWidget->dmHeaderText->setText( i18n( "Create a New Item" ) );
    } else {
      mBaseWidget->dmHeaderText->setText( i18n( "Create a New Item from Template" ) );
    }
    if ( isNew ) {
      mBaseWidget->dmTextEdit->setFocus();
    } else {
      mBaseWidget->dmAmount->setFocus();
    }
    mBaseWidget->mPriceVal->setVisible(showPrices);
    mBaseWidget->priceBoxTextLabel->setVisible(showPrices);
  }
}

#define QL1(x) QLatin1String(x)
QString InsertTemplDialog::prepareText( const QString& input )
{
    QString in(input);

    QLocale *locale = DefaultProvider::self()->locale();
    QString dateStr = locale->toString( QDate::currentDate() );
    in.replace(QL1("{{DATE}}"), dateStr, Qt::CaseInsensitive);

    QString timeStr = locale->toString(QTime::currentTime());
    in.replace(QL1("{{TIME}}"), timeStr, Qt::CaseInsensitive);

    if( in.contains(QL1("{{USERNAME}}"))) {
        struct passwd *pw;
        uid_t uid;
        uid = geteuid ();
        pw = getpwuid (uid);
        if (pw) {
            in.replace(QL1("{{USERNAME}}"), QString::fromUtf8(pw->pw_gecos), Qt::CaseInsensitive);
        }
    }

    return in;
}


QComboBox *InsertTemplDialog::getPositionCombo()
{
  return mBaseWidget->dmPositionCombo;
}

DocPosition InsertTemplDialog::docPosition()
{
  mParkPosition.setText( mBaseWidget->dmTextEdit->toPlainText() );
  mParkPosition.setAmount( mBaseWidget->dmAmount->value() );
  mParkPosition.setUnitPrice( Geld( mBaseWidget->mPriceVal->value() ) );
  int uid = UnitManager::self()->getUnitIDSingular( mBaseWidget->dmUnitCombo->currentText() );

  mParkPosition.setUnit( UnitManager::self()->getUnit( uid ) );
  // mParkPosition.setPosition( itemPos );

  QMapIterator<QCheckBox*, QString> i(mTagMap);
  while (i.hasNext()) {
    i.next();
    if( i.key()->isChecked() ) {
      mParkPosition.addTag( i.value() );
    }
  }

  // qDebug () << "in the dialog: " << mParkPosition.tags();
  return mParkPosition;
}


InsertTemplDialog::~InsertTemplDialog()
{
  QString c = mBaseWidget->mComboChapter->currentText();
  if ( ! c.isEmpty() ) {
    KraftSettings::self()->setInsertTemplChapterName( c );
    KraftSettings::self()->save();
  }
}

void InsertTemplDialog::setCatalogChapters( const QList<CatalogChapter>& chapters, const QString& selectedChap)
{
    if ( chapters.count() > 0 ) {
        QStringList chapterNames;
        for( CatalogChapter chapter: chapters ) {
            if (!chapter.name().isEmpty())
                chapterNames.append( chapter.name() );
        }
        mBaseWidget->mKeepGroup->show();
        mBaseWidget->mComboChapter->insertItems( -1, chapterNames );

        QString selChap { selectedChap };
        if (!selectedChap.isEmpty())  {
            mBaseWidget->mKeepGroup->setChecked(true);
        } else {
            selChap = KraftSettings::self()->insertTemplChapterName();
        }
        if (!selChap.isEmpty() && chapterNames.contains(selChap))
            mBaseWidget->mComboChapter->setCurrentIndex(mBaseWidget->mComboChapter->findText(selChap));
    }
}

// return only a chapter if the checkbox is checked.
QString InsertTemplDialog::chapter() const
{
  QString re;
  if ( mBaseWidget->mKeepGroup->isChecked() )
    re = mBaseWidget->mComboChapter->currentText();
  return re;
}

/* END */

