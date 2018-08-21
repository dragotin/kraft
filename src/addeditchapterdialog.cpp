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

#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <klocalizedstring.h>

#include "addeditchapterdialog.h"
#include "catalogchapter.h"


AddEditChapterDialog::AddEditChapterDialog( QWidget *parent )
  :QDialog( parent )
{
  setObjectName( "CHAPTER_EDIT_DIALOG" );
  setModal( true );
  setWindowTitle( i18n( "Add/Edit Catalog Chapter" ) );

  QVBoxLayout *vbox = new QVBoxLayout;
  this->setLayout( vbox );

  mTopLabel = new QLabel();
  mTopLabel->setText( i18n("Create a new Catalog Chapter"));
  vbox->addWidget( mTopLabel );


  vbox->addWidget( new QLabel( i18n("Chapter Name:")));
  mNameEdit = new QLineEdit;
  vbox->addWidget( mNameEdit );

  vbox->addWidget( new QLabel( i18n("Chapter Description:")));
  mDescEdit = new QLineEdit;
  vbox->addWidget( mDescEdit );

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);

  QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  vbox->addWidget(buttonBox);
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
  mTopLabel->setText( i18n("Create new Catalog Chapter below chapter %1", chapter.name() ));
}

void AddEditChapterDialog::setEditChapter( const CatalogChapter& chapter )
{
  mChapter = chapter;

  mTopLabel->setText( i18n("Edit name and description of chapter %1", chapter.name() ));
  mNameEdit->setText( chapter.name() );
  mDescEdit->setText( chapter.description() );
}

