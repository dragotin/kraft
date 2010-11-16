/***************************************************************************
               addeditchapter - add and edit catalog chapters
                             -------------------
    begin                : Sat Nov 6 2010
    copyright            : (C) 2010 by Klaas Freitag
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

#include <QtGui>

#include <klocale.h>

#include "addeditchapterdialog.h"
#include "catalogchapter.h"


AddEditChapterDialog::AddEditChapterDialog( QWidget *parent )
  :KDialog( parent )
{
  setObjectName( "CHAPTER_EDIT_DIALOG" );
  setModal( true );
  setCaption( i18n( "Add/Edit Catalog Chapter" ) );
  setButtons( Ok|Cancel );

  showButtonSeparator( true );

  QWidget *w = new QWidget(this);
  setMainWidget( w );

  QVBoxLayout *vbox = new QVBoxLayout;
  w->setLayout( vbox );
  vbox->setSpacing( KDialog::spacingHint() );
  vbox->setMargin( KDialog::marginHint() );

  mTopLabel = new QLabel();
  mTopLabel->setText( i18n("Create a new Catalog Chapter"));
  vbox->addWidget( mTopLabel );


  vbox->addWidget( new QLabel( i18n("Chapter Name:")));
  mNameEdit = new QLineEdit;
  vbox->addWidget( mNameEdit );

  vbox->addWidget( new QLabel( i18n("Chapter Description:")));
  mDescEdit = new QLineEdit;
  vbox->addWidget( mDescEdit );
}

QString AddEditChapterDialog::name() const
{
  return mNameEdit->text();
}

QString AddEditChapterDialog::description() const
{
  return mDescEdit->text();
}

void AddEditChapterDialog::setParentChapter( const CatalogChapter& chapter )
{
  mParentChapter = chapter;
  mTopLabel->setText( i18n("Create new Catalog Chapter below chapter %1").arg( chapter.name() ));
}

void AddEditChapterDialog::setEditChapter( const CatalogChapter& chapter )
{
  mChapter = chapter;

  mTopLabel->setText( i18n("Edit name and description of chapter %1").arg( chapter.name() ));
  mNameEdit->setText( chapter.name() );
  mDescEdit->setText( chapter.description() );
}

