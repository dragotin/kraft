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
#include <QComboBox>
#include <QCheckBox>
#include <QToolTip>
#include <QMap>


// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>
#include <kvbox.h>

#include "importfilter.h"
#include "defaultprovider.h"
#include "kraftsettings.h"
#include "tagman.h"

ImportItemDialog::ImportItemDialog( QWidget *parent )
  : KDialog( parent )
{
  // , "IMPORTITEMDIALOG", true, i18n( "Import Items From File" ),
  //               Ok | Cancel )
  setObjectName( "IMPORTITEMDIALOG" );
  setModal( true );
  setCaption( i18n( "Import Items From File" ) );
  setButtons( Ok | Cancel );

  QWidget *w = new QWidget( this );
  setMainWidget(w);
  mBaseWidget = new Ui::importToDocBase;
  mBaseWidget->setupUi( w );

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
    mBaseWidget->mFileRequester->setUrl( KraftSettings::self()->importItemsFileName() );
  }
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
    kError() << "Can not get a ptr to the position combo";
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
  KStandardDirs dir;

  QString filter = QString::fromLatin1( "kraft/importfilter/positions/*.ftr" );
  QStringList filters = dir.findAllResources( "data", filter );

  QStringList combo;
  QString firstFilter;

  for ( QStringList::Iterator it = filters.begin(); it != filters.end(); ++it ) {
    kDebug() << " -> Import filter file " << *it;
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

void ImportItemDialog::slotOk()
{
#if 0
  FIXME!!!
  KraftSettings::self()->setImportItemsSchemaName( mBaseWidget->mSchemaCombo->currentText() );
  KraftSettings::self()->setImportItemsFileName( mBaseWidget->mFileRequester->url() );
  KraftSettings::self()->writeConfig();
#endif
  KDialog::slotButtonClicked( Ok );
}


DocPositionList ImportItemDialog::positionList()
{
  DocPositionList list;
  KUrl url = mBaseWidget->mFileRequester->url();

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

#include "importitemdialog.moc"

/* END */

