/***************************************************************************
   importitemdialog.h  - small dialog to import items into the document
                             -------------------
    begin                : Nov 2008
    copyright            : (C) 2008 Klaas Freitag
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

#include "importitemdialog.h"

// include files for Qt
#include <QLabel>
#include <QButtonGroup>
#include <QComboBox>
#include <QCheckBox>
#include <QToolTip>
#include <QMap>
#include <QDebug>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLocale>

#include "importfilter.h"
#include "defaultprovider.h"
#include "kraftsettings.h"
#include "tagman.h"

ImportItemDialog::ImportItemDialog( QWidget *parent )
  : QDialog( parent )
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    setObjectName( "IMPORTITEMDIALOG" );

    setModal( true );
    setWindowTitle( i18n( "Import Items From File" ) );

    QWidget *w = new QWidget(this);
    mBaseWidget = new Ui::importToDocBase;
    mBaseWidget->setupUi(w);
    mainLayout->addWidget(w);

    // Fill the tags list
    group = new QButtonGroup(this);
    group->setExclusive(false);

    QStringList tags = TagTemplateMan::self()->allTagTemplates();
    int c = 0;

    QVBoxLayout *checkboxLayout = new QVBoxLayout;

    for ( QStringList::Iterator it = tags.begin(); it != tags.end(); ++it ) {
        QCheckBox *cb = new QCheckBox( *it );
        group->addButton(cb, c);
        checkboxLayout->addWidget(cb);
        QString desc = TagTemplateMan::self()->getTagTemplate( *it ).description();
        cb->setToolTip( desc );
        mTagMap[c] = *it;
        c++;
    }

    checkboxLayout->addStretch(2);
    mBaseWidget->mTagGroup->setLayout(checkboxLayout);

    connect( mBaseWidget->mSchemaCombo, SIGNAL( activated( const QString& ) ),
             SLOT( slotSchemaChanged( const QString& ) ) );
    QString selectName = readFilterSpecs();

    if ( ! KraftSettings::self()->importItemsSchemaName().isEmpty() ) {
        selectName = KraftSettings::self()->importItemsSchemaName();
    }
    mBaseWidget->mSchemaCombo->setCurrentIndex(mBaseWidget->mSchemaCombo->findText( selectName ));
    slotSchemaChanged( selectName );

    if ( ! KraftSettings::self()->importItemsFileName().isEmpty() ) {
        mBaseWidget->mFileNameEdit->setText( KraftSettings::self()->importItemsFileName() );
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    mainLayout->addWidget(buttonBox);
}

ImportItemDialog::~ImportItemDialog()
{

}

QComboBox *ImportItemDialog::getPositionCombo()
{
  return mBaseWidget->dmPositionCombo;
}

void ImportItemDialog::setPositionList( DocPositionList list, int intendedPos )
{
  if ( ! getPositionCombo() ) {
    qCritical() << "Can not get a ptr to the position combo";
    return;
  }
  QStringList strList;
  strList << i18n( "the Header of the Document" );

  DocPositionListIterator it( list );
  while( it.hasNext() ) {
    DocPositionBase *dpb = it.next();
    DocPosition *dp = static_cast<DocPosition*>( dpb );
    QString h = QString( "%1. %2" ).arg( list.posNumber( dp ) ).arg( dp->text() );
    if ( h.length() > 50 ) {
      h = h.left( 50 );
      h += i18n( "..." );
    }
    strList.append( h );
  }

  getPositionCombo()->insertItems(-1, strList );
  getPositionCombo()->setCurrentIndex( intendedPos );
}

QString ImportItemDialog::readFilterSpecs()
{
  QString filter = QString::fromLatin1( "kraft/importfilter/positions/*.ftr" );
  QStringList filters = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, filter);
  QStringList combo;
  QString firstFilter;

  for ( QStringList::Iterator it = filters.begin(); it != filters.end(); ++it ) {
    // qDebug () << " -> Import filter file " << *it;
    DocPositionImportFilter filter;
    filter.readDefinition( *it );
    filter.parseDefinition();
    combo << filter.name();
    if( firstFilter.isEmpty() ) firstFilter = filter.name();
    mFilterMap[filter.name()] = filter;
  }
  mBaseWidget->mSchemaCombo->insertItems(-1, combo );

  return firstFilter;
}

void ImportItemDialog::slotSchemaChanged( const QString& name )
{
  QString desc = mFilterMap[name].description();

  mBaseWidget->mSchemaInfo->setText( desc );
}

DocPositionList ImportItemDialog::positionList()
{
  DocPositionList list;
  QUrl url = QUrl::fromLocalFile(mBaseWidget->mFileNameEdit->text());

  if ( ! url.isEmpty() ) {
    DocPositionImportFilter filter = mFilterMap[mBaseWidget->mSchemaCombo->currentText()];

    list = filter.import( url );

    // get the tags
    QStringList tags;

    QMap<int, QString>::Iterator it;
    for ( it = mTagMap.begin(); it != mTagMap.end(); ++it ) {
      QCheckBox *b = static_cast<QCheckBox*>( group->button( it.key() ) );
      if ( b->isChecked() ) tags.append( it.value() );
    }

    if ( tags.size() > 0 ) {
      DocPositionListIterator posIt( list );
      while( posIt.hasNext() ) {
        DocPositionBase *dp = posIt.next();
        for ( QStringList::Iterator it = tags.begin(); it != tags.end(); ++it ) {
          dp->setTag( *it );
        }
      }
    }
  }
  return list;
}

/* END */

