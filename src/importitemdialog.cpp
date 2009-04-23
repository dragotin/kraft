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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "importitemdialog.h"

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
#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>

#include "importtodocbase.h"
#include "importfilter.h"
#include "defaultprovider.h"
#include "kraftsettings.h"
#include "tagman.h"

ImportItemDialog::ImportItemDialog( QWidget *parent )
  : KDialogBase( parent, "IMPORTITEMDIALOG", true, i18n( "Import Items From File" ),
                 Ok | Cancel )
{
  mBaseWidget = new importToDocBase( this );
  QWidget *w = mBaseWidget;

  setMainWidget( w );

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

  connect( mBaseWidget->mSchemaCombo, SIGNAL( activated( const QString& ) ),
           SLOT( slotSchemaChanged( const QString& ) ) );
  QString selectName = readFilterSpecs();

  if ( ! KraftSettings::importItemsSchemaName().isEmpty() ) {
    selectName = KraftSettings::importItemsSchemaName();
  }
  mBaseWidget->mSchemaCombo->setCurrentText( selectName );
  slotSchemaChanged( selectName );

  if ( ! KraftSettings::importItemsFileName().isEmpty() ) {
    mBaseWidget->mFileRequester->setURL( KraftSettings::importItemsFileName() );
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
  DocPositionBase *dpb;
  if ( ! getPositionCombo() ) {
    kdError() << "Can not get a ptr to the position combo" << endl;
    return;
  }
  QStringList strList;
  strList << i18n( "the Header of the Document" );

  for ( dpb = list.first(); dpb; dpb = list.next() ) {
    DocPosition *dp = static_cast<DocPosition*>( dpb );
    QString h = QString( "%1. %2" ).arg( list.posNumber( dp ) ).arg( dp->text() );
    if ( h.length() > 50 ) {
      h = h.left( 50 );
      h += i18n( "..." );
    }
    strList.append( h );
  }

  getPositionCombo()->insertStringList( strList );
  getPositionCombo()->setCurrentItem( intendedPos );
}

QString ImportItemDialog::readFilterSpecs()
{
  KStandardDirs dir;

  QString filter = QString::fromLatin1( "kraft/importfilter/positions/*.ftr" );
  QStringList filters = dir.findAllResources( "data", filter );

  QStringList combo;
  for ( QStringList::Iterator it = filters.begin(); it != filters.end(); ++it ) {
    kdDebug() << " -> Import filter file " << *it << endl;
    DocPositionImportFilter filter;
    filter.readDefinition( *it );
    filter.parseDefinition();
    combo << filter.name();
    mFilterMap[filter.name()] = filter;
  }
  mBaseWidget->mSchemaCombo->insertStringList( combo );

  return combo.first();
}

void ImportItemDialog::slotSchemaChanged( const QString& name )
{
  QString desc = mFilterMap[name].description();

  mBaseWidget->mSchemaInfo->setText( desc );
}

void ImportItemDialog::slotOk()
{
  KraftSettings::setImportItemsSchemaName( mBaseWidget->mSchemaCombo->currentText() );
  KraftSettings::setImportItemsFileName( mBaseWidget->mFileRequester->url() );
  KraftSettings::writeConfig();

  KDialogBase::slotOk();
}


QValueList<DocPosition> ImportItemDialog::positionList()
{
  QValueList<DocPosition> list;
  QString url = mBaseWidget->mFileRequester->url();

  if ( ! url.isEmpty() ) {
    DocPositionImportFilter filter = mFilterMap[mBaseWidget->mSchemaCombo->currentText()];

    list = filter.import( url );

    // get the tags
    QButtonGroup *group = mBaseWidget->mTagGroup;
    QStringList tags;

    QMap<int, QString>::Iterator it;
    for ( it = mTagMap.begin(); it != mTagMap.end(); ++it ) {
      QCheckBox *b = static_cast<QCheckBox*>( group->find( it.key() ) );
      if ( b->isChecked() ) tags.append( it.data() );
    }

    if ( tags.size() > 0 ) {
      QValueList<DocPosition>::iterator posIt;
      for( posIt = list.begin(); posIt != list.end(); ++posIt ) {
        for ( QStringList::Iterator it = tags.begin(); it != tags.end(); ++it ) {
          ( *posIt ).setTag( *it );
        }
      }
    }
  }
  return list;
}

#include "importitemdialog.moc"

/* END */

